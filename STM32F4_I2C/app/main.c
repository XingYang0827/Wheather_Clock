#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f4xx.h"
#include "stm32f4xx_i2c.h"
#include "cpu_delay.h"
#include "usart.h"


#define EEPROM_ADDR  0x50  // A0=A1=A2=0
#define EEPROM_STM32_ADDR 0xA0

void I2C1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef  I2C_InitStruct;

    // PB6 = SCL, PB9 = SDA
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;  
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    I2C_DeInit(I2C1);
    I2C_InitStruct.I2C_ClockSpeed = 100ul * 1000ul;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStruct);

    I2C_Cmd(I2C1, ENABLE);
}

static bool EEPROM_Write(uint16_t MemAddress, uint8_t data[], uint32_t length) {
    // wait until line is empty
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
    I2C_Send7bitAddress(I2C1, EEPROM_ADDR << 1, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    // send high address bytes
    I2C_SendData(I2C1, (uint8_t)(MemAddress >> 8));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    // send low address bytes
    I2C_SendData(I2C1, (uint8_t)(MemAddress & 0xFF));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    // send data
		for (uint32_t i = 0; i < length; i++) {
			I2C_SendData(I2C1, data[i]);
			while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
		}

    I2C_GenerateSTOP(I2C1, ENABLE);
		
		return true;
}

static bool EEPROM_Read(uint16_t MemAddress, uint8_t data[], uint32_t length) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C1, ENABLE);
		I2C_AcknowledgeConfig(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, EEPROM_ADDR << 1, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    // send register address
    I2C_SendData(I2C1, (uint8_t)(MemAddress >> 8));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C1, (uint8_t)(MemAddress & 0xFF));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_AcknowledgeConfig(I2C1, ENABLE); // TODO: try flip
    // restart
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, EEPROM_ADDR << 1, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

		// send data
		for (uint32_t i = 0; i < length; i++) {
			if (i == length - 1) {
					I2C_AcknowledgeConfig(I2C1, DISABLE);
					I2C_GenerateSTOP(I2C1, ENABLE);
			}
			while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
			data[i] = I2C_ReceiveData(I2C1);
		}
		I2C_AcknowledgeConfig(I2C1, ENABLE);

    // stop response
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    I2C_GenerateSTOP(I2C1, ENABLE);
		
		return true;
}

static uint8_t testnums[200];

int main (void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	I2C1_Init();
	
	for (uint32_t i= 0; i < 200; i++) {
		testnums[i] = i+1;
	}
	
	EEPROM_Write(0x0000, testnums, 200);
	memset(testnums, 0, 200);
	EEPROM_Read(0x0000, testnums, 200);
	while(1);
}
