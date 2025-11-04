
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "cpu_tick.h"
#include "usart.h"
#include "espat.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
	
#define ESP_AT_DEBUG 1

/* 
USART3:
    GPIO6 RX  -   PD8 TX
    GPIO7 TX  -   PD9 RX
*/


typedef enum
{
    AT_ACK_NONE,
    AT_ACK_OK,
    AT_ACK_ERROR,
    AT_ACK_BUSY,
    AT_ACK_READY,
} at_ack_t;

typedef struct
{
    at_ack_t ack;
    const char *string;
} at_ack_match_t;

static const at_ack_match_t at_ack_matches[] = 
{
    {AT_ACK_OK, "OK\r\n"},
    {AT_ACK_ERROR, "ERROR\r\n"},
    {AT_ACK_BUSY, "busy p...\r\n"},
    {AT_ACK_READY, "ready\r\n"},
};

static char rxbuf[1024];
static void esp_at_usart_write(const char* data);

static void esp_at_usart_init(void)
{
	// gpio initialization
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
	
	// usart initialization
	USART_InitTypeDef USART_InitStruct;
	USART_StructInit(&USART_InitStruct);
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART3, &USART_InitStruct);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART3, ENABLE); // enable USART3
	
}

bool esp_at_init(void)
{
    esp_at_usart_init();
    esp_at_write_command("AT", 100);
    if (!esp_at_write_command("AT", 100))
        return false;
    if (!esp_at_write_command("AT+RESTORE", 2000))
        return false;
    if (!esp_at_wait_ready(5000))
        return false;
    
    return true;
}

static void esp_at_usart_write(const char* data)
{
		while (data && *data)
    {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, *data++);
    }
		 while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
     USART_SendData(USART3, '\r');
		 while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
     USART_SendData(USART3, '\n');
}

static at_ack_t match_internal_ack(const char *str)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(at_ack_matches); i++)
    {
        if (strcmp(str, at_ack_matches[i].string) == 0)
            return at_ack_matches[i].ack;
    }
    
    return AT_ACK_NONE;
}

static at_ack_t esp_at_usart_wait_receive(uint64_t timeout)
{
    uint32_t rxlen = 0;
    const char *line = rxbuf;
    uint64_t start = cpu_get_ms();
    
    rxbuf[0] = '\0';
    while (rxlen < sizeof(rxbuf) - 1)
    {
        while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET)
        {
            if (cpu_get_ms() - start >= timeout)
                return AT_ACK_NONE;
        }
        rxbuf[rxlen++] = USART_ReceiveData(USART3);
        rxbuf[rxlen] = '\0';
        if (rxbuf[rxlen - 1] == '\n')
        {
            at_ack_t ack = match_internal_ack(line);
            if (ack != AT_ACK_NONE)
                return ack;
            line = rxbuf + rxlen;
        }
    }
    
    return AT_ACK_NONE;
}

bool esp_at_wait_ready(uint64_t timeout)
{
    return esp_at_usart_wait_receive(timeout) == AT_ACK_READY;
}


bool esp_at_write_command(const char* command, uint64_t timeout) // default: ms timeout
{	
		#if ESP_AT_DEBUG
    printf("[DEBUG] Command: %s\n", command);
		#endif

		esp_at_usart_write(command);
		at_ack_t ack = esp_at_usart_wait_receive(timeout);

		#if ESP_AT_DEBUG
				printf("[DEBUG] Response:\n%s\n", rxbuf);
		#endif
		return ack == AT_ACK_OK;
}

const char *esp_at_get_response(void)
{
    return rxbuf;
}
