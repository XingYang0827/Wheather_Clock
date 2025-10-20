#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "usart.h"

static usart_received_func_t received_func;

void usart_init(void)
{
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	// gpio initialization
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
	
	// usart initialization
	USART_InitTypeDef USART_InitStruct;
	USART_StructInit(&USART_InitStruct);
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART1, &USART_InitStruct);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE); // enable usart1
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void usart_write(const char str[])
{
    int len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        USART_ClearFlag(USART1, USART_FLAG_TC);
        USART_SendData(USART1, str[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}

void usart_received_register(usart_received_func_t func)
{
    received_func = func;
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        if (received_func != NULL)
        {
            uint8_t data = USART_ReceiveData(USART1);
            received_func(data);
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

#include <stdio.h>

// declare FILE for Microlib 
struct __FILE { int handle; };
FILE __stdout;

// printf redirect to USART1
int fputc(int ch, FILE *f)
{
    (void)f;
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint16_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return ch;
}

// avoid semi-hosting
int _sys_exit(int x)      { (void)x; while (1); }
int _sys_open(const char *n, int m) { (void)n; (void)m; return 0; }
int _sys_close(int h)     { (void)h; return 0; }
int _sys_istty(int h)     { (void)h; return 1; }
int _sys_seek(int h,long p){ (void)h; (void)p; return 0; }
int _sys_write(int h,const unsigned char *b,unsigned l,int m)
{
    (void)h; (void)m;
    for (unsigned i=0;i<l;i++)
        fputc(b[i], &__stdout);
    return l;
}

