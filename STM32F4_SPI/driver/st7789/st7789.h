#ifndef __ST7789_H__
#define	__ST7789_H__

#include <stdbool.h>
#include <stdint.h>

void st7789_init(void);
void st7789_fill_color(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void st7789_init_display (void);

#endif /* __ST7789_H__ */
