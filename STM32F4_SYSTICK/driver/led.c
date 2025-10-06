#include <stdbool.h>
#include "stm32f4xx.h"

// LED3: PD13
// LED4: PD12
// LED5: PD14

#define LED3_PORT   GPIOD
#define LED3_PIN    GPIO_Pin_13
#define LED4_PORT   GPIOD
#define LED4_PIN    GPIO_Pin_12
#define LED5_PORT   GPIOD
#define LED5_PIN    GPIO_Pin_14

void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
    GPIO_InitStructure.GPIO_Pin = LED3_PIN;
    GPIO_Init(LED3_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LED4_PIN;
    GPIO_Init(LED4_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LED5_PIN;
    GPIO_Init(LED5_PORT, &GPIO_InitStructure);
}

void led_set(uint8_t idx, bool onoff)
{
    switch (idx)
    {
        case 1:
            GPIO_WriteBit(LED3_PORT, LED3_PIN, onoff ? Bit_RESET : Bit_SET);
            break;
        case 2:
            GPIO_WriteBit(LED4_PORT, LED4_PIN, onoff ? Bit_RESET : Bit_SET);
            break;
        case 3:
            GPIO_WriteBit(LED5_PORT, LED5_PIN, onoff ? Bit_RESET : Bit_SET);
            break;
        default:
            break;
    }
}

void led_on(uint8_t idx)
{
    led_set(idx, false);
}

void led_off(uint8_t idx)
{
    led_set(idx, true);
}

void led_all_off(void)
{
    led_set(1, true);
    led_set(2, true);
    led_set(3, true);
}
