#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "key.h"
#include "beep.h"
#include "string.h"
#include "malloc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "spi.h"
#include "w25qxx.h" 
#include "ff.h"  
#include "exfuns.h"  
#include "sdio_sdcard.h"
#include "usmart.h"  
#include "DEV_Config.h"
#include "EPD_Test.h"
#include "fontupd.h"
#include "text.h"	

/************************************************
 ALIENTEK 探索者STM32F407开发板 FreeRTOS实验20-1
 FreeRTOS内存管理实验-库函数版本
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define MALLOC_TASK_PRIO	2
//任务堆栈大小	
#define MALLOC_STK_SIZE 	512
//任务句柄
TaskHandle_t MallocTask_Handler;
//任务函数
void malloc_task(void *p_arg);

FATFS fs1;													/* FatFs文件系统对象 */
FIL fnew;													/* 文件对象 */
FRESULT res_flash;                /* 文件操作结果 */
UINT fnum;            					  /* 文件成功读写数量 */
BYTE ReadBuffer[1024]={0};        /* 读缓冲区 */
BYTE WriteBuffer[] =              /* 写缓冲区*/
"欢迎使用野火STM32 F429开发板 今天是个好日子，新建文件系统测试文件\n"
"——SPI-FatFs文件系统\n"	;

int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init(168);					//初始化延时函数
	uart_init(115200);     				//初始化串口
	LED_Init();		        			//初始化LED端口
	KEY_Init();							//初始化按键
	LCD_Init();							//初始化LCD
	W25QXX_Init();
	SPI2_Init();
	EPD_GPIO_Init();
	usmart_dev.init(84);		//初始化USMART
	my_mem_init(SRAMIN);            	//初始化内部内存池
	exfuns_init();				//为fatfs相关变量申请内存  
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建TASK1任务
    xTaskCreate((TaskFunction_t )malloc_task,             
                (const char*    )"malloc_task",           
                (uint16_t       )MALLOC_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )MALLOC_TASK_PRIO,        
                (TaskHandle_t*  )&MallocTask_Handler);   
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}


//MALLOC任务函数 
void malloc_task(void *pvParameters)
{

	u32 res;
//	
//	exfuns_init();							//为fatfs相关变量申请内存	
	res=f_mount(fs[1],"1:",1); 				//挂载FLASH.
	font_init();
	
//	
	POINT_COLOR=RED;    
	Show_Str(30,50,200,16,"探索者STM32F407开发板",16,0);
	
//	EPD_test();
//	/*----------------------- 文件系统测试：写测试 -----------------------------*/
//	/* 打开文件，如果文件不存在则创建它 */
//	printf("\r\n****** 即将进行文件写入测试... ******\r\n");	
//	res_flash = f_open(&fnew, "1:FatFs读写测试文件.txt",FA_CREATE_ALWAYS | FA_WRITE );
//	if ( res_flash == FR_OK )
//	{
//		printf("》打开/创建FatFs读写测试文件.txt文件成功，向文件写入数据。\r\n");
//    /* 将指定存储区内容写入到文件内 */
//		res_flash=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
//    if(res_flash==FR_OK)
//    {
//      printf("》文件写入成功，写入字节数据：%d\n",fnum);
//      printf("》向文件写入的数据为：\r\n%s\r\n",WriteBuffer);
//    }
//    else
//    {
//      printf("！！文件写入失败：(%d)\n",res_flash);
//    }    
//		/* 不再读写，关闭文件 */
//    f_close(&fnew);
//	}
//	else
//	{	
//		printf("！！打开/创建文件失败。\r\n");
//	}
	
///*------------------- 文件系统测试：读测试 ------------------------------------*/
//	printf("****** 即将进行文件读取测试... ******\r\n");
//	res_flash = f_open(&fnew, "1:FatFs读写测试文件.txt", FA_OPEN_EXISTING | FA_READ); 	 
//	if(res_flash == FR_OK)
//	{
//		printf("》打开文件成功。\r\n");
//		res_flash = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
//    if(res_flash==FR_OK)
//    {
//      printf("》文件读取成功,读到字节数据：%d\r\n",fnum);
//      printf("》读取得的文件数据为：\r\n%s \r\n", ReadBuffer);	
//    }
//    else
//    {
//      printf("！！文件读取失败：(%d)\n",res_flash);
//    }		
//	}
//	else
//	{
//		printf("！！打开文件失败。\r\n");
//	}
//	/* 不再读写，关闭文件 */
//	f_close(&fnew);	
	
	while(1)
	{
		LED0 = 0;
		vTaskDelay(1000);
		LED0 = 1;
		vTaskDelay(1000);	
	}

} 

