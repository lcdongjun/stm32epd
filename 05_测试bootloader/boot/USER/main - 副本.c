#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "string.h"
#include "spi.h"
#include "w25qxx.h" 
#include "malloc.h"	   

#define BOOTLOADER_FLAG_ADDR  0x2001FFFC
#define APP_START_ADDRESS     0x08008000  // 用户程序起始地址

// 标志位定义
#define BOOT_FLAG_BOOTLOADER  0xDEADBEEF  // 进入Bootloader的标志
#define BOOT_FLAG_NONE        0x00000000  // 无标志，直接进入用户程序

#define FLASH_PAGE_SIZE 2048 // STM32F407 每页 Flash 大小为 2KB


void BurnBinToFlash(uint32_t srcAddr, uint32_t destAddr, uint32_t size) {
    uint8_t buffer[256]; // 临时缓冲区
    uint32_t bytesRead = 0;

    // 解锁 Flash
    FLASH_Unlock();

    // 擦除目标区域
    for (uint32_t addr = destAddr; addr < (destAddr + size); addr += FLASH_PAGE_SIZE) {
        FLASH_EraseSector(FLASH_Sector_3, VoltageRange_3); // 擦除一个扇区（适配目标 MCU 的扇区大小）
    }

    // 写入 Flash
    while (bytesRead < size) {
        uint16_t chunkSize = (size - bytesRead > sizeof(buffer)) ? sizeof(buffer) : (size - bytesRead);

        // 从 W25Q128 读取数据
        W25QXX_Read(buffer, srcAddr + bytesRead, chunkSize);
			
			
				memcpy(TX_Buffer, buffer, chunkSize);
        DMA_Cmd(DMA2_Stream7, DISABLE);  // 暂停发送 DMA
        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
        DMA2_Stream7->NDTR = chunkSize; // 设置待发送数据长度
        DMA_Cmd(DMA2_Stream7, ENABLE);    // 启动 DMA 发送	
        while (USART1_TC_Flag);  // 等待上次发送完成
        USART1_TC_Flag = 1;  // 标志位复位
			
        // 写入到 STM32 Flash
        for (uint16_t i = 0; i < chunkSize; i += 4) { // Flash 写入按 4 字节对齐
            uint32_t word = *(uint32_t *)(buffer + i);
            FLASH_ProgramWord(destAddr + bytesRead + i, word);
        }

        bytesRead += chunkSize;
    }

    // 锁定 Flash
    FLASH_Lock();
}

void BootFlashMain(void) {
    // 从 W25Q128 的起始地址读取 BIN 文件，并写入到 STM32 的 Flash
    uint32_t binStartAddress = 0xF60000; // W25Q128 中 BIN 文件的起始地址
    uint32_t flashStartAddress = 0x08008000; // 用户程序的起始地址
    uint32_t binSize = 0x790; // BIN 文件大小
    printf("开始烧录bin");
    BurnBinToFlash(binStartAddress, flashStartAddress, binSize);
}


void JumpToUserApplication(void) 
{
    uint32_t appStackPointer = *(volatile uint32_t *)APP_START_ADDRESS; // 用户程序的 MSP 值
    void (*AppEntryPoint)(void);  // 声明一个函数指针

    // 判断用户程序是否有效（通常是检查 MSP 值是否合理）
    if ((appStackPointer & 0x2FFE0000) != 0x20000000) { // RAM 区域起始地址检查
        return;  // 如果无效，不跳转
    }

    // 关闭全局中断
    __disable_irq();

    // 关闭 SysTick，复位到默认值
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // 关闭外设时钟
    RCC->AHB1ENR = 0;
    RCC->AHB2ENR = 0;
    RCC->APB1ENR = 0;
    RCC->APB2ENR = 0;

    // 切换系统时钟到 HSI
    RCC->CR |= RCC_CR_HSION; // 启用 HSI
    while ((RCC->CR & RCC_CR_HSIRDY) == 0); // 等待 HSI 就绪
    RCC->CFGR = 0x00000000; // 系统时钟切换到 HSI

    // 关闭 PLL
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY); // 等待 PLL 关闭

    // 关闭所有中断并清除挂起标志
    for (uint32_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF; // 禁用所有中断
        NVIC->ICPR[i] = 0xFFFFFFFF; // 清除所有挂起中断
    }

    // 设置主堆栈指针（MSP）
    __set_MSP(appStackPointer);

    // 设置用户程序入口地址（复位向量地址）
    AppEntryPoint = (void (*)(void))(*(volatile uint32_t *)(APP_START_ADDRESS + 4));

    // 启用全局中断（用户程序可能需要）
    __enable_irq();

    // 跳转到用户程序
    AppEntryPoint();

    // 如果跳转失败（理论上不会执行到这里）
    while (1);
}


// 检查是否需要进入 Bootloader
int CheckBootloaderFlag(void) 
{
    uint32_t flag = *(volatile uint32_t *)BOOTLOADER_FLAG_ADDR;
    return (flag == BOOT_FLAG_BOOTLOADER);
}

