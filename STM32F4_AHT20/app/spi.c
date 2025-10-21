// #include <stdint.h>
// #include "stm32f4xx.h"

// void initialize() {
// 	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

// 	GPIO_InitTypeDef gpio;
//     GPIO_StructInit(&gpio);
// 	gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
// 	gpio.GPIO_Mode = GPIO_Mode_AF;
// 	gpio.GPIO_Speed = GPIO_Speed_50MHz;
// 	gpio.GPIO_OType = GPIO_OType_PP;
// 	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
// 	GPIO_Init(GPIOA, &gpio);

// 	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
// 	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
// 	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
// 	gpio.GPIO_Pin = GPIO_Pin_4;
// 	gpio.GPIO_Mode = GPIO_Mode_OUT;
// 	gpio.GPIO_OType = GPIO_OType_PP;
// 	gpio.GPIO_PuPd = GPIO_PuPd_UP;
// 	GPIO_Init(GPIOA, &gpio);
// 	GPIO_SetBits(GPIOA, GPIO_Pin_4); // default: pull up
	
// 	SPI_InitTypeDef spi;
//     SPI_StructInit(&spi);
// 	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
// 	spi.SPI_Mode = SPI_Mode_Master;
// 	spi.SPI_DataSize = SPI_DataSize_8b;
// 	spi.SPI_CPOL = SPI_CPOL_Low;
// 	spi.SPI_CPHA = SPI_CPHA_1Edge;
// 	spi.SPI_NSS = SPI_NSS_Soft;
// 	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // about 10MHz
// 	spi.SPI_FirstBit = SPI_FirstBit_MSB;
// 	spi.SPI_CRCPolynomial = 7;
// 	SPI_Init(SPI1, &spi);
// 	SPI_Cmd(SPI1, ENABLE);
// }

// uint8_t SPI1_Transmit(uint8_t data)
// {
//     while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//     SPI_I2S_SendData(SPI1, data);
//     while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
//     return SPI_I2S_ReceiveData(SPI1);
// }

// uint8_t W25Q_ReadID(void)
// {
//     uint8_t id[3];
//     GPIO_ResetBits(GPIOA, GPIO_Pin_4);   // CS low
//     SPI1_Transmit(0x9F);                 // Read JEDEC ID
//     id[0] = SPI1_Transmit(0xFF);
//     id[1] = SPI1_Transmit(0xFF);
//     id[2] = SPI1_Transmit(0xFF);
//     GPIO_SetBits(GPIOA, GPIO_Pin_4);     // CS high

//     printf("Manufacturer: 0x%02X, Type: 0x%02X, Capacity: 0x%02X\n", id[0], id[1], id[2]);
//     return id[0];
// }

// int fputc(int ch, FILE *f)
// {
//     while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//     USART_SendData(USART1, (uint16_t)ch);
//     return ch;
// }

// int main(void) 
// {
// 	initialize();
//     W25Q_ReadID(); 
// 	while(1){;}
// }


// USART: PB6 TX PB7 RX
// I2C2: PB10 SCK PB11 SDA
// SPI1: PA5 SPI1_SCK; PA6 SPI1_MISO; PA7 SPI1_MOSI； PA15 SPI1_NSS


#include <stdint.h>
#include "stm32f4xx.h"

void initialize(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	SPI_InitTypeDef SPI_InitStruct;
	SPI_StructInit(&SPI_InitStruct);
	SPI_InitStruct.SPI_Direction =  SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI1, &SPI_InitStruct);
	

}

int main(void) {
	while(1);
}