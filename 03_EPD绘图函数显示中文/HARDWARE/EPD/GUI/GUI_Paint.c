#include "GUI_Paint.h"
#include "DEV_Config.h"
#include "Debug.h"
#include "fontupd.h"
#include "ff.h"	  
#include "w25qxx.h" 
#include <stdint.h>
#include <stdlib.h>
#include <string.h> //memset()
//#include <math.h>
#include "arm_math.h" 


PAINT Paint;

/******************************************************************************
function: Create Image
parameter:
    image   :   Pointer to the image cache
    width   :   The width of the picture
    Height  :   The height of the picture
    Color   :   Whether the picture is inverted
******************************************************************************/
void Paint_NewImage(UBYTE *image, UWORD Width, UWORD Height, UWORD Rotate, UWORD Color)
{
    Paint.Image = NULL;
    Paint.Image = image;

    Paint.WidthMemory = Width;
    Paint.HeightMemory = Height;
    Paint.Color = Color;
    Paint.Scale = 2;

    Paint.WidthByte = (Width % 8 == 0) ? (Width / 8) : (Width / 8 + 1);
    Paint.HeightByte = Height;
    //    printf("WidthByte = %d, HeightByte = %d\r\n", Paint.WidthByte, Paint.HeightByte);
    //    printf(" EPD_WIDTH / 8 = %d\r\n",  122 / 8);

    Paint.Rotate = Rotate;
    Paint.Mirror = MIRROR_NONE;

    if (Rotate == ROTATE_0 || Rotate == ROTATE_180)
    {
        Paint.Width = Width;
        Paint.Height = Height;
    }
    else
    {
        Paint.Width = Height;
        Paint.Height = Width;
    }
}

/******************************************************************************
function: Select Image
parameter:
    image : Pointer to the image cache
******************************************************************************/
void Paint_SelectImage(UBYTE *image)
{
    Paint.Image = image;
}

/******************************************************************************
function: Select Image Rotate
parameter:
    Rotate : 0,90,180,270
******************************************************************************/
void Paint_SetRotate(UWORD Rotate)
{
    if (Rotate == ROTATE_0 || Rotate == ROTATE_90 || Rotate == ROTATE_180 || Rotate == ROTATE_270)
    {
        Debug("Set image Rotate %d\r\n", Rotate);
        Paint.Rotate = Rotate;
    }
    else
    {
        Debug("rotate = 0, 90, 180, 270\r\n");
    }
}

void Paint_SetScale(UBYTE scale)
{
    if (scale == 2)
    {
        Paint.Scale = scale;
        Paint.WidthByte = (Paint.WidthMemory % 8 == 0) ? (Paint.WidthMemory / 8) : (Paint.WidthMemory / 8 + 1);
    }
    else if (scale == 4)
    {
        Paint.Scale = scale;
        Paint.WidthByte = (Paint.WidthMemory % 4 == 0) ? (Paint.WidthMemory / 4) : (Paint.WidthMemory / 4 + 1);
    }
    else if (scale == 6 || scale == 7)
    { // Only applicable with 5in65 e-Paper
        Paint.Scale = scale;
        Paint.WidthByte = (Paint.WidthMemory % 2 == 0) ? (Paint.WidthMemory / 2) : (Paint.WidthMemory / 2 + 1);
        ;
    }
    else
    {
        Debug("Set Scale Input parameter error\r\n");
        Debug("Scale Only support: 2 4 7\r\n");
    }
}
/******************************************************************************
function:	Select Image mirror
parameter:
    mirror   :Not mirror,Horizontal mirror,Vertical mirror,Origin mirror
******************************************************************************/
void Paint_SetMirroring(UBYTE mirror)
{
    if (mirror == MIRROR_NONE || mirror == MIRROR_HORIZONTAL ||
        mirror == MIRROR_VERTICAL || mirror == MIRROR_ORIGIN)
    {
        Debug("mirror image x:%s, y:%s\r\n", (mirror & 0x01) ? "mirror" : "none", ((mirror >> 1) & 0x01) ? "mirror" : "none");
        Paint.Mirror = mirror;
    }
    else
    {
        Debug("mirror should be MIRROR_NONE, MIRROR_HORIZONTAL, \
        MIRROR_VERTICAL or MIRROR_ORIGIN\r\n");
    }
}

/******************************************************************************
function: Draw Pixels
parameter:
    Xpoint : At point X
    Ypoint : At point Y
    Color  : Painted colors
******************************************************************************/
void Paint_SetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color)
{
    if (Xpoint > Paint.Width || Ypoint > Paint.Height)
    {
        Debug("Exceeding display boundaries\r\n");
        return;
    }
    UWORD X, Y;

    switch (Paint.Rotate)
    {
    case 0:
        X = Xpoint;
        Y = Ypoint;
        break;
    case 90:
        X = Paint.WidthMemory - Ypoint - 1;
        Y = Xpoint;
        break;
    case 180:
        X = Paint.WidthMemory - Xpoint - 1;
        Y = Paint.HeightMemory - Ypoint - 1;
        break;
    case 270:
        X = Ypoint;
        Y = Paint.HeightMemory - Xpoint - 1;
        break;
    default:
        return;
    }

    switch (Paint.Mirror)
    {
    case MIRROR_NONE:
        break;
    case MIRROR_HORIZONTAL:
        X = Paint.WidthMemory - X - 1;
        break;
    case MIRROR_VERTICAL:
        Y = Paint.HeightMemory - Y - 1;
        break;
    case MIRROR_ORIGIN:
        X = Paint.WidthMemory - X - 1;
        Y = Paint.HeightMemory - Y - 1;
        break;
    default:
        return;
    }

    if (X > Paint.WidthMemory || Y > Paint.HeightMemory)
    {
        Debug("Exceeding display boundaries\r\n");
        return;
    }

    if (Paint.Scale == 2)
    {
        UDOUBLE Addr = X / 8 + Y * Paint.WidthByte;
        UBYTE Rdata = Paint.Image[Addr];
        if (Color == EPD_BLACK)
            Paint.Image[Addr] = Rdata & ~(0x80 >> (X % 8));
        else
            Paint.Image[Addr] = Rdata | (0x80 >> (X % 8));
    }
    else if (Paint.Scale == 4)
    {
        UDOUBLE Addr = X / 4 + Y * Paint.WidthByte;
        Color = Color % 4; // Guaranteed color scale is 4  --- 0~3
        UBYTE Rdata = Paint.Image[Addr];

        Rdata = Rdata & (~(0xC0 >> ((X % 4) * 2)));
        Paint.Image[Addr] = Rdata | ((Color << 6) >> ((X % 4) * 2));
    }
    else if (Paint.Scale == 6 || Paint.Scale == 7)
    {
        UDOUBLE Addr = X / 2 + Y * Paint.WidthByte;
        UBYTE Rdata = Paint.Image[Addr];
        Rdata = Rdata & (~(0xF0 >> ((X % 2) * 4))); // Clear first, then set value
        Paint.Image[Addr] = Rdata | ((Color << 4) >> ((X % 2) * 4));
        // printf("Add =  %d ,data = %d\r\n",Addr,Rdata);
    }
}

