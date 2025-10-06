#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "led.h"

int main(void) 
{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
		RCC_AHB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
		
		led_init();
		led_all_off();
	
		SysTick->LOAD = SystemCoreClock / 1000; // delay 1 ms
		SysTick->VAL = 0;
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk; // systick interrupt
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // enable starts the systick
		
		NVIC_SetPriority(SysTick_IRQn, 5);
//		NVIC_InitTypeDef	NVIC_InitStruct;
//		NVIC_InitStruct.NVIC_IRQChannel = SysTick_IRQn;
//		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
//		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
//		NVIC_Init(&NVIC_InitStruct);
	
		while(1);
			
}

void SysTick_Handler(void) {
		static bool led_state = false; // led off
		static int count = 0;
		if (++count >= 500) {
				count = 0;
				led_state = !led_state;
				led_set(1, led_state);
		}
}