// 清除引导标志
void ClearBootloaderFlag(void) 
{
    *(volatile uint32_t *)BOOTLOADER_FLAG_ADDR = BOOT_FLAG_NONE;
}

u8 First_DMA_Flag = 1;
uint32_t start_address = 0xF60000;   // W25Q128 的最后 0.5MB 起始地址
uint8_t *write_buffer = NULL;       // 动态分配的写缓冲区
uint32_t sector_start;

void BootUsartFlah(void)
{	
		// 模拟 Bootloader 功能
		if (data_ready_flag) 
		{
			data_ready_flag = 0;
			if(!First_DMA_Flag)
			{
        // 动态分配缓冲区
        write_buffer = mymalloc(SRAMIN, received_len);
        if (write_buffer == NULL) {
            printf("Error: Memory allocation failed!\r\n");
					while(1);
        }

        // 将接收到的数据复制到动态分配的缓冲区
        memcpy(write_buffer, RX_Buffer[processing_buf], received_len);

        // 检查是否需要擦除当前扇区
        if (start_address >= sector_start + 4096) {
            sector_start = start_address / 4096 * 4096;
            W25QXX_Erase_Sector(sector_start / 4096);
        }

        // 写入数据到 Flash
        if (start_address + received_len <= 0xFFFFFF) {
            W25QXX_Write(write_buffer, start_address, received_len);

            // 校验写入数据是否成功
            uint8_t *verify_buffer = mymalloc(SRAMIN, received_len);
            if (verify_buffer != NULL) {
                W25QXX_Read(verify_buffer, start_address, received_len);
                if (memcmp(write_buffer, verify_buffer, received_len) == 0) {
                    printf("Write success at address 0x%X\r\n", start_address);
                } else {
                    printf("Error: Data verification failed at address 0x%X\r\n", start_address);
                }
                myfree(SRAMIN, verify_buffer);
            } else {
                printf("Warning: Memory allocation for verification failed!\r\n");
            }

            // 更新地址
            start_address += received_len;
        } else {
            printf("Error: Write range exceeds W25Q128 limit!\r\n");
        }

        // 释放动态分配的缓冲区
        myfree(SRAMIN, write_buffer);
        write_buffer = NULL;

        // 如果超过 Flash 的最大地址，可以根据需求选择暂停任务或环形存储
        if (start_address > 0xFFFFFF) {
            printf("Flash write reached limit. Task suspended.\r\n");
        }
			}
			else
			{
			First_DMA_Flag = 0;
			sector_start = start_address / 4096 * 4096; // 当前扇区起始地址
			}
		}	
}

void BootFlahtoUsart()
{
		uint32_t start_address = 0xF60000;  // 数据存储的起始地址
    uint32_t end_address = 0xF60790;    // 数据存储的结束地址
    uint32_t current_address = start_address;
		uint8_t *read_buffer = NULL; 
		read_buffer = mymalloc(SRAMIN, 64);
    while (1) {
        // 计算本次读取的字节数，避免超出结束地址
        uint32_t bytes_to_read = (current_address + sizeof(read_buffer) <= end_address) 
                                 ? sizeof(read_buffer) 
                                 : (end_address - current_address + 1);

        // 从 Flash 读取数据
        W25QXX_Read(read_buffer, current_address, bytes_to_read);
																 
        memcpy(TX_Buffer, read_buffer, bytes_to_read);
        DMA_Cmd(DMA2_Stream7, DISABLE);  // 暂停发送 DMA
        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
        DMA2_Stream7->NDTR = bytes_to_read; // 设置待发送数据长度
        DMA_Cmd(DMA2_Stream7, ENABLE);    // 启动 DMA 发送	
        while (USART1_TC_Flag);  // 等待上次发送完成
        USART1_TC_Flag = 1;  // 标志位复位
																 
        current_address += bytes_to_read;
        // 如果读取到达结束地址，退出循环
        if (current_address > end_address) {
            break;
        }
				myfree(SRAMIN, read_buffer);	
				delay_ms(5);
		}
}
void BootloaderMain(void) 
{
    // 在此等待新固件（通过串口、SPI 等方式接收）
	while (1) 
	{
		BootUsartFlah();
		if(KEY2 == 0)
		{
			// 读取用户程序，串口发送
			BootFlahtoUsart();
		}
		if(KEY1 == 0)
		{
			// 烧录用户程序
			BootFlashMain();
		}
		if(KEY0 == 0)
		{
			// 烧录完成后跳转到用户程序
			JumpToUserApplication();
		}
		
	}
}


int main(void)
{ 
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init(168);					//初始化延时函数
	uart_init(921600);     				//初始化串口
	LED_Init();		        			//初始化LED端口
	KEY_Init();
	W25QXX_Init(); 
	my_mem_init(SRAMIN);
	printf("Bootloader startup");
	BootloaderMain();	
	while(1)
	{	
	}
}

