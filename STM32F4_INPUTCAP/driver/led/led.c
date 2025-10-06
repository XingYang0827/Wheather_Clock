// #include <stdbool.h>
// #include "stm32f4xx.h"

// // LED_Green: PD12
// // LED_Orange: PD13
// // LED_Red: PD14
// // LED_Blue: PD15

// #define LEDGREEN_PORT   GPIOD
// #define LEDGREEN_PIN    GPIO_Pin_12
// #define LEDOrange_PORT   GPIOD
// #define LEDOrange_PIN    GPIO_Pin_13
// #define LEDRed_PORT   GPIOD
// #define LEDRed_PIN    GPIO_Pin_14

// void led_init(void)
// {
//     GPIO_InitTypeDef GPIO_InitStructure;
//     GPIO_StructInit(&GPIO_InitStructure);
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//     GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//     GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
//     GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
//     GPIO_InitStructure.GPIO_Pin = LEDGREEN_PIN;
//     GPIO_Init(LEDGREEN_PORT, &GPIO_InitStructure);
    
//     GPIO_InitStructure.GPIO_Pin = LEDOrange_PIN;
//     GPIO_Init(LEDOrange_PORT, &GPIO_InitStructure);
    
//     GPIO_InitStructure.GPIO_Pin = LEDRed_PIN;
//     GPIO_Init(LEDRed_PORT, &GPIO_InitStructure);
// }

// void led_set(uint8_t idx, bool onoff)
// {
//     switch (idx)
//     {
//         case 1:
//             GPIO_WriteBit(LEDGREEN_PORT, LEDGREEN_PIN, onoff ? Bit_RESET : Bit_SET);
//             break;
//         case 2:
//             GPIO_WriteBit(LEDOrange_PORT, LEDOrange_PIN, onoff ? Bit_RESET : Bit_SET);
//             break;
//         case 3:
//             GPIO_WriteBit(LEDRed_PORT, LEDRed_PIN, onoff ? Bit_RESET : Bit_SET);
//             break;
//         default:
//             break;
//     }
// }

// void led_on(uint8_t idx)
// {
//     led_set(idx, true);
// }

// void led_off(uint8_t idx)
// {
//     led_set(idx, false);
// }

// void led_all_off(void)
// {
//     led_set(1, false);
//     led_set(2, false);
//     led_set(3, false);
// }


#include <stdbool.h>
#include <stdint.h>
#include "led_desc.h"
#include "led.h"

void led_init(led_desc_t led)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = led->Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(led->Port, &GPIO_InitStructure);
    
    GPIO_WriteBit(led->Port, led->Pin, led->OffBit);
}

void led_set(led_desc_t led, bool onoff)
{
    GPIO_WriteBit(led->Port, led->Pin, onoff ? led->OnBit : led->OffBit);
}

void led_on(led_desc_t led)
{
    GPIO_WriteBit(led->Port, led->Pin, led->OnBit);
}

void led_off(led_desc_t led)
{
    GPIO_WriteBit(led->Port, led->Pin, led->OffBit);
}
