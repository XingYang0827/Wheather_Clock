#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h> 
#include "stm32f4xx.h"
#include "stm32f4xx_i2c.h"
#include "cpu_delay.h"
#include "aht20.h"
#include "usart.h"

#define aht20_ADDR  0x38  
#define aht20_STM32_ADDR (aht20_ADDR<<1) 


#define I2C_CHECK_EVENT(EVENT, TIMEOUT) \
    do { \
        uint32_t timeout = TIMEOUT; \
        while (!I2C_CheckEvent(I2C2, EVENT) && timeout > 0) { \
            cpu_delay(10); \
            timeout -= 1; \
        } \
        if (timeout <= 0) \
            return false; \
    } while (0)

#define I2C_WAIT_FLAG(flag, TIMEOUT) \
    do { \
        uint32_t timeout = TIMEOUT; \
        while (!I2C_GetFlagStatus(I2C2, flag) && timeout > 0) { \
            cpu_delay(10); \
            timeout -= 1; \
        } \
				if (timeout <= 0) \
            return false; \
    } while (0)

#define I2C_WAIT_TXE(TIMEOUT) \
    do { \
        uint32_t timeout = TIMEOUT; \
        while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE) && timeout > 0) { \
            cpu_delay(10); \
            timeout--; \
        } \
        if (timeout == 0) return false; \
    } while(0)

#define I2C_WAIT_RXNE(TIMEOUT) \
    do { \
        uint32_t timeout = TIMEOUT; \
        while (!I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE) && timeout > 0) { \
            cpu_delay(10); \
            timeout--; \
        } \
        if (timeout == 0) return false; \
    } while(0)

void bus_recover (void) {
		// --- after you detect bus locked ---
		printf("bus locked, doing deep recovery\r\n");

		// 1. Disable and hard-reset I2C2
		I2C_Cmd(I2C2, DISABLE);
		I2C_SoftwareResetCmd(I2C2, ENABLE);
		cpu_delay(1000);
		I2C_SoftwareResetCmd(I2C2, DISABLE);
		// 6. Re-init the I2C peripheral
		I2C_Cmd(I2C2, ENABLE);
	
	(void)I2C2->SR1;
(void)I2C2->SR2;
I2C2->CR1 &= ~(I2C_CR1_START | I2C_CR1_STOP);
cpu_delay(100);
	
	printf("BUSY=%d MSL=%d SB=%d\r\n",
       (I2C2->SR2 >> 1) & 1,
       (I2C2->SR2 >> 0) & 1,
       (I2C2->SR1 >> 0) & 1);


}

static bool aht20_write(uint8_t data[], uint32_t length);
static bool aht20_read(uint8_t data[], uint32_t length);
static bool aht20_is_ready(void);


bool aht20_init(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);
	
    // PB10 = SCL, PB11 = SDA
		GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;  
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_I2C2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_I2C2);
		
		I2C_InitTypeDef  I2C_InitStruct;
    I2C_InitStruct.I2C_ClockSpeed = 100ul * 1000ul;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
		
		I2C_DeInit(I2C2);
		I2C_Init(I2C2, &I2C_InitStruct);
		I2C_Cmd(I2C2, ENABLE);
		cpu_delay(20 * 1000);
		
		if (aht20_is_ready()) return true;
    
    if (!aht20_write((uint8_t[]){0xBE, 0x08, 0x00}, 3)) return false;
    
    for (uint32_t t = 0; t < 100; t ++)
    {
        cpu_delay(1000);   // wait for >= 100 ms
        if (aht20_is_ready()) return true;
    }
    return false;
}

static bool aht20_write( uint8_t data[], uint32_t length)
{
    I2C_AcknowledgeConfig(I2C2, ENABLE);
    I2C_GenerateSTART(I2C2, ENABLE);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT, 10000);
	cpu_delay(100);
    I2C_Send7bitAddress(I2C2, aht20_STM32_ADDR, I2C_Direction_Transmitter);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 1000);
		printf("successfully send 7 bit address in write\r\n");
    for (uint32_t i = 0; i < length; i++)
    {
        I2C_SendData(I2C2, data[i]);
        I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTED, 1000);
    }
		printf("finish sending data \r\n");
    I2C_GenerateSTOP(I2C2, ENABLE);
		I2C_AcknowledgeConfig(I2C2, ENABLE); 
		cpu_delay(500); 
    return true;
}

static bool aht20_read(uint8_t data[], uint32_t length)
{
		I2C_AcknowledgeConfig(I2C2, ENABLE);
    I2C_GenerateSTART(I2C2, ENABLE);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT, 1000);

    I2C_Send7bitAddress(I2C2, aht20_STM32_ADDR, I2C_Direction_Receiver);
		cpu_delay(100);
    I2C_CHECK_EVENT(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, 1000);
		printf("successfully send 7 bit address in read\r\n");

    for (int i = 0; i < length; i++)
    {
			 if (i == length - 1)
            I2C_AcknowledgeConfig(I2C2, DISABLE);
        I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_RECEIVED, 1000);
       data[i] = I2C_ReceiveData(I2C2);
    }
		printf("finish recieving data \r\n");
    I2C_GenerateSTOP(I2C2, ENABLE);
		I2C_AcknowledgeConfig(I2C2, ENABLE); 
		
		if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) || !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)) {
			bus_recover();
			cpu_delay(20 * 1000);
		}
		return true;
}

static bool aht20_read_status (uint8_t *status) {
	uint8_t cmd = 0x71;
	if(!aht20_write(&cmd,1)) return false;
	if(!aht20_read(status,1)) return false;
	return true;
}

static bool aht20_is_busy(void) {
	uint8_t status;
	if (!aht20_read_status(&status)) return false;
	return (status & 0x80) != 0;
}

static bool aht20_is_ready(void) {
	uint8_t status;
	if (!aht20_read_status(&status)) return false;
	return (status & 0x18) == 0x18; 
}


bool aht20_start_measurement(void)
{
    return aht20_write((uint8_t[]){0xAC, 0x33, 0x00}, 3);
}

bool aht20_wait_for_measurement(void)
{
    for (uint32_t t = 0; t < 200; t++)
    {
        cpu_delay(1000); // wait for 80+ms for finishing measurement
        if (!aht20_is_busy())
        {
            return true;
        }
    }
    return false;
}

bool aht20_read_measurement(float *temperature, float *humidity)
{
    uint8_t data[6];
    if (!aht20_read(data, 6))
        return false;
    
    uint32_t raw_humidity = ((uint32_t)data[1] << 12) | 
                            ((uint32_t)data[2] << 4) | 
                            ((uint32_t)(data[3] &0xF0) >> 4);
    uint32_t raw_temperature = ((uint32_t)(data[3] & 0x0F) << 16) | 
                               ((uint32_t)data[4] << 8) | 
                               ((uint32_t)data[5]);
    
    *humidity = (float)raw_humidity * 100.0f / (float)0x100000;
    *temperature = (float)raw_temperature * 200.0f / (float)0x100000 - 50.0f;
    
    return true;
}

