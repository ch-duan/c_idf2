#ifndef H_C_IDF_EXTERNAL_FLASH_H
#define H_C_IDF_EXTERNAL_FLASH_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void external_flash_init(void);
int external_flash_read(uint8_t *data, uint32_t addr, uint32_t size);
int external_flash_write(uint8_t *data, uint32_t addr, uint32_t size);
int external_flash_erase(uint32_t addr, uint32_t size);
int external_flash_erase_block_64K(uint32_t block);
int external_flash_chip_erase(void);
#ifdef __cplusplus
}
#endif
#endif