/******************************************************************************
function: Clear the color of the picture
parameter:
    Color : Painted colors
******************************************************************************/
void Paint_Clear(UWORD Color)
{
    if (Paint.Scale == 2)
    {
        for (UWORD Y = 0; Y < Paint.HeightByte; Y++)
        {
            for (UWORD X = 0; X < Paint.WidthByte; X++)
            { // 8 pixel =  1 byte
                UDOUBLE Addr = X + Y * Paint.WidthByte;
                Paint.Image[Addr] = Color;
            }
        }
    }
    else if (Paint.Scale == 4)
    {
        for (UWORD Y = 0; Y < Paint.HeightByte; Y++)
        {
            for (UWORD X = 0; X < Paint.WidthByte; X++)
            {
                UDOUBLE Addr = X + Y * Paint.WidthByte;
                Paint.Image[Addr] = (Color << 6) | (Color << 4) | (Color << 2) | Color;
            }
        }
    }
    else if (Paint.Scale == 6 || Paint.Scale == 7)
    {
        for (UWORD Y = 0; Y < Paint.HeightByte; Y++)
        {
            for (UWORD X = 0; X < Paint.WidthByte; X++)
            {
                UDOUBLE Addr = X + Y * Paint.WidthByte;
                Paint.Image[Addr] = (Color << 4) | Color;
            }
        }
    }
}

/******************************************************************************
function: Clear the color of a window
parameter:
    Xstart : x starting point
    Ystart : Y starting point
    Xend   : x end point
    Yend   : y end point
    Color  : Painted colors
******************************************************************************/
void Paint_ClearWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color)
{
    UWORD X, Y;
    for (Y = Ystart; Y < Yend; Y++)
    {
        for (X = Xstart; X < Xend; X++)
        { // 8 pixel =  1 byte
            Paint_SetPixel(X, Y, Color);
        }
    }
}

/******************************************************************************
function: Draw Point(Xpoint, Ypoint) Fill the color
parameter:
    Xpoint		: The Xpoint coordinate of the point
    Ypoint		: The Ypoint coordinate of the point
    Color		: Painted color
    Dot_Pixel	: point size
    Dot_Style	: point Style
******************************************************************************/
void Paint_DrawPoint(UWORD Xpoint, UWORD Ypoint, UWORD Color,
                     DOT_PIXEL Dot_Pixel, DOT_STYLE Dot_Style)
{
    if (Xpoint > Paint.Width || Ypoint > Paint.Height)
    {
        Debug("Paint_DrawPoint Input exceeds the normal display range\r\n");
        printf("Xpoint = %d , Paint.Width = %d  \r\n ", Xpoint, Paint.Width);
        printf("Ypoint = %d , Paint.Height = %d  \r\n ", Ypoint, Paint.Height);
        return;
    }

    int16_t XDir_Num, YDir_Num;
    if (Dot_Style == DOT_FILL_AROUND)
    {
        for (XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++)
        {
            for (YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1; YDir_Num++)
            {
                if (Xpoint + XDir_Num - Dot_Pixel < 0 || Ypoint + YDir_Num - Dot_Pixel < 0)
                    break;
                // printf("x = %d, y = %d\r\n", Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel);
                Paint_SetPixel(Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel, Color);
            }
        }
    }
    else
    {
        for (XDir_Num = 0; XDir_Num < Dot_Pixel; XDir_Num++)
        {
            for (YDir_Num = 0; YDir_Num < Dot_Pixel; YDir_Num++)
            {
                Paint_SetPixel(Xpoint + XDir_Num - 1, Ypoint + YDir_Num - 1, Color);
            }
        }
    }
}

