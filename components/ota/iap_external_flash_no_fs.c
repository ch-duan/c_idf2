#include <stdio.h>

#include "iap.h"

#if defined(USE_INTERNAL_FLASH)
int SaveAppPacket(uint8_t *buffer, uint32_t size, uint32_t startAddr, uint32_t endAddr) {
  HAL_StatusTypeDef status = iap_write(startAddr, buffer, size);
  if (status == HAL_OK) {
    printf("save app: size:%lu start:%08X,end:%08X success\r\n", size, startAddr, endAddr);
  } else {
    printf("save app: size:%lu start:%08X,end:%08X failure. Error is %d\r\n", size, startAddr, endAddr, status);
  }
  return status;
}

int EraseAppFlash(uint32_t appSize, uint32_t startAddr) {
  HAL_StatusTypeDef status = FLASH_If_Erase(startAddr, startAddr + appSize);
  if (status == HAL_OK) {
    printf("Erase: size:%lu start:%08X,end:%08X success\r\n", appSize, startAddr, startAddr + appSize);
  } else {
    printf("Erase: size:%lu start:%08X,end:%08X failure. Error is %d\r\n", appSize, startAddr, startAddr, status);
  }
  return status;
}
#elif USE_LITTLEFS == 0 && defined(USE_EXTERNAL_FLASH)
int load_app_update_status(update_pack_t *update_buffer) {
  external_flash_read((uint8_t *) update_buffer, APP_STATUS_STORE_ADDR, sizeof(update_pack_t));
  for (int i = 0; i < (int) sizeof(update_pack_t); i++) {
    printf("%02X ", ((uint8_t *) update_buffer)[i]);
  }
  printf("\r\n");
  printf("app is %sapp md5:", update_buffer->app == 1 ? "need update" : "no need update");
  for (int i = 0; i < 16; i++) {
    printf("%02X", update_buffer->md5[i]);
  }
  printf("\r\n");
  return 0;
}
void save_app_update_status_external_flash(update_pack_t *update_buffer) {
  external_flash_write((uint8_t *) update_buffer, APP_STATUS_STORE_ADDR, sizeof(update_pack_t));
}
#endif
