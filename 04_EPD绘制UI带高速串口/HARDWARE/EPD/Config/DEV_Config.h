
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include "sys.h"
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"

/**
 * data
 **/
#define UBYTE uint8_t
#define UWORD uint16_t
#define UDOUBLE uint32_t

#define F_EPD_RST_PIN PCout(0)
#define F_EPD_DC_PIN PCout(1)
#define F_EPD_PWR_PIN PCout(5)
#define F_EPD_CS_PIN PCout(4)
#define F_EPD_BUSY_PIN PCout(2)
#define F_EPD_MOSI_PIN PCout(3)
#define F_EPD_SCLK_PIN PBout(10)
/**
 * e-Paper GPIO
 **/
#define EPD_RST_PIN GPIOC, GPIO_Pin_0
#define EPD_DC_PIN GPIOC, GPIO_Pin_1
#define EPD_PWR_PIN GPIOC, GPIO_Pin_5
#define EPD_CS_PIN GPIOC, GPIO_Pin_4
#define EPD_BUSY_PIN GPIOC, GPIO_Pin_2
#define EPD_MOSI_PIN GPIOC, GPIO_Pin_3
#define EPD_SCLK_PIN GPIOB, GPIO_Pin_10

/**
 * GPIO read and write
 **/
// #define DEV_Digital_Write(_pin, _value) HAL_GPIO_WritePin(_pin, _value == 0? GPIO_PIN_RESET:GPIO_PIN_SET)
// #define DEV_Digital_Read(_pin) HAL_GPIO_ReadPin(_pin)
#define DEV_Digital_Write(_pin, _value) ((_value) == 0 ? GPIO_ResetBits(_pin) : GPIO_SetBits(_pin))
#define DEV_Digital_Read(_pin) GPIO_ReadInputDataBit(_pin)

/**
 * delay x ms
 **/
#define DEV_Delay_ms(__xms) delay_ms(__xms);

void EPD_GPIO_Init(void);

u8 DEV_SPI_WriteByte(UBYTE value);
void DEV_SPI_Write_nByte(UBYTE *value, UDOUBLE len);

int DEV_Module_Init(void);
void DEV_Module_Exit(void);
void DEV_GPIO_Init(void);
void DEV_SPI_Init(void);
void DEV_SPI_SendData(UBYTE Reg);
UBYTE DEV_SPI_ReadData(void);
#endif
