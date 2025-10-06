#include "stm32f4xx.h"
#include "key.h"
#include "key_desc.h"
#include "led.h"
#include "led_desc.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// LED type def
static struct led_desc _led4 = {GPIOD, GPIO_PIN_12, Bit_RESET, Bit_SET};
static struct led_desc _led3 = {GPIOD, GPIO_PIN_13, Bit_RESET, Bit_SET};
static struct led_desc _led5 = {GPIOD, GPIO_PIN_14, Bit_RESET, Bit_SET};
static struct led_desc _led6 = {GPIOD, GPIO_PIN_15, Bit_RESET, Bit_SET};

led_desc_t led4 = &_led4;
led_desc_t led3 = &_led3;
led_desc_t led5 = &_led5;
led_desc_t led6 = &_led6;

// key type def
// KEY1 (User Button): PA0
#define KEY_DEFINE(n, PORT, PIN, IRQn) \
static struct key_desc key##n = \
{ \
    .port = Port##PORT, \
    .pin = GPIO_Pin_##PIN,  \
    .exti_port_src = EXTI_PortSouceGPIO##Port,
    .exti_pin_src =  EXTI_PortSouce##PIN, \
    .exti_line = EXTI_Line##PIN, \
    .irqn = IRQn,  \
};
key_desc_t key##n = &_key_##n;

KEY_DEFINE(1, A, 0, EXTI0_IRQn)

void EXTI0_IRQHandler(void)
{
    if (key1->func != NULL) {
        key1->func();
    }
    EXTI_ClearITPendingBit(EXTI_Line0);
}
