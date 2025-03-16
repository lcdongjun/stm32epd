#include "sys.h"
#include "usart.h"	
#include "string.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOS使用
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	


uint8_t USART1_RX_Buff[USART1_RX_BUF_SIZE] = {0};  // DMA接收缓冲区
uint8_t USART1_TX_Buff[USART1_TX_BUF_SIZE] = {0};  // DMA发送缓冲区
volatile uint16_t USART1_RX_Data_Size = 0;         // 接收到的数据长度
volatile uint8_t USART1_RC_Flag = 0;               // 接收完成标志
volatile uint8_t USART1_TC_Flag = 0;               // 发送完成标志
volatile uint8_t data_ready_flag = 0;  // 标志位，指示接收完成

uint8_t RX_Buffer[2][USART1_RX_BUF_SIZE]; // 双缓冲区
uint8_t TX_Buffer[USART1_RX_BUF_SIZE];    // DMA发送缓冲区
volatile uint8_t current_rx_buf = 0; // 当前接收缓冲区索引
volatile uint8_t processing_buf = 0; // 正在处理的缓冲区索引
volatile uint16_t received_len = 0;  // 每次接收到的数据长度


//初始化IO 串口1 
//bound:波特率
void uart_init(u32 bound){
	
		memset(USART1_RX_Buff, 0, sizeof(USART1_RX_Buff));
		memset(TX_Buffer, 0, sizeof(TX_Buffer));
	
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    // 1. GPIO 初始化
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 2. USART 初始化
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_Init(USART1, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 3. DMA 接收配置
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_DeInit(DMA2_Stream5);
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART1_RX_Buff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = USART1_RX_BUF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  // 环形模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_Init(DMA2_Stream5, &DMA_InitStructure);
    

		DMA_DeInit(DMA2_Stream7);
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)TX_Buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = 1;  // 初始为 1，动态设置
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  // 正常模式
    DMA_Init(DMA2_Stream7, &DMA_InitStructure);
		
		// 4. NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		// 5. 初始化
		DMA_Cmd(DMA2_Stream5, ENABLE);
		USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
		USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		USART_Cmd(USART1, ENABLE);
		USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
		DMA_ITConfig(DMA2_Stream5, DMA_IT_TC | DMA_IT_HT, ENABLE);
		DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

		
#endif
	
}

void USART1_DMA_Send(uint8_t *data, uint32_t length) 
{
    if (length > USART1_TX_BUF_SIZE) return;  // 检查数据长度

    while (USART1_TC_Flag);  // 等待上一次发送完成
    USART1_TC_Flag = 1;

    DMA_Cmd(DMA2_Stream7, DISABLE);
    DMA2_Stream7->M0AR = (uint32_t)data;
    DMA2_Stream7->NDTR = length;
    DMA_Cmd(DMA2_Stream7, ENABLE);
}

void USART1_DMA_Receive(uint8_t *data, uint32_t *length) 
{
    if (USART1_RC_Flag) {
        USART1_RC_Flag = 0;  // 清除接收标志
        memcpy(data, USART1_RX_Buff, USART1_RX_Data_Size);
        *length = USART1_RX_Data_Size;  // 返回接收数据长度
        USART1_RX_Data_Size = 0;        // 清除长度记录
    }
}


void USART1_IRQHandler(void) 
{
   if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET) 
    {
        // 清除空闲中断标志
        volatile uint32_t tmp;
        tmp = USART1->SR;
        tmp = USART1->DR;
        (void)tmp; // 防止编译器优化
        
        // 暂停当前 DMA 接收
        DMA_Cmd(DMA2_Stream5, DISABLE);

        // 计算接收到的数据长度
        received_len = USART1_RX_BUF_SIZE - DMA_GetCurrDataCounter(DMA2_Stream5);

        // 检查是否接收长度有效
        if (received_len > 0 && received_len <= USART1_RX_BUF_SIZE) 
        {
            processing_buf = current_rx_buf;        // 切换处理缓冲区
            current_rx_buf = 1 - current_rx_buf;    // 准备另一个缓冲区

            // 重新配置 DMA 缓冲区地址
            DMA2_Stream5->M0AR = (uint32_t)RX_Buffer[current_rx_buf];
            DMA_SetCurrDataCounter(DMA2_Stream5, USART1_RX_BUF_SIZE);
            DMA_Cmd(DMA2_Stream5, ENABLE);         // 恢复 DMA 接收
        } 
        else 
        {
            // 错误处理：如果接收长度超出范围，重置 DMA 和相关状态
            received_len = 0;
            DMA2_Stream5->M0AR = (uint32_t)RX_Buffer[current_rx_buf];
            DMA_SetCurrDataCounter(DMA2_Stream5, USART1_RX_BUF_SIZE);
            DMA_Cmd(DMA2_Stream5, ENABLE);
        }
				
				data_ready_flag = 1;
    }
}

// DMA2 Stream5 接收中断服务函数
void DMA2_Stream5_IRQHandler(void)
{
//	printf("触发DMA接收中断\r\n");
    if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5) == SET) 
		{
//			printf("触发接收完成中断\r\n");
        DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);
        // 半传输完成标志（可选）
    }
    if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_HTIF5) == SET) 
		{
        DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_HTIF5);
//			printf("触发接收缓冲区前半完成中断\r\n");
        // 可在此处理接收缓冲区前半部分的数据
    }
}

// DMA2 Stream7 发送中断服务函数
void DMA2_Stream7_IRQHandler(void) 
{
//		printf("DMA 发送中断\r\n");
    if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) == SET) {
        USART1_TC_Flag = 0;  // 标志发送完成
//        printf("DMA 发送完成中断触发\r\n");
        DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);  // 清除中断标志
    }
}




 