/******************************************************************************
function: Draw a line of arbitrary slope
parameter:
    Xstart ：Starting Xpoint point coordinates
    Ystart ：Starting Xpoint point coordinates
    Xend   ：End point Xpoint coordinate
    Yend   ：End point Ypoint coordinate
    Color  ：The color of the line segment
    Line_width : Line width
    Line_Style: Solid and dotted lines
******************************************************************************/
void Paint_DrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,
                    UWORD Color, DOT_PIXEL Line_width, LINE_STYLE Line_Style)
{
    if (Xstart > Paint.Width || Ystart > Paint.Height ||
        Xend > Paint.Width || Yend > Paint.Height)
    {
        Debug("Paint_DrawLine Input exceeds the normal display range\r\n");
        return;
    }

    UWORD Xpoint = Xstart;
    UWORD Ypoint = Ystart;
    int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

    // Increment direction, 1 is positive, -1 is counter;
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;

    // Cumulative error
    int Esp = dx + dy;
    char Dotted_Len = 0;

    for (;;)
    {
        Dotted_Len++;
        // Painted dotted line, 2 point is really virtual
        if (Line_Style == LINE_STYLE_DOTTED && Dotted_Len % 3 == 0)
        {
            // Debug("LINE_DOTTED\r\n");
            Paint_DrawPoint(Xpoint, Ypoint, IMAGE_BACKGROUND, Line_width, DOT_STYLE_DFT);
            Dotted_Len = 0;
        }
        else
        {
            Paint_DrawPoint(Xpoint, Ypoint, Color, Line_width, DOT_STYLE_DFT);
        }
        if (2 * Esp >= dy)
        {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx)
        {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

/******************************************************************************
function: Draw a rectangle
parameter:
    Xstart ：Rectangular  Starting Xpoint point coordinates
    Ystart ：Rectangular  Starting Xpoint point coordinates
    Xend   ：Rectangular  End point Xpoint coordinate
    Yend   ：Rectangular  End point Ypoint coordinate
    Color  ：The color of the Rectangular segment
    Line_width: Line width
    Draw_Fill : Whether to fill the inside of the rectangle
******************************************************************************/
void Paint_DrawRectangle(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,
                         UWORD Color, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill)
{
    if (Xstart > Paint.Width || Ystart > Paint.Height ||
        Xend > Paint.Width || Yend > Paint.Height)
    {
        Debug("Input exceeds the normal display range\r\n");
        return;
    }

    if (Draw_Fill)
    {
        UWORD Ypoint;
        for (Ypoint = Ystart; Ypoint < Yend; Ypoint++)
        {
            Paint_DrawLine(Xstart, Ypoint, Xend, Ypoint, Color, Line_width, LINE_STYLE_SOLID);
        }
    }
    else
    {
        Paint_DrawLine(Xstart, Ystart, Xend, Ystart, Color, Line_width, LINE_STYLE_SOLID);
        Paint_DrawLine(Xstart, Ystart, Xstart, Yend, Color, Line_width, LINE_STYLE_SOLID);
        Paint_DrawLine(Xend, Yend, Xend, Ystart, Color, Line_width, LINE_STYLE_SOLID);
        Paint_DrawLine(Xend, Yend, Xstart, Yend, Color, Line_width, LINE_STYLE_SOLID);
    }
}

/******************************************************************************
function: Use the 8-point method to draw a circle of the
            specified size at the specified position->
parameter:
    X_Center  ：Center X coordinate
    Y_Center  ：Center Y coordinate
    Radius    ：circle Radius
    Color     ：The color of the ：circle segment
    Line_width: Line width
    Draw_Fill : Whether to fill the inside of the Circle
******************************************************************************/
void Paint_DrawCircle(UWORD X_Center, UWORD Y_Center, UWORD Radius,
                      UWORD Color, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill)
{
    if (X_Center > Paint.Width || Y_Center >= Paint.Height)
    {
        Debug("Paint_DrawCircle Input exceeds the normal display range\r\n");
        return;
    }

    // Draw a circle from(0, R) as a starting point
    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;

    // Cumulative error,judge the next point of the logo
    int16_t Esp = 3 - (Radius << 1);

    int16_t sCountY;
    if (Draw_Fill == DRAW_FILL_FULL)
    {
        while (XCurrent <= YCurrent)
        { // Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY++)
            {
                Paint_DrawPoint(X_Center + XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 1
                Paint_DrawPoint(X_Center - XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 2
                Paint_DrawPoint(X_Center - sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 3
                Paint_DrawPoint(X_Center - sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 4
                Paint_DrawPoint(X_Center - XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 5
                Paint_DrawPoint(X_Center + XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 6
                Paint_DrawPoint(X_Center + sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 7
                Paint_DrawPoint(X_Center + sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            if (Esp < 0)
                Esp += 4 * XCurrent + 6;
            else
            {
                Esp += 10 + 4 * (XCurrent - YCurrent);
                YCurrent--;
            }
            XCurrent++;
        }
    }
    else
    { // Draw a hollow circle
        while (XCurrent <= YCurrent)
        {
            Paint_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color, Line_width, DOT_STYLE_DFT); // 1
            Paint_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color, Line_width, DOT_STYLE_DFT); // 2
            Paint_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color, Line_width, DOT_STYLE_DFT); // 3
            Paint_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color, Line_width, DOT_STYLE_DFT); // 4
            Paint_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color, Line_width, DOT_STYLE_DFT); // 5
            Paint_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color, Line_width, DOT_STYLE_DFT); // 6
            Paint_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color, Line_width, DOT_STYLE_DFT); // 7
            Paint_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color, Line_width, DOT_STYLE_DFT); // 0

            if (Esp < 0)
                Esp += 4 * XCurrent + 6;
            else
            {
                Esp += 10 + 4 * (XCurrent - YCurrent);
                YCurrent--;
            }
            XCurrent++;
        }
    }
}

/******************************************************************************
function: Show English characters
parameter:
    Xpoint           ：X coordinate
    Ypoint           ：Y coordinate
    Acsii_Char       ：To display the English characters
    Font             ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void Paint_DrawChar(UWORD Xpoint, UWORD Ypoint, const char Acsii_Char,
                    sFONT *Font, UWORD Color_Foreground, UWORD Color_Background)
{
    UWORD Page, Column;

    if (Xpoint > Paint.Width || Ypoint > Paint.Height)
    {
        Debug("Paint_DrawChar Input exceeds the normal display range\r\n");
        return;
    }

    uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height * (Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &Font->table[Char_Offset];

    for (Page = 0; Page < Font->Height; Page++)
    {
        for (Column = 0; Column < Font->Width; Column++)
        {

            // To determine whether the font background color and screen background color is consistent
            if (FONT_BACKGROUND == Color_Background)
            { // this process is to speed up the scan
                if (*ptr & (0x80 >> (Column % 8)))
                    Paint_SetPixel(Xpoint + Column, Ypoint + Page, Color_Foreground);
                // Paint_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            else
            {
                if (*ptr & (0x80 >> (Column % 8)))
                {
                    Paint_SetPixel(Xpoint + Column, Ypoint + Page, Color_Foreground);
                    // Paint_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                }
                else
                {
                    Paint_SetPixel(Xpoint + Column, Ypoint + Page, Color_Background);
                    // Paint_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Background, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                }
            }
            // One pixel is 8 bits
            if (Column % 8 == 7)
                ptr++;
        } // Write a line
        if (Font->Width % 8 != 0)
            ptr++;
    } // Write all
}

/******************************************************************************
function:	Display the string
parameter:
    Xstart           ：X coordinate
    Ystart           ：Y coordinate
    pString          ：The first address of the English string to be displayed
    Font             ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void Paint_DrawString_EN(UWORD Xstart, UWORD Ystart, const char *pString,
                         u8 size, UWORD Color_Foreground, UWORD Color_Background)
{
		sFONT* Font;

		switch (size) {
				case 8:
						Font = &Font8;
						break;
				case 12:
						Font = &Font12;
						break;
				case 16:
						Font = &Font16;
						break;
				case 20:
						Font = &Font20;
						break;
				case 24:
						Font = &Font24;
						break;
				default:
						// 可以根据需求设置默认的字体，或保留 Font 为 nullptr
						break;
		}
		
    UWORD Xpoint = Xstart;
    UWORD Ypoint = Ystart;
		
    if (Xstart > Paint.Width || Ystart > Paint.Height)
    {
        Debug("Paint_DrawString_EN Input exceeds the normal display range\r\n");
        return;
    }

    while (*pString != '\0')
    {
        // if X direction filled , reposition to(Xstart,Ypoint),Ypoint is Y direction plus the Height of the character
        if ((Xpoint + Font->Width) > Paint.Width)
        {
            Xpoint = Xstart;
            Ypoint += Font->Height;
        }

        // If the Y direction is full, reposition to(Xstart, Ystart)
        if ((Ypoint + Font->Height) > Paint.Height)
        {
            Xpoint = Xstart;
            Ypoint = Ystart;
        }
        Paint_DrawChar(Xpoint, Ypoint, *pString, Font, Color_Background, Color_Foreground);

        // The next character of the address
        pString++;

        // The next word of the abscissa increases the font of the broadband
        Xpoint += Font->Width;
    }
}

/******************************************************************************
function: Display the string
parameter:
    Xstart  ：X coordinate
    Ystart  ：Y coordinate
    pString ：The first address of the Chinese string and English
              string to be displayed
    Font    ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void Paint_DrawString_CN(UWORD Xstart, UWORD Ystart, const char *pString, cFONT *font,
                         UWORD Color_Foreground, UWORD Color_Background)
{
    const char *p_text = pString;
    int x = Xstart, y = Ystart;
    int i, j, Num;

    /* Send the string character by character on EPD */
    while (*p_text != 0)
    {
        if (*p_text <= 0x7F)
        { // ASCII < 126
            for (Num = 0; Num < font->size; Num++)
            {
                if (*p_text == font->table[Num].index[0])
                {
                    const char *ptr = &font->table[Num].matrix[0];

                    for (j = 0; j < font->Height; j++)
                    {
                        for (i = 0; i < font->Width; i++)
                        {
                            if (FONT_BACKGROUND == Color_Background)
                            { // this process is to speed up the scan
                                if (*ptr & (0x80 >> (i % 8)))
                                {
                                    Paint_SetPixel(x + i, y + j, Color_Foreground);
                                    // Paint_DrawPoint(x + i, y + j, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                                }
                            }
                            else
                            {
                                if (*ptr & (0x80 >> (i % 8)))
                                {
                                    Paint_SetPixel(x + i, y + j, Color_Foreground);
                                    // Paint_DrawPoint(x + i, y + j, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                                }
                                else
                                {
                                    Paint_SetPixel(x + i, y + j, Color_Background);
                                    // Paint_DrawPoint(x + i, y + j, Color_Background, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                                }
                            }
                            if (i % 8 == 7)
                            {
                                ptr++;
                            }
                        }
                        if (font->Width % 8 != 0)
                        {
                            ptr++;
                        }
                    }
                    break;
                }
            }
            /* Point on the next character */
            p_text += 1;
            /* Decrement the column position by 16 */
            x += font->ASCII_Width;
        }
        else
        { // Chinese
            for (Num = 0; Num < font->size; Num++)
            {
                if ((*p_text == font->table[Num].index[0]) && (*(p_text + 1) == font->table[Num].index[1]))
                {
                    const char *ptr = &font->table[Num].matrix[0];

                    for (j = 0; j < font->Height; j++)
                    {
                        for (i = 0; i < font->Width; i++)
                        {
                            if (FONT_BACKGROUND == Color_Background)
                            { // this process is to speed up the scan
                                if (*ptr & (0x80 >> (i % 8)))
                                {
                                    Paint_SetPixel(x + i, y + j, Color_Foreground);
                                    // Paint_DrawPoint(x + i, y + j, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                                }
                            }
                            else
                            {
                                if (*ptr & (0x80 >> (i % 8)))
                                {
                                    Paint_SetPixel(x + i, y + j, Color_Foreground);
                                    // Paint_DrawPoint(x + i, y + j, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                                }
                                else
                                {
                                    Paint_SetPixel(x + i, y + j, Color_Background);
                                    // Paint_DrawPoint(x + i, y + j, Color_Background, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                                }
                            }
                            if (i % 8 == 7)
                            {
                                ptr++;
                            }
                        }
                        if (font->Width % 8 != 0)
                        {
                            ptr++;
                        }
                    }
                    break;
                }
            }
            /* Point on the next character */
            p_text += 2;
            /* Decrement the column position by 16 */
            x += font->Width;
        }
    }
}

/******************************************************************************
function:	Display nummber
parameter:
    Xstart           ：X coordinate
    Ystart           : Y coordinate
    Nummber          : The number displayed
    Font             ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
#define ARRAY_LEN 255
void Paint_DrawNum(UWORD Xpoint, UWORD Ypoint, int32_t Nummber,
                   u8 size, UWORD Color_Foreground, UWORD Color_Background)
{

    int16_t Num_Bit = 0, Str_Bit = 0;
    uint8_t Str_Array[ARRAY_LEN] = {0}, Num_Array[ARRAY_LEN] = {0};
    uint8_t *pStr = Str_Array;

    if (Xpoint > Paint.Width || Ypoint > Paint.Height)
    {
        Debug("Paint_DisNum Input exceeds the normal display range\r\n");
        return;
    }

    // Converts a number to a string
    do
    {
        Num_Array[Num_Bit] = Nummber % 10 + '0';
        Num_Bit++;
        Nummber /= 10;
    } while (Nummber);

    // The string is inverted
    while (Num_Bit > 0)
    {
        Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
        Str_Bit++;
        Num_Bit--;
    }

    // show
    Paint_DrawString_EN(Xpoint, Ypoint, (const char *)pStr, size, Color_Background, Color_Foreground);

}

/******************************************************************************
function:	Display nummber (Able to display decimals)
parameter:
    Xstart           ：X coordinate
    Ystart           : Y coordinate
    Nummber          : The number displayed
    Font             ：A structure pointer that displays a character size
    Digit            : Fractional width
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void Paint_DrawNumDecimals(UWORD Xpoint, UWORD Ypoint, double Nummber,
                           u8 size, UWORD Digit, UWORD Color_Foreground, UWORD Color_Background)
{
    int16_t Num_Bit = 0, Str_Bit = 0;
    uint8_t Str_Array[ARRAY_LEN] = {0}, Num_Array[ARRAY_LEN] = {0};
    uint8_t *pStr = Str_Array;
    int temp = Nummber;
    float decimals;
    uint8_t i;
    if (Xpoint > Paint.Width || Ypoint > Paint.Height)
    {
        Debug("Paint_DisNum Input exceeds the normal display range\r\n");
        return;
    }

    if (Digit > 0)
    {
        decimals = Nummber - temp;
        for (i = Digit; i > 0; i--)
        {
            decimals *= 10;
        }
        temp = decimals;
        // Converts a number to a string
        for (i = Digit; i > 0; i--)
        {
            Num_Array[Num_Bit] = temp % 10 + '0';
            Num_Bit++;
            temp /= 10;
        }
        Num_Array[Num_Bit] = '.';
        Num_Bit++;
    }

    temp = Nummber;
    // Converts a number to a string
    do
    {
        Num_Array[Num_Bit] = temp % 10 + '0';
        Num_Bit++;
        temp /= 10;
    } while (temp);

    // The string is inverted
    while (Num_Bit > 0)
    {
        Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
        Str_Bit++;
        Num_Bit--;
    }

    // show
    Paint_DrawString_EN(Xpoint, Ypoint, (const char *)pStr, size, Color_Background, Color_Foreground);
}

/******************************************************************************
function:	Display time
parameter:
    Xstart           ：X coordinate
    Ystart           : Y coordinate
    pTime            : Time-related structures
    Font             ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void Paint_DrawTime(UWORD Xstart, UWORD Ystart, PAINT_TIME *pTime, u8 size,
                    UWORD Color_Foreground, UWORD Color_Background)
{
	
			sFONT* Font;

		switch (size) {
				case 8:
						Font = &Font8;
						break;
				case 12:
						Font = &Font12;
						break;
				case 16:
						Font = &Font16;
						break;
				case 20:
						Font = &Font20;
						break;
				case 24:
						Font = &Font24;
						break;
				default:
						// 可以根据需求设置默认的字体，或保留 Font 为 nullptr
						break;
		}
		
    uint8_t value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    UWORD Dx = Font->Width;

    // Write data into the cache
    Paint_DrawChar(Xstart, Ystart, value[pTime->Hour / 10], Font, Color_Background, Color_Foreground);
    Paint_DrawChar(Xstart + Dx, Ystart, value[pTime->Hour % 10], Font, Color_Background, Color_Foreground);
    Paint_DrawChar(Xstart + Dx + Dx / 4 + Dx / 2, Ystart, ':', Font, Color_Background, Color_Foreground);
    Paint_DrawChar(Xstart + Dx * 2 + Dx / 2, Ystart, value[pTime->Min / 10], Font, Color_Background, Color_Foreground);
    Paint_DrawChar(Xstart + Dx * 3 + Dx / 2, Ystart, value[pTime->Min % 10], Font, Color_Background, Color_Foreground);
    Paint_DrawChar(Xstart + Dx * 4 + Dx / 2 - Dx / 4, Ystart, ':', Font, Color_Background, Color_Foreground);
    Paint_DrawChar(Xstart + Dx * 5, Ystart, value[pTime->Sec / 10], Font, Color_Background, Color_Foreground);
    Paint_DrawChar(Xstart + Dx * 6, Ystart, value[pTime->Sec % 10], Font, Color_Background, Color_Foreground);
}

/******************************************************************************
function:	Display monochrome bitmap
parameter:
    image_buffer ：A picture data converted to a bitmap
info:
    Use a computer to convert the image into a corresponding array,
    and then embed the array directly into Imagedata.cpp as a .c file.
******************************************************************************/
void Paint_DrawBitMap(const unsigned char *image_buffer)
{
    UWORD x, y;
    UDOUBLE Addr = 0;

    for (y = 0; y < Paint.HeightByte; y++)
    {
        for (x = 0; x < Paint.WidthByte; x++)
        { // 8 pixel =  1 byte
            Addr = x + y * Paint.WidthByte;
            Paint.Image[Addr] = (unsigned char)image_buffer[Addr];
        }
    }
}

/******************************************************************************
function:	paste monochrome bitmap to a frame buff
parameter:
    image_buffer ：A picture data converted to a bitmap
    xStart: The starting x coordinate
    yStart: The starting y coordinate
    imageWidth: Original image width
    imageHeight: Original image height
    flipColor: Whether the color is reversed
info:
    Use this function to paste image data into a buffer
******************************************************************************/
void Paint_DrawBitMap_Paste(const unsigned char *image_buffer, UWORD xStart, UWORD yStart, UWORD imageWidth, UWORD imageHeight, UBYTE flipColor)
{
    UBYTE color, srcImage;
    UWORD x, y;
    UWORD width = (imageWidth % 8 == 0 ? imageWidth / 8 : imageWidth / 8 + 1);

    for (y = 0; y < imageHeight; y++)
    {
        for (x = 0; x < imageWidth; x++)
        {
            srcImage = image_buffer[y * width + x / 8];
            if (flipColor)
                color = (((srcImage << (x % 8) & 0x80) == 0) ? 1 : 0);
            else
                color = (((srcImage << (x % 8) & 0x80) == 0) ? 0 : 1);
            Paint_SetPixel(x + xStart, y + yStart, color);
        }
    }
}

///******************************************************************************
// function:	SDisplay half of monochrome bitmap
// parameter:
//	Region : 1 Upper half
//					 2 Lower half
// info:
//******************************************************************************/
// void Paint_DrawBitMap_Half(const unsigned char* image_buffer, UBYTE Region)
//{
//     UWORD x, y;
//     UDOUBLE Addr = 0;
//
//		if(Region == 1){
//			for (y = 0; y < Paint.HeightByte; y++) {
//					for (x = 0; x < Paint.WidthByte; x++) {//8 pixel =  1 byte
//							Addr = x + y * Paint.WidthByte;
//							Paint.Image[Addr] = (unsigned char)image_buffer[Addr];
//					}
//			}
//		}else{
//			for (y = 0; y < Paint.HeightByte; y++) {
//					for (x = 0; x < Paint.WidthByte; x++) {//8 pixel =  1 byte
//							Addr = x + y * Paint.WidthByte ;
//							Paint.Image[Addr] = \
//							(unsigned char)image_buffer[Addr+ (Paint.HeightByte)*Paint.WidthByte];
//					}
//			}
//		}
// }

///******************************************************************************
// function:	SDisplay half of monochrome bitmap
// parameter:
//	Region : 1 Upper half
//					 2 Lower half
// info:
//******************************************************************************/
// void Paint_DrawBitMap_OneQuarter(const unsigned char* image_buffer, UBYTE Region)
//{
//     UWORD x, y;
//     UDOUBLE Addr = 0;
//
//		if(Region == 1){
//			for (y = 0; y < Paint.HeightByte; y++) {
//					for (x = 0; x < Paint.WidthByte; x++) {//8 pixel =  1 byte
//							Addr = x + y * Paint.WidthByte;
//							Paint.Image[Addr] = (unsigned char)image_buffer[Addr];
//					}
//			}
//		}else if(Region == 2){
//			for (y = 0; y < Paint.HeightByte; y++) {
//					for (x = 0; x < Paint.WidthByte; x++) {//8 pixel =  1 byte
//							Addr = x + y * Paint.WidthByte ;
//							Paint.Image[Addr] = \
//							(unsigned char)image_buffer[Addr+ (Paint.HeightByte)*Paint.WidthByte];
//					}
//			}
//		}else if(Region == 3){
//			for (y = 0; y < Paint.HeightByte; y++) {
//					for (x = 0; x < Paint.WidthByte; x++) {//8 pixel =  1 byte
//							Addr = x + y * Paint.WidthByte ;
//							Paint.Image[Addr] = \
//							(unsigned char)image_buffer[Addr+ (Paint.HeightByte)*Paint.WidthByte*2];
//					}
//			}
//		}else if(Region == 4){
//			for (y = 0; y < Paint.HeightByte; y++) {
//					for (x = 0; x < Paint.WidthByte; x++) {//8 pixel =  1 byte
//							Addr = x + y * Paint.WidthByte ;
//							Paint.Image[Addr] = \
//							(unsigned char)image_buffer[Addr+ (Paint.HeightByte)*Paint.WidthByte*3];
//					}
//			}
//		}
// }

void Paint_DrawBitMap_Block(const unsigned char *image_buffer, UBYTE Region)
{
    UWORD x, y;
    UDOUBLE Addr = 0;
    for (y = 0; y < Paint.HeightByte; y++)
    {
        for (x = 0; x < Paint.WidthByte; x++)
        { // 8 pixel =  1 byte
            Addr = x + y * Paint.WidthByte;
            Paint.Image[Addr] =
                (unsigned char)image_buffer[Addr + (Paint.HeightByte) * Paint.WidthByte * (Region - 1)];
        }
    }
}


//绘制直线
void Paint_Show_Line(u16 x_start, u16 y_start, u16 x_end, u16 y_end, u16 color)
{
    int dx = abs(x_end - x_start), sx = x_start < x_end ? 1 : -1;
    int dy = -abs(y_end - y_start), sy = y_start < y_end ? 1 : -1;
    int err = dx + dy, e2;
		UWORD Color;
		if(color) Color = EPD_BLACK;
		else Color = EPD_WHITE;
	
    while (1) {
        Paint_SetPixel(x_start, y_start, Color);
        if (x_start == x_end && y_start == y_end) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x_start += sx; }
        if (e2 <= dx) { err += dx; y_start += sy; }
    }
}
//绘制矩形
void Paint_Show_Rectangle(u16 x_start, u16 y_start, u16 x_end, u16 y_end,u8 thickness, u16 color, u8 filled)
{
    if (filled) {
        for (u16 y = y_start; y <= y_end; y++) {
            Paint_Show_Line(x_start, y, x_end, y, color);
        }
    } else {
			for(;thickness>=1;thickness--)
			{
				Paint_Show_Line(x_start, y_start, x_end, y_start, color);  // Top edge
        Paint_Show_Line(x_start, y_end, x_end, y_end, color);      // Bottom edge
        Paint_Show_Line(x_start, y_start, x_start, y_end, color);  // Left edge
        Paint_Show_Line(x_end, y_start, x_end, y_end, color);      // Right edge
				x_start++;
				y_start++;
				x_end--;
				y_end--;
			}

    }
}


// 绘制圆
void Paint_Show_Circle(u16 x_center, u16 y_center, u16 radius, u8 thickness, u16 color, u8 filled)
{
    int x = radius;
    int y = 0;
    int err = 0;

    UWORD Color;
    if (color) Color = EPD_BLACK;
    else Color = EPD_WHITE;

    if (thickness == 1) {
        // 绘制单个圆
        while (x >= y) {
            if (filled) {
                // 绘制填充圆，画四条水平和垂直的线段
                Paint_Show_Line(x_center - x, y_center + y, x_center + x, y_center + y, color);
                Paint_Show_Line(x_center - y, y_center + x, x_center + y, y_center + x, color);
                Paint_Show_Line(x_center - x, y_center - y, x_center + x, y_center - y, color);
                Paint_Show_Line(x_center - y, y_center - x, x_center + y, y_center - x, color);
            } else {
                // 绘制空心圆
                Paint_SetPixel(x_center + x, y_center + y, Color);
                Paint_SetPixel(x_center + y, y_center + x, Color);
                Paint_SetPixel(x_center - y, y_center + x, Color);
                Paint_SetPixel(x_center - x, y_center + y, Color);
                Paint_SetPixel(x_center - x, y_center - y, Color);
                Paint_SetPixel(x_center - y, y_center - x, Color);
                Paint_SetPixel(x_center + y, y_center - x, Color);
                Paint_SetPixel(x_center + x, y_center - y, Color);
            }
            y += 1;
            err += 1 + 2 * y;
            if (2 * (err - x) + 1 > 0) {
                x -= 1;
                err += 1 - 2 * x;
            }
        }
    } else {
        // 绘制具有指定厚度的圆
        for (u8 t = 0; t < thickness; t++) {
            // 计算当前半径（逐渐减少）
            u16 current_radius = radius - t;

            // 绘制当前半径的圆
            x = current_radius;
            y = 0;
            err = 0;

            while (x >= y) {
                if (filled) {
                    // 绘制填充圆，画四条水平和垂直的线段
                    Paint_Show_Line(x_center - x, y_center + y, x_center + x, y_center + y, color);
                    Paint_Show_Line(x_center - y, y_center + x, x_center + y, y_center + x, color);
                    Paint_Show_Line(x_center - x, y_center - y, x_center + x, y_center - y, color);
                    Paint_Show_Line(x_center - y, y_center - x, x_center + y, y_center - x, color);
                } else {
                    // 绘制空心圆
                    Paint_SetPixel(x_center + x, y_center + y, Color);
                    Paint_SetPixel(x_center + y, y_center + x, Color);
                    Paint_SetPixel(x_center - y, y_center + x, Color);
                    Paint_SetPixel(x_center - x, y_center + y, Color);
                    Paint_SetPixel(x_center - x, y_center - y, Color);
                    Paint_SetPixel(x_center - y, y_center - x, Color);
                    Paint_SetPixel(x_center + y, y_center - x, Color);
                    Paint_SetPixel(x_center + x, y_center - y, Color);
                }
                y += 1;
                err += 1 + 2 * y;
                if (2 * (err - x) + 1 > 0) {
                    x -= 1;
                    err += 1 - 2 * x;
                }
            }
        }
    }
}


//绘制椭圆
void Paint_Show_Ellipse(u16 x_center, u16 y_center, u16 x_radius, u16 y_radius, u16 color, u8 filled)
{
		UWORD Color;
		if(color) Color = EPD_BLACK;
		else Color = EPD_WHITE;
	
    int x = x_radius;
    int y = 0;
    
    long x_radius_sq = (long)x_radius * x_radius;
    long y_radius_sq = (long)y_radius * y_radius;
    long two_x_radius_sq = 2 * x_radius_sq;
    long two_y_radius_sq = 2 * y_radius_sq;

    long x_change = y_radius_sq * (1 - 2 * x_radius);
    long y_change = x_radius_sq;
    long error = 0;
    long stopping_x = two_y_radius_sq * x_radius;
    long stopping_y = 0;

    // 绘制第一区域：从顶部到四分之一处
    while (stopping_x >= stopping_y) {
        if (filled) {
            Paint_Show_Line(x_center - x, y_center + y, x_center + x, y_center + y, color);
            Paint_Show_Line(x_center - x, y_center - y, x_center + x, y_center - y, color);
        } else {
            Paint_SetPixel(x_center + x, y_center + y, Color);
            Paint_SetPixel(x_center - x, y_center + y, Color);
            Paint_SetPixel(x_center + x, y_center - y, Color);
            Paint_SetPixel(x_center - x, y_center - y, Color);
        }
        y++;
        stopping_y += two_x_radius_sq;
        error += y_change;
        y_change += two_x_radius_sq;

        if (2 * error + x_change > 0) {
            x--;
            stopping_x -= two_y_radius_sq;
            error += x_change;
            x_change += two_y_radius_sq;
        }
    }

    // 重置变量以处理第二区域：从四分之一到另一侧的顶点
    x = 0;
    y = y_radius;
    x_change = y_radius_sq;
    y_change = x_radius_sq * (1 - 2 * y_radius);
    error = 0;
    stopping_x = 0;
    stopping_y = two_x_radius_sq * y_radius;

    // 绘制第二区域：从四分之一到另一顶点
    while (stopping_x <= stopping_y) {
        if (filled) {
            Paint_Show_Line(x_center - x, y_center + y, x_center + x, y_center + y, color);
            Paint_Show_Line(x_center - x, y_center - y, x_center + x, y_center - y, color);
        } else {
            Paint_SetPixel(x_center + x, y_center + y, Color);
            Paint_SetPixel(x_center - x, y_center + y, Color);
            Paint_SetPixel(x_center + x, y_center - y, Color);
            Paint_SetPixel(x_center - x, y_center - y, Color);
        }
        x++;
        stopping_x += two_y_radius_sq;
        error += x_change;
        x_change += two_y_radius_sq;

        if (2 * error + y_change > 0) {
            y--;
            stopping_y -= two_x_radius_sq;
            error += y_change;
            y_change += two_x_radius_sq;
        }
    }
}



//void Paint_Show_Arc(u16 x_center, u16 y_center, u16 radius, u16 start_angle, u16 end_angle, u8 thickness, u16 color, u8 filled)
//{
//	
//		UWORD Color;
//		if(color) Color = EPD_BLACK;
//		else Color = EPD_WHITE;
//   // 使用 STM32 DSP 库的 arm_cos_f32 和 arm_sin_f32 函数来加速三角函数运算
//    float radian_start = start_angle * 3.14159f / 180.0f;
//    float radian_end = end_angle * 3.14159f / 180.0f;
//    
//    // 设置步进大小，控制绘制的平滑度
//    float angle_step = 0.002f; // 每次递增的弧度，值越小越平滑

//    // 初始位置
//    float x_prev = x_center + (radius * arm_cos_f32(radian_start));
//    float y_prev = y_center + (radius * arm_sin_f32(radian_start));

//    // 绘制弧线
//    for (float angle = radian_start + angle_step; angle <= radian_end; angle += angle_step) {
//        // 使用 STM32 DSP 库的 arm_cos_f32 和 arm_sin_f32 来计算圆上点的坐标
//        float x = x_center + (radius * arm_cos_f32(angle));
//        float y = y_center + (radius * arm_sin_f32(angle));

//        // 使用画线函数连接相邻的点，形成弧线
//        Paint_Show_Line(x_prev, y_prev, x, y, color);

//        // 更新前一个点的坐标
//        x_prev = x;
//        y_prev = y;
//    }
//}		

void Paint_Show_Arc(u16 x_center, u16 y_center, u16 radius, u16 start_angle, u16 end_angle, u8 thickness, u16 color, u8 filled)
{

    // 将角度转换为弧度
    float radian_start = start_angle * 3.14159 / 180.0;
    float radian_end = end_angle * 3.14159 / 180.0;

    // 设置步进大小，控制绘制的平滑度
    float angle_step = 0.002; // 每次递增的弧度，值越小越平滑

    // 初始位置
    u16 x_prev = x_center + (int)(radius * arm_cos_f32(radian_start));
    u16 y_prev = y_center + (int)(radius * arm_sin_f32(radian_start));

    // 绘制弧线
    for (float angle = radian_start + angle_step; angle <= radian_end; angle += angle_step) {
        // 使用三角函数计算圆上点的坐标
        u16 x = x_center + (int)(radius * arm_cos_f32(angle));
        u16 y = y_center + (int)(radius * arm_sin_f32(angle));

        // 使用画线函数连接相邻的点，形成弧线
        Paint_Show_Line(x_prev, y_prev, x, y, color);

        // 更新前一个点的坐标
        x_prev = x;
        y_prev = y;
    }

    // 如果是填充的弧线（扇形），绘制从圆心到弧线的线段
    if (filled) {
        for (float angle = radian_start + angle_step; angle <= radian_end; angle += angle_step) {
            u16 x = x_center + (int)(radius * arm_cos_f32(angle));
            u16 y = y_center + (int)(radius * arm_sin_f32(angle));
            Paint_Show_Line(x_center, y_center, x, y, color);
        }
    }

    // 如果需要更宽的弧线（厚度），可以重复绘制不同半径的弧线（向内绘制）
    if (thickness > 1) {
        for (u8 i = 1; i < thickness; i++) {
            u16 offset_radius = radius - i; // 使用减法来确保厚度向内扩展
            x_prev = x_center + (int)(offset_radius * arm_cos_f32(radian_start));
            y_prev = y_center + (int)(offset_radius * arm_sin_f32(radian_start));

            // 绘制具有不同半径的弧线（将弧线向内绘制）
            for (float angle = radian_start + angle_step; angle <= radian_end; angle += angle_step) {
                u16 x = x_center + (int)(offset_radius * arm_cos_f32(angle));
                u16 y = y_center + (int)(offset_radius * arm_sin_f32(angle));
                Paint_Show_Line(x_prev, y_prev, x, y, color);
                x_prev = x;
                y_prev = y;
            }

            // 如果是填充的弧线，绘制扇形
            if (filled) {
                for (float angle = radian_start + angle_step; angle <= radian_end; angle += angle_step) {
                    u16 x = x_center + (int)(offset_radius * arm_cos_f32(angle));
                    u16 y = y_center + (int)(offset_radius * arm_sin_f32(angle));
                    Paint_Show_Line(x_center, y_center, x, y, color);
                }
            }
        }
    }
}

//    // 如果是填充的弧线（扇形），绘制从圆心到弧线的线段
//    if (filled) {
//        for (float angle = radian_start + angle_step; angle <= radian_end; angle += angle_step) {
//            float x = x_center + (radius * arm_cos_f32(angle));
//            float y = y_center + (radius * arm_sin_f32(angle));
//            Paint_Show_Line(x_center, y_center, x, y, color);
//        }
//    }

//    // 如果需要更宽的弧线（厚度），可以重复绘制不同半径的弧线（向内绘制）
//    if (thickness > 1) {
//        for (u8 i = 1; i < thickness; i++) {
//            float offset_radius = radius - i; // 使用减法来确保厚度向内扩展
//            x_prev = x_center + (offset_radius * arm_cos_f32(radian_start));
//            y_prev = y_center + (offset_radius * arm_sin_f32(radian_start));

//            // 绘制具有不同半径的弧线（将弧线向内绘制）
//            for (float angle = radian_start + angle_step; angle <= radian_end; angle += angle_step) {
//                float x = x_center + (offset_radius * arm_cos_f32(angle));
//                float y = y_center + (offset_radius * arm_sin_f32(angle));
//                Paint_Show_Line(x_prev, y_prev, x, y, color);
//                x_prev = x;
//                y_prev = y;
//            }

//            // 如果是填充的弧线，绘制扇形
//            if (filled) {
//                for (float angle = radian_start + angle_step; angle <= radian_end; angle += angle_step) {
//                    float x = x_center + (offset_radius * arm_cos_f32(angle));
//                    float y = y_center + (offset_radius * arm_sin_f32(angle));
//                    Paint_Show_Line(x_center, y_center, x, y, color);
//                }
//            }
//        }
//    }





//绘制圆角矩形
void Paint_Show_RoundRect(u16 x_start, u16 y_start, u16 x_end, u16 y_end, u16 radius, u8 thickness, u16 color, u8 filled)
{
    // 确保矩形的宽度和高度允许绘制圆角
    if ((x_end - x_start < 2 * radius) || (y_end - y_start < 2 * radius)) {
        return; // 矩形太小，无法绘制圆角
    }

    if (filled) {
				Paint_Show_Rectangle(x_start,y_start+radius, x_end,y_end-radius, thickness, color,1);
				Paint_Show_Rectangle(x_start+radius,y_start,x_end-radius,y_start+radius,thickness,  color,1);
				Paint_Show_Rectangle(x_start+radius,y_end-radius, x_end-radius,y_end, thickness, color,1);

        // 绘制圆角部分的四个圆弧
        Paint_Show_Arc(x_start + radius, y_start + radius, radius, 180, 270, 1,color,1); // 左上
        Paint_Show_Arc(x_end - radius, y_start + radius, radius, 270, 360, 1,color,1);   // 右上
        Paint_Show_Arc(x_start + radius, y_end - radius, radius, 90, 180, 1,color,1);    // 左下
        Paint_Show_Arc(x_end - radius, y_end - radius, radius, 0, 90, 1,color,1);       // 右下
    } else {
        // 绘制空心矩形的边界（去除圆角部分）
				Paint_Show_Rectangle(x_start + radius, y_start, x_end - radius, y_start+thickness,thickness,color,1);
				Paint_Show_Rectangle(x_start + radius, y_end-thickness, x_end - radius, y_end,thickness,color,1);	
				Paint_Show_Rectangle(x_start, y_start+radius, x_start+thickness, y_end- radius,thickness,color,1);
				Paint_Show_Rectangle(x_end, y_start+radius, x_end-thickness, y_end- radius,thickness,color,1);

        // 绘制圆角部分的四个圆弧
        Paint_Show_Arc(x_start + radius, y_start + radius, radius, 180, 270, thickness,color,0); // 左上
        Paint_Show_Arc(x_end - radius, y_start + radius, radius, 270, 360, thickness,color,0);   // 右上
        Paint_Show_Arc(x_start + radius, y_end - radius, radius, 90, 180, thickness,color,0);    // 左下
        Paint_Show_Arc(x_end - radius, y_end - radius, radius, 0, 90, thickness,color,0);       // 右下
    }
}

//绘制三角形
void Paint_Show_Triangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 color, u8 filled)
{
    if (filled) {
        // 绘制填充三角形
        for (int y = y0; y <= y2; y++) {
            int x_start = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            int x_end = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            Paint_Show_Line(x_start, y, x_end, y, color);
        }
    } else {
        // 绘制空心三角形的三条边
        Paint_Show_Line(x0, y0, x1, y1, color);
        Paint_Show_Line(x1, y1, x2, y2, color);
        Paint_Show_Line(x2, y2, x0, y0, color);
    }
}




void Paint_Get_HzMat(unsigned char *code,unsigned char *mat,u8 size)
{		    
	unsigned char qh,ql;
	unsigned char i;					  
	unsigned long foffset; 
	u8 csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数 
	qh=*code;
	ql=*(++code);
	if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//非 常用汉字
	{   		    
	    for(i=0;i<csize;i++)*mat++=0x00;//填充满格
	    return; //结束访问
	}          
	if(ql<0x7f)ql-=0x40;//注意!
	else ql-=0x41;
	qh-=0x81;   
	foffset=((unsigned long)190*qh+ql)*csize;	//得到字库中的字节偏移量  		  
	switch(size)
	{
		case 12:
			W25QXX_Read(mat,foffset+ftinfo.f12addr,csize);
			break;
		case 16:
			W25QXX_Read(mat,foffset+ftinfo.f16addr,csize);
			break;
		case 24:
			W25QXX_Read(mat,foffset+ftinfo.f24addr,csize);
			break;
			
	}     												    
}  

//显示一个指定大小的英文字符
//x,y :字符的坐标
//Acsii_Char:字符
//size:字体大小
//mode:1,黑字;0,白字  
//cover:1,覆盖;0,不覆盖
void Paint_Show_Char(u16 x,u16 y,const char Acsii_Char,u8 size,u8 mode,u8 cover)
{
		sFONT* Font;
		switch (size) {
				case 8:
						Font = &Font8;
						break;
				case 12:
						Font = &Font12;
						break;
				case 16:
						Font = &Font16;
						break;
				case 20:
						Font = &Font20;
						break;
				case 24:
						Font = &Font24;
						break;
				default:
						break;
		}
		UWORD Page, Column;

    if (x > Paint.Width || y > Paint.Height)
    {
        Debug("Paint_DrawChar Input exceeds the normal display range\r\n");
        return;
    }

    uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height * (Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &Font->table[Char_Offset];

    for (Page = 0; Page < Font->Height; Page++)
    {
        for (Column = 0; Column < Font->Width; Column++)
        {
            uint8_t temp = *ptr & (0x80 >> (Column % 8));  // 当前位的像素状态
            if (mode == 0)
            {
                // 标准显示
                if (cover == 1 && temp == 0)  // 背景色条件
                    Paint_SetPixel(x + Column, y + Page, EPD_BLACK);
                if (temp)  // 前景色条件
                    Paint_SetPixel(x + Column, y + Page, EPD_WHITE);
            }
            else
            {
                // 反转显示
                if (cover == 1 && temp == 0)  // 反转的背景色条件
                    Paint_SetPixel(x + Column, y + Page, EPD_WHITE);
                if (temp)  // 反转的前景色条件
                    Paint_SetPixel(x + Column, y + Page, EPD_BLACK);
            }

            // 每8列后更新指针
            if (Column % 8 == 7)
                ptr++;
        }
        // 如果字体宽度不是8的倍数，则处理每行未满的字节
        if (Font->Width % 8 != 0)
            ptr++;
    }
//	Paint_DrawChar(x,y,*str,Font,EPD_BLACK,EPD_WHITE);
	
}
//显示一个指定大小的汉字
//x,y :汉字的坐标
//font:汉字GBK码
//size:字体大小
//mode:1,黑字;0,白字 
//cover:1,覆盖;0,不覆盖
void Paint_Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode,u8 cover)
{
	u8 temp,t,t1;
	u16 y0=y;
	u8 dzk[72];   
	u8 csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数	 
	if(size!=12&&size!=16&&size!=24)return;	//不支持的size
	Paint_Get_HzMat(font,dzk,size);	//得到相应大小的点阵数据 
	for(t=0;t<csize;t++)
	{   												   
		temp=dzk[t];			//得到点阵数据                          
		for(t1=0;t1<8;t1++)
		{
			if(mode==0)
			{
				if(cover==1)Paint_SetPixel(x,y,EPD_BLACK);
				if (temp&0x80)Paint_SetPixel(x,y,EPD_WHITE);
			}
			else
			{
				if(cover==1)Paint_SetPixel(x,y,EPD_WHITE);
				if (temp&0x80)Paint_SetPixel(x,y,EPD_BLACK);
			}
//				if(temp&0x80)Paint_SetPixel(x,y,EPD_BLACK);
//				else if (mode == 0)Paint_SetPixel(x,y,EPD_WHITE);
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
	}  
}

//在指定位置开始显示一个字符串	    
//支持自动换行
//(x,y):起始坐标
//str  :字符串
//size :字体大小
//mode:1,黑字;0,白字  
//cover:1,覆盖;0,不覆盖
void Paint_Show_Str(u16 x,u16 y,u8*str,u8 size,u8 mode,u8 cover)
{			
	u16 width;
	u16 height;
	width = size*strlen((const char*)str);;
	height = size;
	u16 x0=x;
	u16 y0=y;							  	  
	u8 bHz=0;     //字符或者中文  	    				    				  	  
    while(*str!=0)//数据未结束
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//中文 
	        else              //字符
	        {      
                if(x>(x0+width-size/2))//换行
				{				   
					y+=size;
					x=x0;	   
				}							    
		        if(y>(y0+height-size))break;//越界返回      
		        if(*str==13)//换行符号
		        {         
		            y+=size;
					x=x0;
		            str++; 
		        }  
//						Paint_Show_Char(u16 x,u16 y,const char Acsii_Char,u8 size,u8 mode,u8 cover)
		        else Paint_Show_Char(x,y,*str,size,mode,cover);//有效部分写入 
				str++; 
		        x+=size/2; //字符,为全字的一半 
	        }
        }else//中文 
        {     
            bHz=0;//有汉字库    
            if(x>(x0+width-size))//换行
			{	    
				y+=size;
				x=x0;		  
			}
	        if(y>(y0+height-size))break;//越界返回  						     
	        Paint_Show_Font(x,y,str,size,mode,cover); //显示这个汉字,空心显示 
	        str+=2; 
	        x+=size;//下一个汉字偏移	    
        }						 
    }   
}  		

//在指定位置开始显示数字	    
//(x,y):起始坐标
//nummber 数字
//size :字体大小
//mode:1,黑字;0,白字 
//cover:1,覆盖;0,不覆盖
void Paint_Show_xNum(u16 x, u16 y, s32 nummber, u8 size, u8 mode, u8 cover)
{
    int16_t Num_Bit = 0, Str_Bit = 0;
    uint8_t Str_Array[ARRAY_LEN] = {0}, Num_Array[ARRAY_LEN] = {0};
    uint8_t *pStr = Str_Array;
    int is_negative = 0;

    if (x > Paint.Width || y > Paint.Height)
    {
        Debug("Paint_DisNum Input exceeds the normal display range\r\n");
        return;
    }

    // 检查是否为负数
    if (nummber < 0)
    {
        is_negative = 1;
        nummber = -nummber;  // 转为正数处理
    }

    // 将数字转换为字符串
    do
    {
        Num_Array[Num_Bit] = nummber % 10 + '0';
        Num_Bit++;
        nummber /= 10;
    } while (nummber);

    // 如果是负数，加上负号
    if (is_negative)
    {
        Num_Array[Num_Bit] = '-';
        Num_Bit++;
    }

    // 字符数组反转，得到正确的显示顺序
    while (Num_Bit > 0)
    {
        Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
        Str_Bit++;
        Num_Bit--;
    }

    // 显示最终的字符串
    Paint_Show_Str(x, y, pStr, size, mode, cover);
}

//在指定位置开始显示小数	    
//(x,y):起始坐标
//nummber 小数
//size :字体大小
//digit:显示小数后多少位
//mode:1,黑字;0,白字 
//cover:1,覆盖;0,不覆盖
void Paint_Show_xDecnum(u16 x, u16 y, double nummber, u8 size, u8 digit, u8 mode, u8 cover)
{
    int16_t Num_Bit = 0, Str_Bit = 0;
    uint8_t Str_Array[ARRAY_LEN] = {0}, Num_Array[ARRAY_LEN] = {0};
    uint8_t *pStr = Str_Array;
    int is_negative = 0;
    int integer_part;
    int decimal_part;
    uint8_t i;

    if (x > Paint.Width || y > Paint.Height)
    {
        Debug("Paint_Show_xDecnum Input exceeds the normal display range\r\n");
        return;
    }

    // 检查是否为负数
    if (nummber < 0)
    {
        is_negative = 1;
        nummber = -nummber;  // 将数值转换为正数来处理
    }

    // 提取整数部分和小数部分
    integer_part = (int)nummber;

    if (digit > 0)
    {
        // 将小数部分放大为整数
        double scale = 1;
        for (i = 0; i < digit; i++)
            scale *= 10;

        decimal_part = (int)((nummber - integer_part) * scale + 0.5);  // 四舍五入

        // 转换小数部分为字符串
        for (i = digit; i > 0; i--)
        {
            Num_Array[Num_Bit] = decimal_part % 10 + '0';
            Num_Bit++;
            decimal_part /= 10;
        }
        Num_Array[Num_Bit] = '.';
        Num_Bit++;
    }

    // 转换整数部分为字符串
    int temp = integer_part;
    do
    {
        Num_Array[Num_Bit] = temp % 10 + '0';
        Num_Bit++;
        temp /= 10;
    } while (temp);

    // 如果是负数，加上负号
    if (is_negative)
    {
        Num_Array[Num_Bit] = '-';
        Num_Bit++;
    }

    // 将字符数组反转，以正确的顺序显示
    while (Num_Bit > 0)
    {
        Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
        Str_Bit++;
        Num_Bit--;
    }

    // 显示最终的字符串
    Paint_Show_Str(x, y, pStr, size, mode, cover);
}



