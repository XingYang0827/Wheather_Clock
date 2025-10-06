#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"

int main(void) 
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	// LED  LED4: PD12 	LED3: PD13
	GPIO_InitTypeDef	GPIO_LED_InitStruct;
	GPIO_StructInit(&GPIO_LED_InitStruct);
	GPIO_LED_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_LED_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_LED_InitStruct.GPIO_Speed = GPIO_Medium_Speed;
	GPIO_LED_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_Init(GPIOD, &GPIO_LED_InitStruct);
	GPIO_WriteBit(GPIOD, GPIO_Pin_12 | GPIO_Pin_13, Bit_RESET); 
	GPIOD->ODR &= ~(GPIO_Pin_12 | GPIO_Pin_13);

	// btn not pressed: 0V, pressed: VDD
	GPIO_InitTypeDef	GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Medium_Speed;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0); // set btn0 (user btn) as interrupt mode
	EXTI_InitTypeDef	EXTI_InitStruct;
	EXTI_StructInit(&EXTI_InitStruct);
	EXTI_InitStruct.EXTI_Line = EXTI_Line0;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitTypeDef	NVIC_InitStruct;
	memset(&NVIC_InitStruct, 0, sizeof(NVIC_InitStruct));
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);	
	
	while(1);
}

void EXTI0_IRQHandler(void)
{
		GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
	//GPIO_WriteBit(GPIOD, GPIO_Pin_12 | GPIO_Pin_13, Bit_SET); // stm32f407: Bit_SET: light up
		for (volatile int i=0; i<1000000; i++);

    EXTI_ClearITPendingBit(EXTI_Line0);
}