
#ifndef __GUI_PAINT_H
#define __GUI_PAINT_H

#include "DEV_Config.h"
#include "../Fonts/fonts.h"


/**
 * Image attributes
**/
typedef struct {
    UBYTE *Image;
    UWORD Width;
    UWORD Height;
    UWORD WidthMemory;
    UWORD HeightMemory;
    UWORD Color;
    UWORD Rotate;
    UWORD Mirror;
    UWORD WidthByte;
    UWORD HeightByte;
    UWORD Scale;
} PAINT;
extern PAINT Paint;

/**
 * Display rotate
**/
#define ROTATE_0            0
#define ROTATE_90           90
#define ROTATE_180          180
#define ROTATE_270          270

/**
 * Display Flip
**/
typedef enum {
    MIRROR_NONE  = 0x00,
    MIRROR_HORIZONTAL = 0x01,
    MIRROR_VERTICAL = 0x02,
    MIRROR_ORIGIN = 0x03,
} MIRROR_IMAGE;
#define MIRROR_IMAGE_DFT MIRROR_NONE

/**
 * image color
**/
#define EPD_WHITE          0xFF
#define EPD_BLACK          0x00
#define EPD_RED            EPD_BLACK

#define IMAGE_BACKGROUND    EPD_WHITE
#define FONT_FOREGROUND     EPD_BLACK
#define FONT_BACKGROUND     EPD_WHITE

#define TRUE 1
#define FALSE 0

//4 Gray level
#define  GRAY1 0x03 //Blackest
#define  GRAY2 0x02
#define  GRAY3 0x01 //gray
#define  GRAY4 0x00 //white

/**
 * The size of the point
**/
typedef enum {
    DOT_PIXEL_1X1  = 1,		// 1 x 1
    DOT_PIXEL_2X2  , 		// 2 X 2
    DOT_PIXEL_3X3  ,		// 3 X 3
    DOT_PIXEL_4X4  ,		// 4 X 4
    DOT_PIXEL_5X5  , 		// 5 X 5
    DOT_PIXEL_6X6  , 		// 6 X 6
    DOT_PIXEL_7X7  , 		// 7 X 7
    DOT_PIXEL_8X8  , 		// 8 X 8
} DOT_PIXEL;
#define DOT_PIXEL_DFT  DOT_PIXEL_1X1  //Default dot pilex

/**
 * Point size fill style
**/
typedef enum {
    DOT_FILL_AROUND  = 1,		// dot pixel 1 x 1
    DOT_FILL_RIGHTUP  , 		// dot pixel 2 X 2
} DOT_STYLE;
#define DOT_STYLE_DFT  DOT_FILL_AROUND  //Default dot pilex

/**
 * Line style, solid or dashed
**/
typedef enum {
    LINE_STYLE_SOLID = 0,
    LINE_STYLE_DOTTED,
} LINE_STYLE;

/**
 * Whether the graphic is filled
**/
typedef enum {
    DRAW_FILL_EMPTY = 0,
    DRAW_FILL_FULL,
} DRAW_FILL;

/**
 * Custom structure of a time attribute
**/
typedef struct {
    UWORD Year;  //0000
    UBYTE  Month; //1 - 12
    UBYTE  Day;   //1 - 30
    UBYTE  Hour;  //0 - 23
    UBYTE  Min;   //0 - 59
    UBYTE  Sec;   //0 - 59
} PAINT_TIME;
extern PAINT_TIME sPaint_time;


//init and Clear

void Paint_NewImage(UBYTE *image, UWORD Width, UWORD Height, UWORD Rotate, UWORD Color);
//创建图像
void Paint_SelectImage(UBYTE *image);

void Paint_SetRotate(UWORD Rotate);
//图像旋转
void Paint_SetMirroring(UBYTE mirror);
//镜像图像
void Paint_SetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color);
//画点函数(像素级)
void Paint_SetScale(UBYTE scale);


void Paint_Clear(UWORD Color);
//清屏
void Paint_ClearWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color);
//局部清屏


//Drawing

