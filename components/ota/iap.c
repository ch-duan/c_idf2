/*
 * iap.c
 *
 *  Created on: Jul 28, 2020
 *      Author: ch
 */
#include "iap.h"

#include <stdlib.h>

#include "crc.h"
#include "stFlash.h"
#include "stdio.h"
#include "string.h"

iap_cb_t m_systemReset_cb = NULL;
iap_cb_t m_iwdg_cb = NULL;
iap_cb_t m_iap_rw_cb = NULL;
static const char *iap_vaersion_ = NULL;
#ifdef USE_LITTLEFS
#elif defined(USE_EXTERNAL_FLASH)
#include "external_flash.h"
#endif

uint8_t *updateData = NULL;

iap_replay_handler_t m_replay_halder;

update_pack_t update_pack = {
    .magic = IAP_MAGIC,
    .packageLen = 0,
    .storeLen = 0,
    .isCheck = 0,
    .md5 = {0},
    .app = 0,
    .updateState = IAP_NONE,
#if defined(USE_LITTLEFS)
    .fileName = {0},
#endif
    .appStartADDR = 0,
    .newAppStartADDR = 0,
    .updateFileType = 0,
};

IAP_Frame iap_Frame = {
    .header = {0x49, 0x41, 0x50},
      .protocol = {0},
      .len = {0xE, 0x0},
      .iap_Data = {.cmd = 0x00},
      .crc16 = {0}
};
uint8_t *iapFrame = (uint8_t *) &iap_Frame;

void iap_init(IAP_config_t *iap_config) {
  if (iap_config->version != NULL) {
    iap_vaersion_ = iap_config->version;
  }
  if (iap_config->replay_halder != NULL) {
    m_replay_halder = iap_config->replay_halder;
  }
  if (iap_config->systemReset_cb != NULL) {
    m_systemReset_cb = iap_config->systemReset_cb;
  }
  if (iap_config->iwdg_cb != NULL) {
    m_iwdg_cb = iap_config->iwdg_cb;
  }
  if (iap_config->temp_cache != NULL) {
    updateData = iap_config->temp_cache;
  }
  memset(&update_pack, 0, sizeof(update_pack_t));
  load_app_update_status(&update_pack);
}

int iap_write(uint32_t addr, uint8_t *in_data, uint32_t size) {
  return FLASH_If_Write(addr, addr + size, in_data, size);
}

int iap_read(uint32_t addr, uint32_t *out_data, uint32_t size) {
  for (uint32_t i = addr; i < addr + size; i++) {
    *(out_data + i) = *(__IO uint32_t *) i;
  }
  return 0;
}

static int64_t byteToInt(const uint8_t *b, int byteLen, bool isBigEndian) {
  if (b == NULL) {
    return 0;
  }
  int64_t result = 0;
  int i = 0;
  if (isBigEndian) {
    for (i = byteLen - 1; i >= 0; i--) {
      result += (int64_t) b[i] << (byteLen - 1 - i) * 8;
    }
  } else {
    for (i = 0; i < byteLen; i++) {
      result += (int64_t) b[i] << i * 8;
    }
  }
  return result;
}

static void intToByte(int64_t v, uint8_t *out, int byteLen, bool isBigEndian) {
  int i = 0;
  if (isBigEndian) {
    for (i = byteLen - 1; i >= 0; i--) {
      out[i] = (uint8_t) (v >> (byteLen - 1 - i) * 8);
    }
  } else {
    for (i = 0; i < byteLen; i++) {
      out[i] = (uint8_t) (v >> i * 8);
    }
  }
}

static int compareVersion(const char *v1, const char *v2) {
  const char *p_v1 = v1;
  const char *p_v2 = v2;

  while (*p_v1 && *p_v2) {
    char buf_v1[32] = {0};
    char buf_v2[32] = {0};

    char *i_v1 = strchr(p_v1, '.');
    char *i_v2 = strchr(p_v2, '.');

    if (!i_v1 || !i_v2)
      break;

    if (i_v1 != p_v1) {
      strncpy(buf_v1, p_v1, i_v1 - p_v1);
      p_v1 = i_v1;
    } else
      p_v1++;

    if (i_v2 != p_v2) {
      strncpy(buf_v2, p_v2, i_v2 - p_v2);
      p_v2 = i_v2;
    } else
      p_v2++;

    int order = atoi(buf_v1) - atoi(buf_v2);
    if (order != 0) {
      return order < 0 ? -1 : 1;
    }
  }

  double res = atof(p_v1) - atof(p_v2);
  if (res < 0)
    return -1;
  if (res > 0)
    return 1;
  return 0;
}

