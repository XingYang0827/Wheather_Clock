
#include <stdint.h>
#include "stm32f4xx.h"
#include "cpu_tick.h"
#include <stdio.h>
#include "espat.h"
#include "usart.h"

int main(void) 
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	cpu_tick_init();
	usart_init();
	if (!esp_at_init()) goto ERROR;

  while (1)
	{
		if (!esp_at_write_command("AT", 1000)) 
		{
			printf("AT Failed\r\n");
			goto ERROR;
		}
		cpu_delay_ms(1000);
	}
	ERROR:
		while (1)
		{
			printf("AT Error");
			cpu_delay_ms(1000);
		}
}
