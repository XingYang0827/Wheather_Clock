#include <stdint.h>
#include <stdio.h>
#include "board.h"
#include "stm32f4xx.h"
#include "cpu_delay.h"
#include "usart.h"

static volatile uint32_t duty_cycle;
static volatile uint32_t frequency;

int main(void)
{
    board_lowlevel_init();
	
	// PD12 for TIM4_CH1
	// PD14 for TIM4_CH3
	// PD15 for TIM4_CH4

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    usart_init();
	
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_StructInit(&GPIO_InitStruct);
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
		GPIO_Init(GPIOD, &GPIO_InitStruct);
	
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;
    TIM_PWMIConfig(TIM4, &TIM_ICInitStructure);
    
    TIM_SelectInputTrigger(TIM4, TIM_TS_TI1FP1);
    
    TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM4,TIM_MasterSlaveMode_Enable);

    TIM_Cmd(TIM4, ENABLE);

    TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
		TIM_ITConfig(TIM4, TIM_IT_CC2, ENABLE);

    char str[100];
    while (1)
    {
        cpu_delay(100ul * 1000ul);
        snprintf(str, sizeof(str), "freq: %lu, duty: %lu\n", frequency, duty_cycle);
        usart_write(str);
    }
}

void TIM4_IRQHandler(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    uint32_t TIM4_apb1_clk_hz = RCC_Clocks.PCLK1_Frequency * 2ul;

    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);

    uint32_t ic1 = TIM_GetCapture1(TIM4) + 1;
    uint32_t ic2 = TIM_GetCapture2(TIM4) + 1;

    if (ic1 != 0)
    {
        duty_cycle = (ic2 * 100) / ic1;
        frequency = TIM4_apb1_clk_hz / ic1;
    }
    else
    {
        duty_cycle = 0;
        frequency = 0;
    }
}
