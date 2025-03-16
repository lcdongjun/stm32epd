#include "sys.h"
#include "usart.h"	
#include "string.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOSʹ��
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif
 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	


uint8_t USART1_RX_Buff[USART1_RX_BUF_SIZE] = {0};  // DMA���ջ�����
uint8_t USART1_TX_Buff[USART1_TX_BUF_SIZE] = {0};  // DMA���ͻ�����
volatile uint16_t USART1_RX_Data_Size = 0;         // ���յ������ݳ���
volatile uint8_t USART1_RC_Flag = 0;               // ������ɱ�־
volatile uint8_t USART1_TC_Flag = 0;               // ������ɱ�־
volatile uint8_t data_ready_flag = 0;  // ��־λ��ָʾ�������

uint8_t RX_Buffer[2][USART1_RX_BUF_SIZE]; // ˫������
uint8_t TX_Buffer[USART1_RX_BUF_SIZE];    // DMA���ͻ�����
volatile uint8_t current_rx_buf = 0; // ��ǰ���ջ���������
volatile uint8_t processing_buf = 0; // ���ڴ���Ļ���������
volatile uint16_t received_len = 0;  // ÿ�ν��յ������ݳ���


//��ʼ��IO ����1 
//bound:������
void uart_init(u32 bound){
	
		memset(USART1_RX_Buff, 0, sizeof(USART1_RX_Buff));
		memset(TX_Buffer, 0, sizeof(TX_Buffer));
	
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    // 1. GPIO ��ʼ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 2. USART ��ʼ��
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_Init(USART1, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 3. DMA ��������
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_DeInit(DMA2_Stream5);
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART1_RX_Buff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = USART1_RX_BUF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  // ����ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_Init(DMA2_Stream5, &DMA_InitStructure);
    

		DMA_DeInit(DMA2_Stream7);
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)TX_Buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = 1;  // ��ʼΪ 1����̬����
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  // ����ģʽ
    DMA_Init(DMA2_Stream7, &DMA_InitStructure);
		
		// 4. NVIC����
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		// 5. ��ʼ��
		DMA_Cmd(DMA2_Stream5, ENABLE);
		USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
		USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		USART_Cmd(USART1, ENABLE);
		USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
		DMA_ITConfig(DMA2_Stream5, DMA_IT_TC | DMA_IT_HT, ENABLE);
		DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

		
#endif
	
}

void USART1_DMA_Send(uint8_t *data, uint32_t length) 
{
    if (length > USART1_TX_BUF_SIZE) return;  // ������ݳ���

    while (USART1_TC_Flag);  // �ȴ���һ�η������
    USART1_TC_Flag = 1;

    DMA_Cmd(DMA2_Stream7, DISABLE);
    DMA2_Stream7->M0AR = (uint32_t)data;
    DMA2_Stream7->NDTR = length;
    DMA_Cmd(DMA2_Stream7, ENABLE);
}

void USART1_DMA_Receive(uint8_t *data, uint32_t *length) 
{
    if (USART1_RC_Flag) {
        USART1_RC_Flag = 0;  // ������ձ�־
        memcpy(data, USART1_RX_Buff, USART1_RX_Data_Size);
        *length = USART1_RX_Data_Size;  // ���ؽ������ݳ���
        USART1_RX_Data_Size = 0;        // ������ȼ�¼
    }
}


void USART1_IRQHandler(void) 
{
   if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET) 
    {
        // ��������жϱ�־
        volatile uint32_t tmp;
        tmp = USART1->SR;
        tmp = USART1->DR;
        (void)tmp; // ��ֹ�������Ż�
        
        // ��ͣ��ǰ DMA ����
        DMA_Cmd(DMA2_Stream5, DISABLE);

        // ������յ������ݳ���
        received_len = USART1_RX_BUF_SIZE - DMA_GetCurrDataCounter(DMA2_Stream5);

        // ����Ƿ���ճ�����Ч
        if (received_len > 0 && received_len <= USART1_RX_BUF_SIZE) 
        {
            processing_buf = current_rx_buf;        // �л���������
            current_rx_buf = 1 - current_rx_buf;    // ׼����һ��������

            // �������� DMA ��������ַ
            DMA2_Stream5->M0AR = (uint32_t)RX_Buffer[current_rx_buf];
            DMA_SetCurrDataCounter(DMA2_Stream5, USART1_RX_BUF_SIZE);
            DMA_Cmd(DMA2_Stream5, ENABLE);         // �ָ� DMA ����
        } 
        else 
        {
            // ������������ճ��ȳ�����Χ������ DMA �����״̬
            received_len = 0;
            DMA2_Stream5->M0AR = (uint32_t)RX_Buffer[current_rx_buf];
            DMA_SetCurrDataCounter(DMA2_Stream5, USART1_RX_BUF_SIZE);
            DMA_Cmd(DMA2_Stream5, ENABLE);
        }
				
				data_ready_flag = 1;
    }
}

// DMA2 Stream5 �����жϷ�����
void DMA2_Stream5_IRQHandler(void)
{
//	printf("����DMA�����ж�\r\n");
    if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5) == SET) 
		{
//			printf("������������ж�\r\n");
        DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);
        // �봫����ɱ�־����ѡ��
    }
    if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_HTIF5) == SET) 
		{
        DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_HTIF5);
//			printf("�������ջ�����ǰ������ж�\r\n");
        // ���ڴ˴�����ջ�����ǰ�벿�ֵ�����
    }
}

// DMA2 Stream7 �����жϷ�����
void DMA2_Stream7_IRQHandler(void) 
{
//		printf("DMA �����ж�\r\n");
    if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) == SET) {
        USART1_TC_Flag = 0;  // ��־�������
//        printf("DMA ��������жϴ���\r\n");
        DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);  // ����жϱ�־
    }
}




 



