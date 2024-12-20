#include "EPD_Test.h"
#include "EPD_4in2_V2.h"
#include <string.h>
#include "calendar.h"

int EPD_test(void)
{
    printf("EPD_4IN2_V2_test Demo\r\n");
    if (DEV_Module_Init() != 0)
    {
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
    EPD_4IN2_V2_Init();
    EPD_4IN2_V2_Clear();
    DEV_Delay_ms(500);

    // Create a new image cache
    UBYTE *BlackImage;
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    BlackImage = mymalloc(SRAMIN, ((EPD_4IN2_V2_WIDTH % 8 == 0) ? (EPD_4IN2_V2_WIDTH / 8) : (EPD_4IN2_V2_WIDTH / 8 + 1)) * EPD_4IN2_V2_HEIGHT);
    //    UWORD Imagesize = ((EPD_4IN2_V2_WIDTH % 8 == 0)? (EPD_4IN2_V2_WIDTH / 8 ): (EPD_4IN2_V2_WIDTH / 8 + 1)) * EPD_4IN2_V2_HEIGHT;
    //    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    //        printf("Failed to apply for black memory...\r\n");
    //        return -1;
    //    }
    if (BlackImage == NULL)
    {
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_4IN2_V2_WIDTH, EPD_4IN2_V2_HEIGHT, 0, EPD_WHITE);

#if 0 // show bmp
    printf("show window BMP-----------------\r\n");
    Paint_SelectImage(BlackImage);
    Paint_Clear(EPD_WHITE);
    Paint_DrawBitMap(gImage_4in2);
    EPD_4IN2_V2_Display(BlackImage);
    DEV_Delay_ms(2000);

#endif

#if 0 // show image for array   
//    EPD_4IN2_V2_Init_Fast(Seconds_1_5S);
    EPD_4IN2_V2_Init_Fast(Seconds_1S);
    printf("show image for array\r\n");
    Paint_SelectImage(BlackImage);
    Paint_Clear(EPD_WHITE);
    Paint_DrawBitMap(gImage_4in2);
    EPD_4IN2_V2_Display_Fast(BlackImage);
    DEV_Delay_ms(2000);
#endif

#if 0 // Drawing on the image
		u32 startTick,endTick;
    EPD_4IN2_V2_Init();
		EPD_4IN2_V2_Init_Fast(Seconds_1S);
    //1.Select Image
    printf("SelectImage:BlackImage\r\n");
    Paint_SelectImage(BlackImage);
    Paint_Clear(EPD_WHITE);

    startTick = xTaskGetTickCount();  // 获取当前 tick 值
		
    // 2.Drawing on the image
    printf("Drawing:BlackImage\r\n");

		Paint_Show_Rectangle(100,20,190,50,10,1,0,0);
		endTick = xTaskGetTickCount();    // 获取结束时的 tick 值
    TickType_t elapsedTicks = endTick - startTick;
    printf("矩形消耗的tick %lu ticks\r\n", elapsedTicks);
		startTick = xTaskGetTickCount();  // 获取当前 tick 值
		

		Paint_Show_Circle(300,200,30,15,1,0,0);
		endTick = xTaskGetTickCount();    // 获取结束时的 tick 值
    elapsedTicks = endTick - startTick;
    printf("圆环消耗的tick %lu ticks\r\n", elapsedTicks);
		startTick = xTaskGetTickCount();  // 获取当前 tick 值
		
		
		Paint_Show_Ellipse(100,200,40,20,1,0,0);
		endTick = xTaskGetTickCount();    // 获取结束时的 tick 值
    elapsedTicks = endTick - startTick;
    printf("椭圆消耗的tick %lu ticks\r\n", elapsedTicks);
		startTick = xTaskGetTickCount();  // 获取当前 tick 值
		
		
		Paint_Show_RoundRect(170,150,210,180,8,5,1,0,0);
		endTick = xTaskGetTickCount();    // 获取结束时的 tick 值
    elapsedTicks = endTick - startTick;
    printf("圆角矩形消耗的tick %lu ticks\r\n", elapsedTicks);
		startTick = xTaskGetTickCount();  // 获取当前 tick 值
		
		
		Paint_Show_Str(10,120,"风 l",24,1,0);
		endTick = xTaskGetTickCount();    // 获取结束时的 tick 值
    elapsedTicks = endTick - startTick;
    printf("汉字以及英文消耗的tick %lu ticks\r\n", elapsedTicks);
		startTick = xTaskGetTickCount();  // 获取当前 tick 值
		
		
		Paint_Show_Font(100,80,"董",24,1,0);
		Paint_Show_Font(124,80,"骏",24,1,1);
		
		
		endTick = xTaskGetTickCount();    // 获取结束时的 tick 值
    elapsedTicks = endTick - startTick;
    printf("两个中文消耗的tick %lu ticks\r\n", elapsedTicks);
		
		
		EPD_4IN2_V2_Display_Fast(Paint.Image);
		
    printf("EPD_Display\r\n");
    // EPD_4IN2_V2_Display(BlackImage);
//	EPD_4IN2_V2_Display(BlackImage);
//	DEV_Delay_ms(2000);
#endif

extern Year yearStruct;

#if 1
	printf("Partial refresh\r\n");
    Paint_NewImage(BlackImage, 200, 50, 0, EPD_WHITE);
//	PAINT_TIME sPaint_time;
//    sPaint_time.Hour = 12;
//    sPaint_time.Min = 34;
//    sPaint_time.Sec = 56;
//    UBYTE num = 10;
//	for (;;) {
//		sPaint_time.Sec = sPaint_time.Sec + 1;
//		if (sPaint_time.Sec == 60) {
//			sPaint_time.Min = sPaint_time.Min + 1;
//			sPaint_time.Sec = 0;
//			if (sPaint_time.Min == 60) {
//				sPaint_time.Hour =  sPaint_time.Hour + 1;
//				sPaint_time.Min = 0;
//				if (sPaint_time.Hour == 24) {
//					sPaint_time.Hour = 0;
//					sPaint_time.Min = 0;
//					sPaint_time.Sec = 0;
//				}
//			}
//		}
		Paint_Clear(EPD_WHITE);
		
//		Paint_Show_xNum(20,10,yearStruct.months[11].daysArray[12].time.Hour,24,1,0);
//		Paint_Show_Char(20+(24*2),10,':',24,1,0);
//		Paint_Show_xNum(20+(24*3),10,yearStruct.months[11].daysArray[12].time.Min,24,1,0);
//		Paint_Show_Char(20+(24*5),10,':',24,1,0);
//		Paint_Show_xNum(20+(24*6),10,yearStruct.months[11].daysArray[12].time.Sec,24,1,0);
		
//		Paint_DrawTime(20, 10, &sPaint_time, 24, EPD_WHITE, EPD_BLACK);
		EPD_4IN2_V2_PartialDisplay(BlackImage, 80, 20, 280, 70);
		DEV_Delay_ms(500);//Analog clock 1s
//		num = num - 1;
//		if(num == 0) {
//			break;
//		}
//    }
#endif

#if 0
//     EPD_4IN2_V2_Init();
//	   EPD_4IN2_V2_Clear();
	EPD_4IN2_V2_Init_4Gray();
	printf("show Gray------------------------\r\n");
	myfree(SRAMIN,BlackImage);
	BlackImage = NULL;
	BlackImage = mymalloc(SRAMIN,((EPD_4IN2_V2_WIDTH % 8 == 0)? (EPD_4IN2_V2_WIDTH / 4 ): (EPD_4IN2_V2_WIDTH / 4 + 1)) * EPD_4IN2_V2_HEIGHT);
    if(BlackImage == NULL) 
		{
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }
//	BlackImage = ((EPD_4IN2_V2_WIDTH % 8 == 0)? (EPD_4IN2_V2_WIDTH / 4 ): (EPD_4IN2_V2_WIDTH / 4 + 1)) * EPD_4IN2_V2_HEIGHT;
//    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
//        printf("Failed to apply for black memory...\r\n");
//        return -1;
//    }
	Paint_NewImage(BlackImage, EPD_4IN2_V2_WIDTH, EPD_4IN2_V2_HEIGHT, 0, EPD_WHITE);
	Paint_SetScale(4);
	Paint_Clear(EPD_WHITE);

	Paint_DrawPoint(10, 80, EPD_BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, EPD_BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, EPD_BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(20, 70, 70, 120, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(20, 70, 70, 120, EPD_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, EPD_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawCircle(45, 95, 20, EPD_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, EPD_WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(85, 95, 125, 95, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawString_EN(10, 0, "waveshare", &Font16, EPD_BLACK, EPD_WHITE);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, EPD_WHITE, EPD_BLACK);
    Paint_DrawNum(10, 33, 123456789, &Font12, EPD_BLACK, EPD_WHITE);
    Paint_DrawNum(10, 50, 987654321, &Font16, EPD_WHITE, EPD_BLACK);
    Paint_DrawString_CN(140, 0, "你好abc", &Font12CN, GRAY1, GRAY4);
    Paint_DrawString_CN(140, 40, "你好abc", &Font12CN, GRAY2, GRAY3);
    Paint_DrawString_CN(140, 80, "你好abc", &Font12CN, GRAY3, GRAY2);
    Paint_DrawString_CN(140, 120, "你好abc", &Font12CN, GRAY4, GRAY1);
	
    Paint_DrawString_CN(220, 0, "微雪电子", &Font24CN, GRAY1, GRAY4);
    Paint_DrawString_CN(220, 40, "微雪电子", &Font24CN, GRAY2, GRAY3);
    Paint_DrawString_CN(220, 80, "微雪电子", &Font24CN, GRAY3, GRAY2);
    Paint_DrawString_CN(220, 120, "微雪电子", &Font24CN, GRAY4, GRAY1);
	
	EPD_4IN2_V2_Display_4Gray(BlackImage);
	DEV_Delay_ms(2000);

	Paint_Clear(EPD_WHITE);
    Paint_DrawBitMap(gImage_4in2_4Gray);
    EPD_4IN2_V2_Display_4Gray(BlackImage);
	DEV_Delay_ms(2000);

#endif

    //    EPD_4IN2_V2_Init();
    //    EPD_4IN2_V2_Clear();
    //    printf("Goto Sleep...\r\n");
    //    EPD_4IN2_V2_Sleep();
    //    free(BlackImage);
    //    BlackImage = NULL;
    //    DEV_Delay_ms(2000);//important, at least 2s
    //    // close 5V
    //    printf("close 5V, Module enters 0 power consumption ...\r\n");
    myfree(SRAMIN, BlackImage);
    DEV_Module_Exit();

    return 0;
}
