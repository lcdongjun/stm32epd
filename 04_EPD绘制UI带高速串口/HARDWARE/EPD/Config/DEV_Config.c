#include "DEV_Config.h"

void EPD_GPIO_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // 使能GPIOC时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // 普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 浮空
	GPIO_Init(GPIOC, &GPIO_InitStructure);			   // 初始化GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	 // 输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 浮空
	GPIO_Init(GPIOC, &GPIO_InitStructure);			 // 初始化GPIO
}
u8 DEV_SPI_WriteByte(UBYTE value)
{

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
	{
	}							   // 等待发送区空
	SPI_I2S_SendData(SPI2, value); // 通过外设SPIx发送一个byte  数据
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
	{
	} // 等待接收完一个byte
	return SPI_I2S_ReceiveData(SPI2);
}

void DEV_SPI_Write_nByte(UBYTE *value, UDOUBLE len)
{
	uint8_t data;
	for (; len > 0; len--)
	{
		data = *value;			 // 读取当前字节
		DEV_SPI_WriteByte(data); // 发送字节
		value++;				 // 将指针移动到下一个字节
	}
}

void DEV_GPIO_Init()
{
	//    HAL_SPI_MspDeInit(&hspi2);

	// HAL_SPI_DeInit(&hspi1);
	//    __HAL_RCC_SPI1_CLK_DISABLE();
	//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
}

void DEV_SPI_Init()
{
	//    HAL_SPI_MspInit(&hspi2);
	// HAL_SPI_DeInit(&hspi1);
	//    __HAL_RCC_SPI1_CLK_DISABLE();
	//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
}

void DEV_SPI_SendData(UBYTE Reg)
{
	UBYTE i, j = Reg;

	F_EPD_MOSI_PIN = 1;
	F_EPD_SCLK_PIN = 1;
	DEV_Digital_Write(EPD_CS_PIN, 0);
	for (i = 0; i < 8; i++)
	{
		DEV_Digital_Write(EPD_SCLK_PIN, 0);
		if (j & 0x80)
		{
			DEV_Digital_Write(EPD_MOSI_PIN, 1);
		}
		else
		{
			DEV_Digital_Write(EPD_MOSI_PIN, 0);
		}

		DEV_Digital_Write(EPD_SCLK_PIN, 1);
		j = j << 1;
	}
	DEV_Digital_Write(EPD_SCLK_PIN, 0);
	DEV_Digital_Write(EPD_CS_PIN, 1);
}

UBYTE DEV_SPI_ReadData()
{
	UBYTE i, j = 0xff;

	F_EPD_MOSI_PIN = 0;
	F_EPD_SCLK_PIN = 1;
	DEV_Digital_Write(EPD_CS_PIN, 0);
	for (i = 0; i < 8; i++)
	{
		DEV_Digital_Write(EPD_SCLK_PIN, 0);
		j = j << 1;
		if (DEV_Digital_Read(EPD_MOSI_PIN))
		{
			j = j | 0x01;
		}
		else
		{
			j = j & 0xfe;
		}
		DEV_Digital_Write(EPD_SCLK_PIN, 1);
	}
	DEV_Digital_Write(EPD_SCLK_PIN, 0);
	DEV_Digital_Write(EPD_CS_PIN, 1);
	return j;
}

int DEV_Module_Init(void)
{
	DEV_Digital_Write(EPD_DC_PIN, 0);
	DEV_Digital_Write(EPD_CS_PIN, 0);
	DEV_Digital_Write(EPD_PWR_PIN, 1);
	DEV_Digital_Write(EPD_RST_PIN, 1);
	return 0;
}

void DEV_Module_Exit(void)
{
	DEV_Digital_Write(EPD_DC_PIN, 0);
	DEV_Digital_Write(EPD_CS_PIN, 0);

	// close 5V
	DEV_Digital_Write(EPD_PWR_PIN, 0);
	DEV_Digital_Write(EPD_RST_PIN, 0);
}
