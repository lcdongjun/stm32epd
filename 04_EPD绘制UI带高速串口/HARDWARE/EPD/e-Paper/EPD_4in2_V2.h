
#ifndef _EPD_4IN2_V2_H_
#define _EPD_4IN2_V2_H_

#include "DEV_Config.h"

// Display resolution
#define EPD_4IN2_V2_WIDTH       400
#define EPD_4IN2_V2_HEIGHT      300

#define Seconds_1_5S      0
#define Seconds_1S        1

void EPD_4IN2_V2_Init(void);
void EPD_4IN2_V2_Init_Fast(UBYTE Mode);
void EPD_4IN2_V2_Init_4Gray(void);
void EPD_4IN2_V2_Clear(void);
void EPD_4IN2_V2_Display(UBYTE *Image);
void EPD_4IN2_V2_Display_Fast(UBYTE *Image);
void EPD_4IN2_V2_Display_4Gray(UBYTE *Image);
void EPD_4IN2_V2_PartialDisplay(UBYTE *Image, UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);
void EPD_4IN2_V2_Sleep(void);

#endif
