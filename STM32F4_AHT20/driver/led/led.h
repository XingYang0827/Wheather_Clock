// #ifndef __LED_H__
// #define __LED_H__

// #include <stdbool.h>
// #include <stdint.h>

// void led_init(void);
// void led_set(uint8_t idx, bool onoff);
// void led_on(uint8_t idx);
// void led_off(uint8_t idx);
// void led_all_off(void);

// #endif /* __LED_H__ */


#ifndef __LED_H__
#define __LED_H__

#include <stdbool.h>
#include <stdint.h>

struct led_desc;
typedef struct led_desc *led_desc_t;

void led_init(led_desc_t led);
void led_set(led_desc_t led, bool onoff);
void led_on(led_desc_t led);
void led_off(led_desc_t led);


#endif /* __LED_H__ */
