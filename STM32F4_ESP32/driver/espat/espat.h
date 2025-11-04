#ifndef __ESPAT_H__
#define __ESPAT_H__

// Note: esp_at printf redirection (fputc/_sys_write) implemented in esp_at.c

#include <stdint.h>
#include <stdbool.h>

bool esp_at_init(void);
bool esp_at_write_command(const char* command, uint64_t timeout);
bool esp_at_wait_ready(uint64_t timeout);
const char *esp_at_get_response(void);

#endif /* __ESPAT_H__ */
