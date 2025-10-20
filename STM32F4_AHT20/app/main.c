#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "aht20.h"
#include "cpu_delay.h"
#include "usart.h"

int main(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
	
    usart_init();
		printf("uart initialized. \r\n");
	
    if (!aht20_init())
    {
        printf("[ERROR] aht20 init failed!!!\r\n");
				return 0;
    }
    
    uint32_t failcount = 0;
    float temperature, humidity;
    while (1)
    {
        if (failcount >= 10)
        {
            printf("[ERROR] aht20 failed %u times! program stop!!!\r\n", failcount);
            break;
        }
        if (!aht20_start_measurement())
        {
            printf("[ERROR] aht20 start measurement failed!!!\r\n");
            failcount++;
            continue;
        }
        if (!aht20_wait_for_measurement())
        {
            printf("[ERROR] aht20 measurement timeout!!!\r\n");
            failcount++;
            continue;
        }
        if (!aht20_read_measurement(&temperature, &humidity))
        {
            printf("[ERROR] aht20 read measurement failed!!!\r\n");
            failcount++;
            continue;
        }
        
        failcount = 0;
        printf("temperature: %.2f, humidity:%.2f\r\n", temperature, humidity);
        cpu_delay(1000 * 1000);
    }
    
    while (1)
    {
        printf("system failed, please check log history\r\n");
        cpu_delay(1000 * 1000);
    }
}

