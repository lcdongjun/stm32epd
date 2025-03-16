#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2011/6/14
//�汾��V1.4
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
			
#define USART1_RX_BUF_SIZE  1024  // ���ջ�������С
#define USART1_TX_BUF_SIZE  1024  // ���ͻ�������С
//#define BUFFER_SIZE 512  // ���δ��仺������С


extern uint8_t USART1_RX_Buff[USART1_RX_BUF_SIZE];
extern uint8_t USART1_TX_Buff[USART1_TX_BUF_SIZE];

volatile extern uint16_t USART1_RX_Data_Size;         // ���յ������ݳ���
volatile extern uint8_t USART1_RC_Flag;               // ������ɱ�־
volatile extern uint8_t USART1_TC_Flag;               // ������ɱ�־
volatile extern uint8_t data_ready_flag;

extern uint8_t RX_Buffer[2][USART1_RX_BUF_SIZE]; // ˫������
extern uint8_t TX_Buffer[USART1_RX_BUF_SIZE];    // DMA���ͻ�����
volatile extern uint8_t current_rx_buf; // ��ǰ���ջ���������
volatile extern uint8_t processing_buf; // ���ڴ���Ļ���������
volatile extern uint16_t received_len;  // ÿ�ν��յ������ݳ���


extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);
#endif


