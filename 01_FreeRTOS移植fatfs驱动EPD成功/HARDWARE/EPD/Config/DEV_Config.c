/*****************************************************************************
* | File      	:   DEV_Config.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master
*                and enhance portability
*----------------
* |	This version:   V2.0
* | Date        :   2018-10-30
* | Info        :
# ******************************************************************************
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "DEV_Config.h"



void EPD_GPIO_Init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	//ʹ��GPIOCʱ��
	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//����
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//��ʼ��GPIO
//	GPIO_SetBits(GPIOC,GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;				//����ģʽ
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//����
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//��ʼ��GPIO
	
}
//extern SPI_HandleTypeDef hspi2;
u8 DEV_SPI_WriteByte(UBYTE value)
{

  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET){}//�ȴ���������  
	SPI_I2S_SendData(SPI2, value); //ͨ������SPIx����һ��byte  ����
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET){} //�ȴ�������һ��byte 
//	vTaskDelay(pdMS_TO_TICKS(1));
	return SPI_I2S_ReceiveData(SPI2);

}

void DEV_SPI_Write_nByte(UBYTE *value, UDOUBLE len)
{
    uint8_t data;
    for (; len > 0; len--)
    {
        data = *value;            // ��ȡ��ǰ�ֽ�
        DEV_SPI_WriteByte(data); // �����ֽ�
        value++;                  // ��ָ���ƶ�����һ���ֽ�
    }
}

void DEV_GPIO_Init()
{
//    HAL_SPI_MspDeInit(&hspi2);
    
    //HAL_SPI_DeInit(&hspi1); 
//    __HAL_RCC_SPI1_CLK_DISABLE();
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
}

void DEV_SPI_Init()
{
//    HAL_SPI_MspInit(&hspi2);
    //HAL_SPI_DeInit(&hspi1); 
//    __HAL_RCC_SPI1_CLK_DISABLE();
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
}


void DEV_SPI_SendData(UBYTE Reg)
{
	UBYTE i,j=Reg;

			F_EPD_MOSI_PIN = 1;
			F_EPD_SCLK_PIN = 1;
	DEV_Digital_Write(EPD_CS_PIN, 0);
	for(i = 0; i<8; i++)
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
	UBYTE i,j=0xff;
	
//	DEV_GPIO_Mode(EPD_MOSI_PIN, 0);
//    DEV_GPIO_Mode(EPD_SCLK_PIN, 1);
	F_EPD_MOSI_PIN = 0;
	F_EPD_SCLK_PIN = 1;
	DEV_Digital_Write(EPD_CS_PIN, 0);
	for(i = 0; i<8; i++)
	{
		DEV_Digital_Write(EPD_SCLK_PIN, 0);
		j = j << 1;
		if (DEV_Digital_Read(EPD_MOSI_PIN))
		{
            j = j | 0x01;
		}
		else
		{
            j= j & 0xfe;
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

    //close 5V
	DEV_Digital_Write(EPD_PWR_PIN, 0);
    DEV_Digital_Write(EPD_RST_PIN, 0);
}