void System_SoftReset(void) {
  if (m_systemReset_cb != NULL) {
    m_systemReset_cb();
  }
  NVIC_SystemReset();
}

void updateCpy(unsigned char *dst, unsigned char *src, int startLen, int size) {
  int i;
  for (i = 0; i < size; i++) {
    dst[startLen + i] = src[i];
  }
}

void Vector_Init_Setting(uint32_t addr)  // The interrupt vector transfer
{
  SCB->VTOR = addr;
}

int write_app_file(void) {
  uint32_t statusWrite = 0;
  uint8_t retry = 0;
  update_pack.updateState = IAP_FLASH_WRITE_IN_PROGRESS;
  FLASH_If_Erase(update_pack.newAppStartADDR, update_pack.newAppStartADDR + update_pack.packageLen);
  uint32_t packet = 0, cur_len = 0, total_len = 0, packet_len = IAP_WRITE_MAX;
#ifdef USE_LITTLEFS
  if (read_file_size(UPDATE_FILE_NAME, &total_len) == -1) {
    return -1;
  }
#elif
  total_len = update_buffer.packageLen;
#endif
  packet = total_len % IAP_WRITE_MAX == 0 ? total_len / IAP_WRITE_MAX : total_len / IAP_WRITE_MAX + 1;
  if (total_len < IAP_WRITE_MAX) {
    packet_len = total_len;
  }
  uint32_t actual_size = 0;
  for (uint32_t i = 0; i < packet; i++) {
#ifdef USE_LITTLEFS
    if (read_file_with_addr(UPDATE_FILE_NAME, i * IAP_WRITE_MAX, buffer_read, packet_len, &actual_size) != 0) {
      return -1;
    }
#elif USE_EXTERNAL_FLASH
    external_flash_read(buffer, APP_STORE_ADDR + i * IAP_WRITE_MAX, packet_len);
#endif
    statusWrite = iap_write(update_pack.newAppStartADDR + i * IAP_WRITE_MAX, buffer_read, packet_len);
    if (HAL_OK != statusWrite) {
      retry++;
      uint32_t t_cur_len = cur_len + packet_len;
      printf("[write flash error] seq:%lu,total packet:%lu,need packet:%lu,total size:%lu,cur size:%lu,need size:%lu,start addr:%08X,end addr:%08X\r\n", i,
             packet, packet - i, total_len, t_cur_len, total_len - t_cur_len, update_pack.newAppStartADDR + i * IAP_WRITE_MAX,
             update_pack.newAppStartADDR + i * IAP_WRITE_MAX + packet_len);
      i--;
      if (retry > 3) {
        statusWrite = HAL_ERROR;
        printf("write flash check error. retry >3.\r\n");
        iap_Frame.iap_Data.cmd = IAP_CMD_WRITE_INTERNAL_FLASH_ERR;
        m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
        update_pack.app = 0;
        update_pack.updateState = IAP_STATUS_WRITE_INTERNAL_FLASH_ERR;
        save_app_update_status(&update_pack);
        return -1;
      }
      continue;
    } else {
      retry = 0;
    }
    cur_len += packet_len;
    packet_len = total_len - cur_len >= IAP_WRITE_MAX ? IAP_WRITE_MAX : total_len - cur_len;
    printf("[write flash success] seq:%lu,total packet:%lu,need packet:%lu,total size:%lu,cur size:%lu,need size:%lu,start addr:%08X,end addr:%08X\r\n", i,
           packet, packet - i, total_len, cur_len, total_len - cur_len, update_pack.newAppStartADDR + i * IAP_WRITE_MAX,
           update_pack.newAppStartADDR + i * IAP_WRITE_MAX + packet_len);
  }
  update_pack.updateState = IAP_NONE;
  return 0;
}

int save_app_update_status(update_pack_t *update_pack) {
#ifdef USE_LITTLEFS
  save_file(UPDATE_STATUS_FILE_NAME, update_pack, sizeof(update_pack_t));
#elif USE_EXTERNAL_FLASH
  save_app_update_status_external_flash(update_pack);
#elif USE_INTERNAL_FLASH
#elif
#endif
  return 0;
}