void USART1_ReceiveTask(void *pvParameters) 
{
	/**************************串口接收并写入flash***************************
	
   uint32_t start_address = 0xF60000;   // W25Q128 的最后 0.5MB 起始地址
    uint32_t sector_start = start_address / 4096 * 4096; // 当前扇区起始地址
    uint8_t *write_buffer = NULL;       // 动态分配的写缓冲区

    while (1) {
        // 等待通知，表示接收到新数据
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 动态分配缓冲区
        write_buffer = mymalloc(SRAMIN, received_len);
        if (write_buffer == NULL) {
            printf("Error: Memory allocation failed!\r\n");
            continue;
        }

        // 将接收到的数据复制到动态分配的缓冲区
        memcpy(write_buffer, RX_Buffer[processing_buf], received_len);

        // 检查是否需要擦除当前扇区
        if (start_address >= sector_start + 4096) {
            sector_start = start_address / 4096 * 4096;
            W25QXX_Erase_Sector(sector_start / 4096);
        }

        // 写入数据到 Flash
        if (start_address + received_len <= 0xFFFFFF) {
            W25QXX_Write(write_buffer, start_address, received_len);

            // 校验写入数据是否成功
            uint8_t *verify_buffer = mymalloc(SRAMIN, received_len);
            if (verify_buffer != NULL) {
                W25QXX_Read(verify_buffer, start_address, received_len);
                if (memcmp(write_buffer, verify_buffer, received_len) == 0) {
                    printf("Write success at address 0x%X\r\n", start_address);
                } else {
                    printf("Error: Data verification failed at address 0x%X\r\n", start_address);
                }
                myfree(SRAMIN, verify_buffer);
            } else {
                printf("Warning: Memory allocation for verification failed!\r\n");
            }

            // 更新地址
            start_address += received_len;
        } else {
            printf("Error: Write range exceeds W25Q128 limit!\r\n");
        }

        // 释放动态分配的缓冲区
        myfree(SRAMIN, write_buffer);
        write_buffer = NULL;

        // 如果超过 Flash 的最大地址，可以根据需求选择暂停任务或环形存储
        if (start_address > 0xFFFFFF) {
            printf("Flash write reached limit. Task suspended.\r\n");
            vTaskSuspend(NULL); // 暂停任务
        }
    }
	*/
	
	/**************************flash读取并串口发送************************************
	
		uint32_t start_address = 0xF60000;  // 数据存储的起始地址
    uint32_t end_address = 0xF6004D;    // 数据存储的结束地址
    uint32_t current_address = start_address;
		uint8_t *read_buffer = NULL; 
		read_buffer = mymalloc(SRAMIN, 64);
    while (1) {
        // 计算本次读取的字节数，避免超出结束地址
        uint32_t bytes_to_read = (current_address + sizeof(read_buffer) <= end_address) 
                                 ? sizeof(read_buffer) 
                                 : (end_address - current_address + 1);

        // 从 Flash 读取数据
        W25QXX_Read(read_buffer, current_address, bytes_to_read);
																 
        memcpy(TX_Buffer, read_buffer, bytes_to_read);
        DMA_Cmd(DMA2_Stream7, DISABLE);  // 暂停发送 DMA
        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
        DMA2_Stream7->NDTR = bytes_to_read; // 设置待发送数据长度
        DMA_Cmd(DMA2_Stream7, ENABLE);    // 启动 DMA 发送	
        while (USART1_TC_Flag);  // 等待上次发送完成
        USART1_TC_Flag = 1;  // 标志位复位
																 
        current_address += bytes_to_read;
        // 如果读取到达结束地址，循环回到起始地址
        if (current_address > end_address) {
            current_address = start_address;
        }
				myfree(SRAMIN, read_buffer);
				vTaskDelay(pdMS_TO_TICKS(500)); // 延时 500ms（根据需要调整）
				
		}
		*/
    while (1) 
    {
        // 等待通知，确保任务不会频繁执行
			if(!First_DMA_Flag)
			{
        // 检查接收到的有效数据
        if (received_len > 0 && received_len <= USART1_RX_BUF_SIZE) 
        {
            // 数据处理逻辑
            memcpy(TX_Buffer, RX_Buffer[processing_buf], received_len);
					
            // DMA 发送
            DMA_Cmd(DMA2_Stream7, DISABLE);
            DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
            DMA2_Stream7->NDTR = received_len;
            DMA_Cmd(DMA2_Stream7, ENABLE);

            // 等待 DMA 发送完成
            while (USART1_TC_Flag);
            USART1_TC_Flag = 1;  // 重置发送标志
        }
			}
			else{
			First_DMA_Flag = 0;
			}
    }													 
}

//        // 等待接收任务通知
//        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//        // 发送接收到的数据
//        memcpy(TX_Buffer, RX_Buffer[processing_buf], received_len);
//        DMA_Cmd(DMA2_Stream7, DISABLE);  // 暂停发送 DMA
//        DMA2_Stream7->M0AR = (uint32_t)TX_Buffer;
//        DMA2_Stream7->NDTR = received_len; // 设置待发送数据长度
//        DMA_Cmd(DMA2_Stream7, ENABLE);    // 启动 DMA 发送

//        // 等待 DMA 发送完成（可以用信号量）
//        while (USART1_TC_Flag);  // 等待上次发送完成
//        USART1_TC_Flag = 1;  // 标志位复位
