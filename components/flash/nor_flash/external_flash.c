#include "external_flash.h"

#include "driver_w25qxx.h"
#include "driver_w25qxx_advance.h"

void external_flash_init(void) {
  // w25qxx_qspi_read_dummy_t dummy = W25QXX_QSPI_READ_DUMMY_8_80MHZ;
  // w25qxx_qspi_read_wrap_length_t length = W25QXX_QSPI_READ_WRAP_LENGTH_8_BYTE;
  // w25qxx_security_register_t num = W25QXX_SECURITY_REGISTER_1;
  // w25qxx_burst_wrap_t wrap = W25QXX_BURST_WRAP_NONE;
  w25qxx_interface_t interface = W25QXX_INTERFACE_SPI;
  w25qxx_type_t chip_type = W25Q16;
  uint8_t manufacturer = 0;
  uint8_t device_id = 0;
  uint8_t res;
  res = w25qxx_advance_init(chip_type, interface, W25QXX_BOOL_FALSE);
  if (res != 0) {
    printf("w25qxx init failed\n");
    return;
  }
  res = w25qxx_advance_get_id((uint8_t *) &manufacturer, (uint8_t *) &device_id);
  if (res != 0) {
    printf("w25qxx get id failed\n");
  }
  printf("w25qxx: manufacturer is 0x%02X device id is 0x%02X.\n", manufacturer, device_id);
}

int external_flash_read(uint8_t *data, uint32_t addr, uint32_t size) {
  return w25qxx_advance_read(addr, data, size);
}
int external_flash_write(uint8_t *data, uint32_t addr, uint32_t size) {
  return w25qxx_advance_write(addr, data, size);
}
int external_flash_erase(uint32_t addr, uint32_t size) {
  return w25qxx_advance_sector_erase_4k(addr);
}

int external_flash_erase_block_64K(uint32_t block) {
  return w25qxx_advance_block_erase_64k(block);
}

int external_flash_chip_erase(void) {
  return w25qxx_advance_chip_erase();
}
