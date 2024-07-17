/*
 * iap.h
 *
 *  Created on: Jul 28, 2020
 *      Author: ch
 */

#ifndef INC_IAP_H_
#define INC_IAP_H_
#include <stdio.h>
#include <string.h>

#include "m_malloc.h"
#include "main.h"
#include "stFlash.h"
#include "stm32f4xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* if use fatfs */
// #define USE_LITTLEFS
// #define USE_LITTLEFS 0

/* user code form external flash */
// #define USE_EXTERNAL_FLASH

#define IAP_USE_RTOS

#define IAP_WRITE_MAX 4096

#if USE_LITTLEFS == 1
#include "vfs.h"
#endif

#ifdef USE_EXTERNAL_FLASH
#include "external_flash.h"
#endif

#ifdef IAP_USE_RTOS
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "portmacro.h"
#endif

/* user code form internal flash */
// #define USE_INTERNAL_FLASH

#define APP_START_ADDR        0x8020000
#define APP_STATUS_STORE_ADDR 0x0

#define APP_USER_NVS_ADDR     0x1000

#define APP_STORE_ADDR        0x100000

#define SRAM_8M               1

#define IAP_TEMP_CACHE_SIZE   1024 * 1024 * 1

#define STMF4

#define IAP_MAGIC 0x5A5F5B5E
typedef enum {
  IAP_NONE,
  IAP_PREPARATION,
  IAP_RECEIVE_DATA,
  IAP_RECEIVE_COMPLETE,
  IAP_RECEIVE_ERROR,
  IAP_SAVE_BIN,
  IAP_SAVE_OK,
  IAP_SAVE_ERROR,
  IAP_CMP_ERROR,
  IAP_SAVE_APP_STATUS,
  IAP_FLASH_ERASING,
  IAP_FLASH_ERASE_DONE,
  IAP_FLASH_WRITE_IN_PROGRESS,
  IAP_FLASH_WRITE_DONE,
  IAP_UPDATE_DONE,
  IAP_START_APP,
  IAP_REBOOT,
  IAP_STATUS_WRITE_INTERNAL_FLASH_ERR,
} IAP_UpdateState_t;

typedef enum {
  IAP_CMD_BOOTLOADER = 0x0,
  IAP_CMD_DATA_START,
  IAP_CMD_RECV_DATA,
  IAP_CMD_DATA_OVER,
  IAP_CMD_UPDATE_FINISH,
  IAP_CMD_UPDATE_ERR,
  IAP_CMD_RECV_ERR,
  IAP_CMD_VERSION_ERR,
  IAP_CMD_DATA_SENDING,
  IAP_CMD_DATA_NEXT,
  IAP_CMD_DATA_RETRY,
  IAP_CMD_TIME,
  IAP_CMD_WRITE_INTERNAL_FLASH_ERR,
  IAP_CMD_MD5_ERR,
  IAP_CMD_JMPAPP = 0x20,
  IAP_CMD_JMPBOOT,
  IAP_CMD_REBOOT,
  IAP_CMD_BOOTVERSION,
  IAP_CMD_APPVERSION,
  IPA_CMD_OTADATA = 0x50,
  IAP_INVALID_PROTO = 0xff
} IAP_CMD_T;

typedef enum {
  IAP_FILE_NONE,
  IAP_FILE_APP,
  IAP_FILE_RESOUCE,
} IAP_File_t;

#pragma pack(1)
typedef struct {
  uint32_t magic;
  uint32_t packageLen;
  uint32_t storeLen;
  uint8_t isCheck;
  uint8_t md5[16];
  uint8_t app; /* 1 need update */
  IAP_UpdateState_t updateState;
#if defined(USE_LITTLEFS)
  char fileName[256];
#endif
  uint32_t appStartADDR;
  uint32_t newAppStartADDR;
  IAP_File_t updateFileType;
  uint8_t cmpVersion;
} update_pack_t;
#pragma pack()

typedef struct {
  uint8_t app;
  uint8_t isCheck;
  uint8_t appStartADDR;
  uint8_t appEndADDR;
  uint8_t md5;
} IAP_Option;

typedef struct {
  uint8_t cmd;
} IAP_Data;

typedef struct {
  uint8_t header[3];
  uint8_t protocol[4];
  uint8_t len[2];
  uint8_t taskID[2];
  IAP_Data iap_Data;
  uint8_t crc16[2];
} IAP_Frame;

extern uint8_t *updateData;

extern update_pack_t update_pack;
extern IAP_Frame iap_Frame;
extern uint8_t *iapFrame;
extern uint8_t *iapFrame1;
extern uint8_t buffer_read[IAP_WRITE_MAX];

//((uint32_t)0x08000000)-((uint32_t)0x08010000)为bootLoader空间
#define APP1STARTADDR ((uint32_t) ADDR_FLASH_SECTOR_5)
#define APP1ENDADDR   ((uint32_t) ADDR_FLASH_SECTOR_9 - 1)
#define APP2STARTADDR ((uint32_t) ADDR_FLASH_SECTOR_9)
#define APP2ENDADDR   ((uint32_t) 0x080FFFFF)

typedef void (*iap_replay_handler_t)(uint8_t *buffer, uint32_t size); /**< function called when an event is posted to the queue */
typedef void (*iap_cb_t)(void);
typedef struct {
  const char *version;
  iap_replay_handler_t replay_halder;
  iap_cb_t systemReset_cb;
  iap_cb_t iwdg_cb;
  uint8_t *temp_cache;  // 下载固件的缓存
} IAP_config_t;

void iap_init(IAP_config_t *iap_config);

int iap_write(uint32_t addr, uint8_t *in_data, uint32_t size);
int iap_read(uint32_t addr, uint32_t *out_data, uint32_t size);
void updateCpy(unsigned char *dst, unsigned char *src, int startLen, int size);

void System_SoftReset(void);

// boot里调用
void bootCheckUpdateApp(void);
void check_update_bin(void);
void boot_iapCheckUpdate(void);
// app调用
void iapCheckUpdate(void);

#if SRAM_8M == 1
void save_app_temp(update_pack_t *update_buffer, const uint8_t *file_buffer, uint32_t file_len);
#else
#endif
void iapParser(uint8_t *iapPacket, uint32_t iapPacketLen);
// buffer文件的指针，buffer_len文件的大小，md5：原文件的md5
int CmpMd5(uint8_t *buffer, uint32_t buffer_len, uint8_t md5[16]);
int checkout_temp_file_md5(update_pack_t update_buffer, uint8_t *buffer);

int save_app_update_status(update_pack_t *update_buffer);
int write_app_file(void);
void printf_iap_info(update_pack_t update_buffer);
void save_app_file(update_pack_t *update_buffer, const uint8_t *file_buffer, uint32_t file_len);

#ifdef USE_LITTLEFS
#define UPDATE_FILE_NAME        "appFile.bin"
#define UPDATE_STATUS_FILE_NAME "status.txt"

int CheckoutAppUpdateFile(update_pack_t update_buffer);
int load_app_update_status(update_pack_t *update_buffer);
#elif defined(USE_EXTERNAL_FLASH)
void LoadAppUpdateFile(uint8_t *buffer, uint32_t *buffer_len);
int CheckoutAppUpdateFile(update_pack_t update_buffer);
void save_app_update_status_external_flash(update_pack_t *update_buffer);
int load_app_update_status(update_pack_t *update_buffer);

#elif  defined(USE_INTERNAL_FLASH)
#endif

void Vector_Init_Setting(uint32_t addr);

// 防止看门狗复位之类的

#ifdef __cplusplus
}
#endif

#endif /* INC_IAP_H_ */
