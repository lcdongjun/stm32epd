#ifndef __UPDATE_H
#define __UPDATE_H 			   
#include <sys.h>	  
#include "at_command.h"

#define APP_START_ADDRESS 0x08008000 // 用户程序起始地址

// 标志位定义
#define BOOT_FLAG_NONE 0x00000000		// 无标志，直接进入用户程序
#define FLASH_PAGE_SIZE 131072 // STM32F407 每页 Flash 大小为 2KB
#define BOOT_FLAG_ADDRESS 0x2001FFF0  // 与用户程序中一致
#define BOOT_FLAG_VALUE   0xDEADBEEF  // 与用户程序中一致

extern UploadState current_state;

void SetBootFlag(void);
void ClearBootFlag(void);
uint8_t CheckBootFlag(void);
void JumpToUserApplication(void);
uint8_t IsUserProgramValid(void);
void STM32_Flash_Init(void);
void STM32_Flash_DeInit(void);
u8 STM32_Flash_Erase(uint32_t startAddress, uint32_t binSize);
void STM32_Flash_Write(uint32_t address, uint8_t *data, uint32_t length);
UploadState STM32_Update(uint32_t WriteAddress,uint32_t size);

#endif






