void Paint_DrawPoint(UWORD Xpoint, UWORD Ypoint, UWORD Color, DOT_PIXEL Dot_Pixel, DOT_STYLE Dot_FillWay);
//画点函数
void Paint_DrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color, DOT_PIXEL Line_width, LINE_STYLE Line_Style);
//画线函数
void Paint_DrawRectangle(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill);
//画矩形函数
void Paint_DrawCircle(UWORD X_Center, UWORD Y_Center, UWORD Radius, UWORD Color, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill);
//画圆函数


//Display string

void Paint_DrawChar(UWORD Xstart, UWORD Ystart, const char Acsii_Char, sFONT* Font, UWORD Color_Foreground, UWORD Color_Background);
//显示英文字符
void Paint_DrawString_EN(UWORD Xstart, UWORD Ystart, const char *pString,u8 size, UWORD Color_Foreground, UWORD Color_Background);
//显示英文字符串
void Paint_DrawString_CN(UWORD Xstart, UWORD Ystart, const char * pString, cFONT* font, UWORD Color_Foreground, UWORD Color_Background);
//一种内置字体显示中文
void Paint_DrawNum(UWORD Xpoint, UWORD Ypoint, int32_t Nummber, u8 size, UWORD Color_Foreground, UWORD Color_Background);
//显示数字
void Paint_DrawNumDecimals(UWORD Xpoint, UWORD Ypoint, double Nummber, u8 size, UWORD Digit, UWORD Color_Foreground, UWORD Color_Background); // Able to display decimals
//支持小数的数字显示
void Paint_DrawTime(UWORD Xstart, UWORD Ystart, PAINT_TIME *pTime, u8 size, UWORD Color_Foreground, UWORD Color_Background);
//显示时间

//pic
void Paint_DrawBitMap(const unsigned char* image_buffer);
void Paint_DrawBitMap_Paste(const unsigned char* image_buffer, UWORD Xstart, UWORD Ystart, UWORD imageWidth, UWORD imageHeight, UBYTE flipColor);
//void Paint_DrawBitMap_Half(const unsigned char* image_buffer, UBYTE Region);
//void Paint_DrawBitMap_OneQuarter(const unsigned char* image_buffer, UBYTE Region);
//void Paint_DrawBitMap_OneEighth(const unsigned char* image_buffer, UBYTE Region);
void Paint_DrawBitMap_Block(const unsigned char* image_buffer, UBYTE Region);



void Paint_Show_Line(u16 x_start, u16 y_start, u16 x_end, u16 y_end, u16 color, u8 invPix);

void Paint_Show_Arc(u16 x_center, u16 y_center, u16 radius, u16 start_angle, u16 end_angle, u8 thickness, u16 color, u8 filled, u8 invPix);

void Paint_Show_Rectangle(u16 x_start, u16 y_start, u16 x_end, u16 y_end, u8 thickness,u16 color, u8 filled, u8 invPix);

void Paint_Show_Circle(u16 x_center, u16 y_center, u16 radius, u8 thickness, u16 color, u8 filled, u8 invPix);

void Paint_Show_Ellipse(u16 x_center, u16 y_center, u16 x_radius, u16 y_radius, u16 color, u8 filled, u8 invPix);
	
void Paint_Show_RoundRect(u16 x_start, u16 y_start, u16 x_end, u16 y_end, u16 radius, u8 thickness, u16 color, u8 filled, u8 invPix);

void Paint_Show_Triangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 color, u8 filled, u8 invPix);


void Paint_Show_Char(u16 x,u16 y,const char Acsii_Char,u8 size,u8 mode,u8 cover);
//显示一个英文字符
void Paint_Get_HzMat(unsigned char *code,unsigned char *mat,u8 size);
void Paint_Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode,u8 cover);
//显示一个汉字
void Paint_Show_Str(u16 x,u16 y,u8*str,u8 size,u8 mode,u8 cover);
//显示字符串，支持中文
void Paint_Show_xNum(u16 x, u16 y, s32 nummber, u8 size, u8 mode, u8 cover);
//显示数字
void Paint_Show_xDecnum(u16 x,u16 y,double nummber,u8 size,u8 digit,u8 mode,u8 cover);
//显示小数

#endif





