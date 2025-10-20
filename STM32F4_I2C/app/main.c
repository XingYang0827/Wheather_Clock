#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "eeprom.h"
#include "cpu_delay.h"
#include "usart.h"

#define EEPROM_SN_OFFSET	0
#define EEPROM_BOOT_COUNT	30

int main(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	usart_init();
	cpu_delay(5*1000);
	eeprom_init();
	cpu_delay(5*1000);
	
	const char default_sn[] = "random_1234567";
	char sn[14];
	if (eeprom_read(0, (uint8_t*) sn, 14)) {
		if (memcmp(default_sn, sn, 6) == 0) {
			printf("board sn: %s\r\n", sn);
		} else {
				if (!eeprom_write(EEPROM_SN_OFFSET, (uint8_t*) default_sn, sizeof(default_sn))) {
					printf("[ERROR] eeprom write default sn failed!!! \r\n");
				}
		}
	} else {
	
		printf("[ERROR] eeprom read sn failed!!! \r\n");
	}
	
}

int fputc(int ch, FILE* f) {
	USART_SendData(USART1, ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	return ch;
}