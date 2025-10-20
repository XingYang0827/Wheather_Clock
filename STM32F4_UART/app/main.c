
#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include <stdio.h>
#include <stm32f4xx_usart.h>

volatile uint32_t sysclk, hclk, pclk1, pclk2;

static void board_lowlevel_init(void)
{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}

static void usart_init(void)
{
		// PB6 + PB7
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7; // TX/RX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6,  GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

    USART_InitTypeDef USART_InitStructure;
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate   = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;
    USART_InitStructure.USART_Parity     = USART_Parity_No;
    USART_InitStructure.USART_Mode       = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}


static void usart_write(const char* str)
{
		while (*str)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}

static void led_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14; // LD4
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
}

 // clock check
void check_clock(void) {
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);

    sysclk = clocks.SYSCLK_Frequency;
    hclk   = clocks.HCLK_Frequency;
    pclk1  = clocks.PCLK1_Frequency;
    pclk2  = clocks.PCLK2_Frequency;
}


int main(void)
{
    SystemInit();  
		SystemCoreClockUpdate();
		//check_clock();
    board_lowlevel_init();
    usart_init();
//    led_init();
		usart_write("HELLO\r\n");

    while (1) {
			// wait for gpio get data
			while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
			
			// read data from gpio register
			uint8_t rdata = (uint8_t)USART_ReceiveData(USART1);
			
			// print
			printf("received: %c ascii: %d\r\n", rdata, rdata);
		}
}

int fputc(int c, FILE *stream)
{
		(void)stream; // state stream var is not used
	
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, (uint16_t)c);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	
		return c;
}
