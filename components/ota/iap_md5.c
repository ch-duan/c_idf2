#include <stdio.h>

#include "iap.h"
#include "md5.h"

uint8_t buffer_read[IAP_WRITE_MAX];
uint8_t decrypt[16] = {0};
int CmpMd5(uint8_t *buffer, uint32_t buffer_len, uint8_t md5[16]) {
  uint8_t decrypt[16] = {0};
  MD5_CTX md5_ctx;
  MD5Init(&md5_ctx);
  MD5Update(&md5_ctx, buffer, buffer_len);
  MD5Final(&md5_ctx, decrypt);
  printf("md5: ");
  for (int i = 0; i < 16; i++) {
    printf("%02X", decrypt[i]);
  }
  printf("\t");
  for (int i = 0; i < 16; i++) {
    printf("%02X", md5[i]);
  }
  printf("\r\n");
  if (strncasecmp((const char *) decrypt, (const char *) md5, 16) == 0) {
    return 0;
  }
  return 1;
}

int CheckoutAppUpdateFile(update_pack_t update_buffer) {
  uint8_t decrypt[16] = {0};
  MD5_CTX md5_ctx;
  MD5Init(&md5_ctx);
  uint32_t packet_MAX = 4096;
  uint32_t packet = 0, cur_len = 0, total_len = 0, packet_len = packet_MAX;
#if USE_LITTLEFS == 1
  if (read_file_size(UPDATE_FILE_NAME, &total_len) == -1) {
    return -1;
  }
#else
  total_len = update_buffer.packageLen;
#endif
  packet = total_len % packet_MAX == 0 ? total_len / packet_MAX : total_len / packet_MAX + 1;
  if (total_len < packet_MAX) {
    packet_len = total_len;
  }
  uint32_t actual_size = 0;
  for (uint32_t i = 0; i < packet; i++) {
#if USE_LITTLEFS == 1
    if (read_file_with_addr(UPDATE_FILE_NAME, i * packet_MAX, buffer_read, packet_len, &actual_size) != 0) {
      return -1;
    }
#elif USE_EXTERNAL_FLASH
    external_flash_read(buffer_read, APP_STORE_ADDR + i * packet_MAX, packet_len);
#endif
    MD5Update(&md5_ctx, buffer_read, packet_len);
    cur_len += packet_len;
    packet_len = total_len - cur_len >= packet_MAX ? packet_MAX : total_len - cur_len;
#if USE_LITTLEFS == 1
    printf("read app update file seq:%lu,total packet:%lu,need packet:%lu,total size:%lu,cur size:%lu,need size:%lu,start addr:%08X,end addr:%08X\r\n", i,
           packet, packet - i, total_len, cur_len, total_len - cur_len, i * packet_MAX, i * packet_MAX + packet_len);
#elif USE_EXTERNAL_FLASH
    printf("read app update file seq:%lu,total packet:%lu,need packet:%lu,total size:%lu,cur size:%lu,need size:%lu,start addr:%08X,end addr:%08X\r\n", i,
           packet, packet - i, total_len, cur_len, total_len - cur_len, APP_STORE_ADDR + i * packet_MAX, APP_STORE_ADDR + i * packet_MAX + packet_len);
#endif

#if defined(CMSIS_OS2_H_)
    osDelay(1);
#elif defined(INC_FREERTOS_H)
    vTaskDelay(1 / portTICK_PERIOD_MS);
#else
    HAL_Delay(1);
#endif
  }
  MD5Final(&md5_ctx, decrypt);
  printf("md5: ");
  for (int i = 0; i < 16; i++) {
    printf("%02X", decrypt[i]);
  }
  printf("\t");
  for (int i = 0; i < 16; i++) {
    printf("%02X", update_buffer.md5[i]);
  }
  printf("\r\n");
  if (strncasecmp((const char *) decrypt, (const char *) update_buffer.md5, 16) == 0) {
    return 0;
  } else {
    return 1;
  }
}

int checkout_temp_file_md5(update_pack_t update_buffer, uint8_t *buffer) {
  if (buffer == NULL) {
    return -1;
  }
  uint8_t decrypt[16] = {0};
  MD5_CTX md5_ctx;
  MD5Init(&md5_ctx);
  uint32_t packet_MAX = 4096;
  uint32_t packet = 0, cur_len = 0, total_len = 0, packet_len = packet_MAX;
  total_len = update_buffer.packageLen;
  packet = total_len % packet_MAX == 0 ? total_len / packet_MAX : total_len / packet_MAX + 1;
  if (total_len < packet_MAX) {
    packet_len = total_len;
  }
  for (uint32_t i = 0; i < packet; i++) {
    MD5Update(&md5_ctx, buffer + i * packet_MAX, packet_len);
    cur_len += packet_len;
    packet_len = total_len - cur_len >= packet_MAX ? packet_MAX : total_len - cur_len;
    printf("read app update file seq:%lu,total packet:%lu,need packet:%lu,total size:%lu,cur size:%lu,need size:%lu,start addr:%08X,end addr:%08X\r\n", i,
           packet, packet - i, total_len, cur_len, total_len - cur_len, i * packet_MAX, i * packet_MAX + packet_len);
#if defined(CMSIS_OS2_H_)
    osDelay(1);
#elif defined(INC_FREERTOS_H)
    vTaskDelay(1 / portTICK_PERIOD_MS);
#else
    HAL_Delay(1);
#endif
  }
  MD5Final(&md5_ctx, decrypt);
  printf("md5: ");
  for (int i = 0; i < 16; i++) {
    printf("%02X", decrypt[i]);
  }
  printf("\t");
  for (int i = 0; i < 16; i++) {
    printf("%02X", update_buffer.md5[i]);
  }
  printf("\r\n");
  if (strncasecmp((const char *) decrypt, (const char *) update_buffer.md5, 16) == 0) {
    return 0;
  } else {
    return 1;
  }
}
