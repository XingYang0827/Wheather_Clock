#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdbool.h>
#include <stdint.h>

void eeprom_init(void);
bool eeprom_write(uint16_t address, uint8_t data[], uint32_t length);
bool eeprom_read(uint16_t address, uint8_t data[], uint32_t length);

#endif /* __BL24C512_H__ */
