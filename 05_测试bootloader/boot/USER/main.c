#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "spi.h"
#include "w25qxx.h"
#include "malloc.h"
#include "update.h"

void BootloaderMain(void)
{
	while (1)
	{
		AT_Command();
	}
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // 设置系统中断优先级分组4
	delay_init(168);								// 初始化延时函数
	uart_init(921600);								// 初始化串口
	LED_Init();										// 初始化LED端口
	KEY_Init();
	W25QXX_Init();
	my_mem_init(SRAMIN);
	
	if (WK_UP == 1) 
	{
		printf("\r\nButton pressed. Staying in Bootloader.\r\n");
		BootloaderMain(); // 执行 Bootloader 功能
	}
	else
	{
			if (CheckBootFlag()) 
				{
					printf("\r\n   Boot flag detected. Staying in Bootloader.\r\n");
					BootloaderMain(); // 执行 Bootloader 功能
				}
			else if (IsUserProgramValid())
			{
					printf("\r\nEnter Key_UP to BootLoader\r\n");
					printf("Valid user program found. Jumping to user application...\r\n");
					JumpToUserApplication();
			} 
			else if(!IsUserProgramValid())
			{
					printf("\r\nNo valid user program found. Staying in Bootloader.\r\n");
					printf("User application Err\r\n");
					printf("Bootloader Starting\r\n");
					BootloaderMain(); // 执行 Bootloader 功能
			}
	}	
	//理论上不会执行到这里
	while (1);

}
