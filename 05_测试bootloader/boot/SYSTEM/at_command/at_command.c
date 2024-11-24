#include "sys.h"
#include "at_command.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "usart.h"
#include "update.h"

UploadState current_state = STATE_IDLE;

uint32_t file_size = 0;       // 文件大小
uint32_t received_size = 0;   // 已接收数据大小


void parse_command(char *command) {
    if (strncmp(command, "AT+UploadStart=", 15) == 0) {
        // 提取文件大小
        sscanf(command + 15, " File size: {\"size\":%lu}", &file_size);
        printf("Received file size: %lu\n", file_size);
        // 状态转移到接收数据阶段
        current_state = STATE_RECEIVE_SIZE;
        received_size = 0;  // 重置已接收大小

    } else if (strncmp(command, "AT+UploadDate=", 14) == 0) {
        // 转移到接收数据状态
        current_state = STATE_RECEIVE_DATA;

    } else if (strncmp(command, "AT+UploadEnd= 1", 15) == 0) {
				printf("Upgrade completed successfully!\n");
				current_state = STATE_UPLOAD_DONE;
    } else if (strncmp(command, "AT+RebootToUserApp", 15) == 0) {
				current_state = STATE_TO_USERAPP;
    } else {
        printf("Unknown command: %s\n", command);
        current_state = STATE_IDLE;
    }
}



void AT_Command(){
		
	while (1) {
		switch (current_state) {
		case STATE_IDLE:
				// 接收指令并解析
				while(!data_ready_flag);
				data_ready_flag = 0;
				parse_command((char*)RX_Buffer[processing_buf]);
				break;

		case STATE_RECEIVE_SIZE:
				STM32_Flash_Erase(0x08008000,file_size); // BIN 文件大小
				printf("Erasing Flash OK\r\n");
				printf("File size acknowledged. Waiting for data...\n");
				
				current_state = STATE_IDLE;  // 等待下一条指令
				break;

		case STATE_RECEIVE_DATA:
				printf("Update System\r\n");
				current_state = STM32_Update(0x08008000,file_size);
				break;

		case STATE_UPLOAD_DONE:
				printf("Upgrade process finished.\n");
				current_state = STATE_IDLE;  // 返回空闲状态，准备下一次
				break;
		case STATE_TO_USERAPP:
				printf("Reboot To UserApplication!\n");
				JumpToUserApplication();
				current_state = STATE_IDLE;  // 返回空闲状态，准备下一次
				break;

		default:
				printf("Unknown state.\n");
				current_state = STATE_IDLE;
				break;
		}
	}
}

			 



