// 打印update_pack_t中的所有数据
void printf_iap_info(update_pack_t update_buffer) {
  printf("app packetlen:%lu\r\n", update_buffer.packageLen);
  printf("app storeLen:%lu\r\n", update_buffer.storeLen);
  printf("app %s\r\n", update_buffer.isCheck == 1 ? "need check md5" : "no need check md5");
  printf("app updateState:%lu\r\n", update_buffer.updateState);
  printf("app appStartADDR:0x%08x\r\n", update_buffer.appStartADDR);
  printf("app newStartADDR:0x%08x\r\n", update_buffer.newAppStartADDR);
  printf("app updateFileType:%lu\r\n", update_buffer.updateFileType);
  printf("app is %s app md5:", update_buffer.app == 1 ? "need update" : "no need update");
  printf("app md5:");
  for (int i = 0; i < 16; i++) {
    printf("%02X", update_buffer.md5[i]);
  }
  printf("\r\n");
}

void iapParser(uint8_t *iapPacket, uint32_t iapPacketLen) {
  if (iapPacketLen >= 14) {
    if (strncmp((char *) iapPacket, (char *) iapFrame, 3) == 0) {
      switch (iapPacket[11]) {
        case IAP_CMD_JMPBOOT:
          System_SoftReset();
          break;
        case IAP_CMD_REBOOT:
          System_SoftReset();
          break;
        case IAP_CMD_BOOTVERSION:
          break;
        case IAP_CMD_APPVERSION:
          break;
        case IAP_CMD_DATA_START: {
          update_pack.packageLen = 0;
          update_pack.storeLen = 0;
          update_pack.isCheck = iapPacket[12] == 1 ? 1 : 0;
          update_pack.newAppStartADDR = byteToInt(iapPacket + 13, 4, false);
          if (update_pack.isCheck == 1) {
            // 指令后面+16个字节md5
            memcpy(update_pack.md5, iapPacket + 17, 16);
          }

          char version[30] = {0};
          int version_len = 0;
          update_pack.cmpVersion = iapPacket[33];
          version_len = iapPacket[34];
          printf("version_len=%d\r\n", version_len);
          if (version_len > 30) {
            printf("version len is error. len is %d\r\n", version_len);
            iap_Frame.iap_Data.cmd = IAP_CMD_RECV_ERR;
            m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
            return;
          }
          memcpy(version, (char *) iapPacket + 35, version_len);
          if (update_pack.cmpVersion) {
            if (compareVersion(version, iap_vaersion_) != 1) {
              iap_Frame.iap_Data.cmd = IAP_CMD_VERSION_ERR;
              m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
              return;
            }
          }
          printf("recv app%d pack,isCheck=%d,md5=\r\n", update_pack.app, update_pack.isCheck);
          for (int i = 0; i < 16; i++) {
            printf("%02X", update_pack.md5[i]);
          }
          printf("\r\n\r\n");
          update_pack.updateFileType = IAP_FILE_APP;
#if defined(USE_LITTLEFS)
          if (iapPacketLen - 35 - version_len > 2) {
            if (iapPacket[35 + version_len] == 1) {
              update_pack.updateFileType = IAP_FILE_RESOUCE;
              int file_name_len = iapPacket[36 + version_len] + ((int) iapPacket[37 + version_len] << 8);
              if (file_name_len == 0) {
                update_pack.updateState = IAP_RECEIVE_ERROR;
                iap_Frame.iap_Data.cmd = IAP_CMD_RECV_ERR;
                m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
                return;
              } else {
                printf("file name:%.*s", file_name_len, iapPacket + 37);
                sniprintf(update_pack.fileName, 256, "%.*s", file_name_len, iapPacket + 38 + version_len);
              }
            }
          }
          delete_file(UPDATE_FILE_NAME);
#endif
          update_pack.updateState = IAP_RECEIVE_DATA;
          iap_Frame.iap_Data.cmd = IAP_CMD_RECV_DATA;
          m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
          break;
        }
        case IAP_CMD_DATA_SENDING: {
          uint16_t crc16 = 0;
          uint16_t crc16_src = ((uint16_t) iapPacket[iapPacketLen - 1] << 8) + iapPacket[iapPacketLen - 2];
          if (update_pack.updateState == IAP_RECEIVE_DATA) {
            crc16 = CRC16(iapPacket, iapPacketLen - 2);
            printf("debug crc16:%X,%X\r\n", crc16_src, crc16);
            if (crc16 == crc16_src) {
#if defined(USE_LITTLEFS)
              if (updateData == NULL) {
                iap_Frame.iap_Data.cmd = IAP_CMD_UPDATE_ERR;
                m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
                return;
              }
              save_app_temp(&update_pack, iapPacket + 12, iapPacketLen - 14);
#else
              save_app_file(&update_pack, iapPacket + 12, iapPacketLen - 14);
#endif
              update_pack.packageLen += iapPacketLen - 14;
              printf("debug packageLen = %lu\r\n", update_pack.packageLen);
              iap_Frame.iap_Data.cmd = IAP_CMD_DATA_NEXT;
            } else {
              printf("debug recv failure. Waiting for retry.\r\n");
              iap_Frame.iap_Data.cmd = IAP_CMD_DATA_RETRY;
            }
            printf("debug send IAP_CMD_DATA_NEXT\r\n");
            m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
          }
          break;
        }
        case IAP_CMD_DATA_OVER:
          if (update_pack.updateState == IAP_RECEIVE_DATA) {
            printf("update_pack.packageLen = %lu\r\n", update_pack.packageLen);
            update_pack.updateState = IAP_PREPARATION;
          } else if (update_pack.updateState == IAP_CMP_ERROR) {
            iap_Frame.iap_Data.cmd = IAP_CMD_UPDATE_ERR;
            m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
          }
          break;
        case IAP_CMD_TIME: {
          break;
        }
        default:
          break;
      }
      return;
    }
  } else {
    printf("[iap] error length\r\n");
  }
}

