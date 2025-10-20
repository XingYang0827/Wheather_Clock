#include "stm32f4xx.h"

// LED1: PE9
// LED2: PB1
// LED3: PB0

static void cpu_delay(uint32_t us)
{
		SysTick->LOAD = SystemCoreClock / 1000 * us -1;
		SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
		while ((SysTick->CTRL &  SysTick_CTRL_COUNTFLAG_Msk) == 0);
		SysTick->CTRL = ~SysTick_CTRL_ENABLE_Msk;
}

static void board_lowlevel_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
}

static void board_led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void led_set(uint8_t led, uint8_t state)
{
    if (led == 1)
    {
        GPIO_WriteBit(GPIOE, GPIO_Pin_9, state ? Bit_RESET : Bit_SET);
    }
    else if (led == 2)
    {
        GPIO_WriteBit(GPIOB, GPIO_Pin_1, state ? Bit_RESET : Bit_SET);
    }
    else if (led == 3)
    {
        GPIO_WriteBit(GPIOB, GPIO_Pin_0, state ? Bit_RESET : Bit_SET);
    }
}

static void led_toggle(uint8_t led)
{
    if (led == 1)
    {
        GPIO_ToggleBits(GPIOE, GPIO_Pin_9);
    }
    else if (led == 2)
    {
        GPIO_ToggleBits(GPIOB, GPIO_Pin_1);
    }
    else if (led == 3)
    {
        GPIO_ToggleBits(GPIOB, GPIO_Pin_0);
    }
}

int main(void)
{
    board_lowlevel_init();
    board_led_init();

    led_set(1, 0);
    led_set(2, 0);
    led_set(3, 0);
    cpu_delay(50*1000);

    while (1)
    {
        led_toggle(1);
        led_toggle(2);
        led_toggle(3);
        cpu_delay(50*1000);
    }
}
