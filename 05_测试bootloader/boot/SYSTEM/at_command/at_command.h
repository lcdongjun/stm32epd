#ifndef __AT_COMMAND_H
#define __AT_COMMAND_H 			   
#include <sys.h>	  


typedef enum {
    STATE_IDLE,          // 等待指令
    STATE_RECEIVE_SIZE,  // 接收文件大小
    STATE_RECEIVE_DATA,  // 接收文件数据
    STATE_UPLOAD_DONE,   // 升级完成
		STATE_TO_USERAPP     //重启到用户程序
} UploadState;

void AT_Command(void);

#endif






























