#include "DEV_Config.h"

void EPD_GPIO_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // ʹ��GPIOCʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // ��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // ����
	GPIO_Init(GPIOC, &GPIO_InitStructure);			   // ��ʼ��GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	 // ����ģʽ
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ����
	GPIO_Init(GPIOC, &GPIO_InitStructure);			 // ��ʼ��GPIO
}
u8 DEV_SPI_WriteByte(UBYTE value)
{

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
	{
	}							   // �ȴ���������
	SPI_I2S_SendData(SPI2, value); // ͨ������SPIx����һ��byte  ����
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
	{
	} // �ȴ�������һ��byte
	return SPI_I2S_ReceiveData(SPI2);
}

void DEV_SPI_Write_nByte(UBYTE *value, UDOUBLE len)
{
	uint8_t data;
	for (; len > 0; len--)
	{
		data = *value;			 // ��ȡ��ǰ�ֽ�
		DEV_SPI_WriteByte(data); // �����ֽ�
		value++;				 // ��ָ���ƶ�����һ���ֽ�
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