void check_update_bin(void) {
  uint32_t statusWrite = 0;
  int ret = 0;
  printf("save update bin.\r\n");
  ret = CheckoutAppUpdateFile(update_pack);
  if (ret != 0) {
    update_pack.updateState = IAP_CMP_ERROR;
    iap_Frame.iap_Data.cmd = IAP_CMD_MD5_ERR;
    m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
    printf("check md5 failure.%d\r\n", ret);
  } else {
    printf("check md5 success.%d\r\n", ret);
    update_pack.app = 1;
    save_app_update_status(&update_pack);
    update_pack.updateState = IAP_FLASH_WRITE_IN_PROGRESS;
    statusWrite = write_app_file();
    if (statusWrite == HAL_OK) {
      printf("write flash ok\r\n");
      update_pack.app = 0;
      update_pack.appStartADDR = update_pack.newAppStartADDR;
      save_app_update_status(&update_pack);
      update_pack.updateState = IAP_NONE;
      iap_Frame.iap_Data.cmd = IAP_CMD_UPDATE_FINISH;
      printf("debug send IAP_CMD_UPDATE_FINISH\r\n");
      m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
      // waiting for flash
#if defined(CMSIS_OS2_H_)
      osDelay(3000);
#elif defined(STM32_HAL_LEGACY)
      HAL_Delay(3000);
#elif defined(INC_FREERTOS_H)
      vTaskDelay(3000 / portTICK_PERIOD_MS);
#endif
      jump2APP(update_pack.appStartADDR);
    } else {
      printf("write flash failure.%lu\r\n", statusWrite);
    }
  }
}

void bootCheckUpdateApp(void) {
  load_app_update_status(&update_pack);
  if (update_pack.app == 1) {
    check_update_bin();
  }
}

void iapCheckUpdate(void) {
  int ret = 0;
  if (update_pack.updateState == IAP_PREPARATION) {
    ret = checkout_temp_file_md5(update_pack, updateData);
    if (ret != 0) {
      update_pack.updateState = IAP_CMP_ERROR;
      iap_Frame.iap_Data.cmd = IAP_CMD_UPDATE_ERR;
      m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
      printf("check md5 failure.%d\r\n", ret);
    } else {
      printf("check md5 success.%d start addr: 0x%x\r\n", ret, update_pack.newAppStartADDR);
      delete_file(UPDATE_FILE_NAME);
      if (save_file(UPDATE_FILE_NAME, updateData, update_pack.packageLen) == 0) {
        printf("save update bin.\r\n");
        update_pack.app = 1;
        update_pack.updateState = IAP_SAVE_OK;
        save_app_update_status(&update_pack);
        iap_Frame.iap_Data.cmd = IAP_CMD_JMPBOOT;
        printf("debug send IAP_CMD_JMPBOOT\r\n");
        m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
        load_app_update_status(&update_pack);
#if defined(CMSIS_OS2_H_)
        osDelay(1000);
#elif defined(STM32_HAL_LEGACY)
        HAL_Delay(1000);
#elif defined(INC_FREERTOS_H)
        vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif
        printf("jmp boot\r\n");
        System_SoftReset();
      } else {
        printf("save update bin error.\r\n");
        update_pack.updateState = IAP_CMP_ERROR;
        iap_Frame.iap_Data.cmd = IAP_CMD_UPDATE_ERR;
        m_replay_halder((uint8_t *) iapFrame, sizeof(iap_Frame));
      }
    }
  }
}

