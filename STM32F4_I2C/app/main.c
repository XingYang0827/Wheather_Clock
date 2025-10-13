#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f4xx.h"
#include "stm32f4xx_i2c.h"
#include "cpu_delay.h"
#include "usart.h"


#define EEPROM_ADDR  0x50  // A0=A1=A2=0 0b1010000 (7 bit)
#define EEPROM_STM32_ADDR 0xA0 // 0b10100000 (8 bit)

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
		cpu_delay(100);
}

#define I2C_CHECK_EVENT(EVENT, TIMEOUT) \
    do { \
        uint32_t timeout = TIMEOUT; \
        while (!I2C_CheckEvent(I2C1, EVENT) && timeout > 0) { \
            cpu_delay(10); \
            timeout -= 1; \
        } \
        if (timeout <= 0) \
            return false; \
    } while (0)


static bool eeprom_page_write(uint16_t address, uint8_t data[], uint32_t length)
{
		while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_GenerateSTART(I2C1, ENABLE);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT, 10000);
    I2C_Send7bitAddress(I2C1, EEPROM_STM32_ADDR, I2C_Direction_Transmitter);
		//cpu_delay(100);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 1000);
    I2C_SendData(I2C1, (address >> 8) & 0xff);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED, 1000);
    I2C_SendData(I2C1, address & 0xff);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED, 1000);
    for (uint32_t i = 0; i < length; i++)
    {
        I2C_SendData(I2C1, data[i]);
        I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED, 1000);
    }
//		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
//		(void)I2C1->SR1;     // ? SR1 ???
//		(void)I2C1->SR2;     // ? SR2 ???
    I2C_GenerateSTOP(I2C1, ENABLE);
//		while (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);
//		I2C_ClearFlag(I2C1, I2C_FLAG_STOPF);
    // cpu_delay(10*1000);
    return true;
}
static bool eeprom_page_read(uint16_t address, uint8_t data[], uint32_t length)
{
		// while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_GenerateSTART(I2C1, ENABLE);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT, 1000);

    I2C_Send7bitAddress(I2C1, EEPROM_STM32_ADDR, I2C_Direction_Transmitter);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 1000);

    I2C_SendData(I2C1, (address >> 8) & 0xff);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED, 1000);
    I2C_SendData(I2C1, address & 0xff);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED, 1000);

    cpu_delay(10000); // give eeprom time to prep for repeated start
		I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_GenerateSTART(I2C1, ENABLE);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT, 1000);

    I2C_Send7bitAddress(I2C1, EEPROM_STM32_ADDR, I2C_Direction_Receiver);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, 1000);

    for (int i = 0; i < length; i++)
    {
        if (i == length - 1)
            I2C_AcknowledgeConfig(I2C1, DISABLE);
        I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_RECEIVED, 1000);
        data[i] = I2C_ReceiveData(I2C1);
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE); // re-enable for next op
    return true;
}


static uint8_t testnums[16];

int main (void) {
	cpu_delay(10000); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	I2C1_Init();
	cpu_delay(10000);  
	
	for (uint32_t i= 0; i < 16; i++) {
		testnums[i] = i+1;
	}
	
	bool b1 = eeprom_page_write(0x0000, testnums, 16);
	memset(testnums, 0, 16);

	/* reset + reinitialization */
	I2C_Cmd(I2C1, DISABLE);
	I2C_DeInit(I2C1);
	I2C1_Init();
	
	bool b2 = eeprom_page_read(0x0000, testnums, 16);
	while(1);
}
