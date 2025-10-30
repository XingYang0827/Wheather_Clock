#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct 
{
    uint16_t height;
    uint16_t width;
    const uint8_t *model; 
} font_t;

extern const font_t font32;

#endif
