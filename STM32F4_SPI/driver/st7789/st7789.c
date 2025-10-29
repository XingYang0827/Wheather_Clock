
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stm32f4xx.h"
#include "cpu_delay.h"
#include "st7789.h"

/*
PA4  CS
PA5  SCK
PA6  MISO (optional)
PA7  MOSI
PB0  DC
PB1  RESET
PE7  BACKLIGHT
*/

#define CS_PORT				GPIOA
#define CS_PIN				GPIO_Pin_4
#define DC_PORT				GPIOB
#define DC_PIN				GPIO_Pin_0
#define RESET_PORT		GPIOB
#define RESET_PIN			GPIO_Pin_1
#define BL_PORT				GPIOE
#define BL_PIN				GPIO_Pin_7

#define ST7789_WIDTH	240
#define ST7789_HEIGHT	240

#define delay_us(x) 	cpu_delay(x)
#define delay_ms(x)		cpu_delay((x) * 1000)

void st7789_init(void) 
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);

    // === GPIOB: DC, RESET ===
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
		// === GPIOE: BACKLIGHT ===
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_SetBits(GPIOE, GPIO_Pin_7); // turn on backlight

    // === GPIOA: SPI1 + CS ===
    // PA4 CS
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_SetBits(GPIOA, GPIO_Pin_4); // CS default to be on

    // PA5/6/7 -> SPI1
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

    // === SPI1 initialization ===
    SPI_InitTypeDef SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);
    SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &SPI_InitStruct);
    SPI_Cmd(SPI1, ENABLE);
}


static void st7789_reset (void) 
{
		GPIO_ResetBits(RESET_PORT, RESET_PIN);
    delay_us(20); // at least 10 us
    GPIO_SetBits(RESET_PORT, RESET_PIN);
		delay_ms(120); // at least 120 ms: the first cmd after reset is 0x11
}


static void st7789_write_register(uint8_t reg, const uint8_t *data, uint16_t length)
{
    GPIO_ResetBits(CS_PORT, CS_PIN);  // CS pulled down, start transmitting
    GPIO_ResetBits(DC_PORT, DC_PIN); // Command mode

    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
    SPI_SendData(SPI1, reg);
    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_BSY) == SET);

		GPIO_SetBits(DC_PORT, DC_PIN); // Data mode
		for (uint16_t i = 0; i < length; i++) {
				while (SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
				SPI_SendData(SPI1, data[i]);
		}
		while (SPI_GetFlagStatus(SPI1, SPI_FLAG_BSY) == SET);
    GPIO_SetBits(CS_PORT, CS_PIN); // CS pulled up, stop transmitting
}


void st7789_set_range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    st7789_write_register(0x2A, (uint8_t[]){
        (x1 >> 8) & 0xFF, x1 & 0xFF,
        (x2 >> 8) & 0xFF, x2 & 0xFF
    }, 4);

    st7789_write_register(0x2B, (uint8_t[]){
        (y1 >> 8) & 0xFF, y1 & 0xFF,
        (y2 >> 8) & 0xFF, y2 & 0xFF
    }, 4);
}


static void st7789_set_gram_mode(void)
{
    st7789_write_register(0x2C, NULL, 0);
}


static bool in_screen_range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if (x1 >= ST7789_WIDTH || y1 >= ST7789_HEIGHT)
        return false;
    if (x2 >= ST7789_WIDTH || y2 >= ST7789_HEIGHT)
        return false;
    if (x1 > x2 || y1 > y2)
        return false;

    return true;
}


void st7789_fill_color(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{	
		if (!in_screen_range(x1, y1, x2, y2)) return;
	
		st7789_set_range(x1, y1, x2, y2);
		st7789_set_gram_mode();
	
    uint8_t color_data[2] = { color >> 8, color & 0xFF };
    uint32_t pixels = (x2 - x1 + 1) * (y2 - y1 + 1);

    GPIO_ResetBits(CS_PORT, CS_PIN);
    GPIO_SetBits(DC_PORT, DC_PIN); 	// data mode
    for (uint32_t i = 0; i < pixels; i++) {
        while (SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
        SPI_SendData(SPI1, color_data[0]);
        while (SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
        SPI_SendData(SPI1, color_data[1]);
    }
    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_BSY) == SET);
    GPIO_SetBits(CS_PORT, CS_PIN);
}


static void st7789_set_backlight (bool on) 
{
	GPIO_WriteBit(BL_PORT, BL_PIN, on ? Bit_SET : Bit_RESET);
}

void st7789_init_display (void) 
{
		st7789_reset(); // reset screen 
	
		st7789_write_register(0x11, NULL, 0);
    delay_ms(120);
    
    st7789_write_register(0x36, (uint8_t[]){0x70}, 1);
    st7789_write_register(0x3A, (uint8_t[]){0x55}, 1);
		st7789_write_register(0xB2, (uint8_t[]){0x0C, 0x0C, 0x00, 0x33, 0x33}, 5);
    st7789_write_register(0xB7, (uint8_t[]){0x46}, 1);
    st7789_write_register(0xBB, (uint8_t[]){0x1B}, 1);
    st7789_write_register(0xC0, (uint8_t[]){0x2C}, 1);
    st7789_write_register(0xC2, (uint8_t[]){0x01}, 1);
    st7789_write_register(0xC4, (uint8_t[]){0x20}, 1);
    st7789_write_register(0xC6, (uint8_t[]){0x0F}, 1);
    st7789_write_register(0xD0, (uint8_t[]){0xA4,0xA1}, 2);
    st7789_write_register(0xD6, (uint8_t[]){0xA1}, 1);
    st7789_write_register(0xE0, (uint8_t[]){0xF0,0x00,0x06,0x04,0x05,0x05,0x31,0x44,0x48,0x36,0x12,0x12,0x2B,0x34}, 14);
    st7789_write_register(0xE1, (uint8_t[]){0xF0,0x0B,0x0F,0x0F,0x0D,0x26,0x31,0x43,0x47,0x38,0x14,0x14,0x2C,0x32}, 14);
    st7789_write_register(0x21, NULL, 0);
    st7789_write_register(0x29, NULL, 0);
		
		st7789_set_backlight(true);
		st7789_fill_color(0,0,239, 239, 0xffff); // lights all on
}