void boot_iapCheckUpdate(void) {
  if (update_pack.updateState == IAP_PREPARATION) {
    check_update_bin();
  }
}

void save_app_file(update_pack_t *update_buffer, const uint8_t *file_buffer, uint32_t file_len) {
  uint32_t packet = 0, cur_len = 0, total_len = 0, packet_len = IAP_WRITE_MAX;
  total_len = file_len;
  packet = total_len % IAP_WRITE_MAX == 0 ? total_len / IAP_WRITE_MAX : total_len / IAP_WRITE_MAX + 1;
  if (total_len < IAP_WRITE_MAX) {
    packet_len = total_len;
  }
  for (uint32_t i = 0; i < packet; i++) {
#if USE_LITTLEFS
    append_file(UPDATE_FILE_NAME, APP_STORE_ADDR + update_buffer->storeLen + i * IAP_WRITE_MAX, (void *) ((char *) file_buffer + i * IAP_WRITE_MAX),
                packet_len);
#elif USE_LITTLEFS == 0 && defined(USE_EXTERNAL_FLASH)
    external_flash_write((uint8_t *) file_buffer + i * IAP_WRITE_MAX, APP_STORE_ADDR + update_buffer->storeLen + i * IAP_WRITE_MAX, packet_len);
#endif
    cur_len += packet_len;
    update_buffer->storeLen += packet_len;
    packet_len = total_len - cur_len >= IAP_WRITE_MAX ? IAP_WRITE_MAX : total_len - cur_len;
    printf("[write app] update file seq:%lu,total packet:%lu,need packet:%lu,total size:%lu,cur size:%lu,need size:%lu,start addr:%08X,end addr:%08X", i,
           packet, packet - i, total_len, cur_len, total_len - cur_len, APP_STORE_ADDR + update_buffer->storeLen + i * IAP_WRITE_MAX,
           APP_STORE_ADDR + update_buffer->storeLen + i * IAP_WRITE_MAX + packet_len);
  }
}

void save_app_temp(update_pack_t *update_buffer, const uint8_t *file_buffer, uint32_t file_len) {
  if (updateData == NULL || file_buffer == NULL || file_len == 0) {
    return;
  }
  uint32_t packet = 0, cur_len = 0, total_len = 0, packet_len = IAP_WRITE_MAX;
  total_len = file_len;
  packet = total_len % IAP_WRITE_MAX == 0 ? total_len / IAP_WRITE_MAX : total_len / IAP_WRITE_MAX + 1;
  if (total_len < IAP_WRITE_MAX) {
    packet_len = total_len;
  }
  uint32_t storeLen = update_buffer->storeLen;
  for (uint32_t i = 0; i < packet; i++) {
    memcpy(updateData + update_buffer->storeLen + i * IAP_WRITE_MAX, (uint8_t *) file_buffer + i * IAP_WRITE_MAX, packet_len);
    cur_len += packet_len;
    update_buffer->storeLen += packet_len;
    packet_len = total_len - cur_len >= IAP_WRITE_MAX ? IAP_WRITE_MAX : total_len - cur_len;
    printf("[write temp] update file seq:%ld,total packet:%ld,need packet:%ld,total size:%ld,cur size:%ld,need size:%ld,start addr:%08X,end addr:%08X\r\n", i,
           packet, packet - i, total_len, cur_len, total_len - cur_len, update_buffer->storeLen + i * IAP_WRITE_MAX,
           update_buffer->storeLen + i * IAP_WRITE_MAX + packet_len);
  }
  uint32_t err_cnt = 0;
  bool has_err = false;
  for (uint32_t i = 0; i < packet; i++) {
    if (file_buffer[i] != updateData[storeLen + i]) {
      err_cnt++;
      has_err = true;
    }
  }
  if (has_err) {
    printf("write temp] write failure.%ld\r\n", err_cnt);
  }
}
