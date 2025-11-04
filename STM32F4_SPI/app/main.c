
#include <stdint.h>
#include "stm32f4xx.h"
#include "cpu_delay.h"
#include "st7789.h"
#include "font.h"
#include "image.h"

int main(void) {
	
    st7789_init();        
    st7789_init_display();  
		//st7789_draw_image(0, 0, &image_tv);
   while (1) 
		{
    st7789_fill_color(0, 0, 239, 239, 0xF800); // red
		st7789_write_ascii(0, 0, "Hello World", 0xFFFF, 0xF800, &font32);
    cpu_delay(1000 * 1000);

    st7789_fill_color(0, 0, 239, 239, 0x07E0); // green
		st7789_write_ascii(0, 0, "Hello World", 0x0000, 0x07E0, &font32);
    cpu_delay(1000 * 1000);

    st7789_fill_color(0, 0, 239, 239, 0x001F); // blue
		st7789_write_ascii(0, 0, "Hello World", 0xFFFF, 0x001F, &font32);
    cpu_delay(1000 * 1000);

		}

}
