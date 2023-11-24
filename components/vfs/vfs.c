#include "vfs.h"

#ifdef USE_VFS
#include <stdint.h>
#include <stdio.h>

#include "external_flash.h"

static vfs_config_t vfs_ = {0};

uint8_t read_cache[VFS_CACHE_SIZE];
uint8_t prog_cache[VFS_CACHE_SIZE];
void vfs_call_iwdg(void) {
  if (vfs_.vfs_iwdg) {
    vfs_.vfs_iwdg();
  }
}
#if VFS_USE_OS == 1
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "portmacro.h"
#include "task.h"
osSemaphoreId_t mutex = NULL;
#else
#include "cmsis_gcc.h"
#include "main.h"
#endif
#if defined(USE_LFS)
#include "lfs.h"
int read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size);

int prog(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size);

int erase(const struct lfs_config* c, lfs_block_t block);

int sync(const struct lfs_config* c);
int lock(const struct lfs_config* c);
int unlock(const struct lfs_config* c);
// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read = read,
    .prog = prog,
    .erase = erase,
    .sync = sync,
#ifdef LFS_THREADSAFE
    .lock = lock,
    .unlock = unlock,
#endif

    // block device configuration
    .read_size = 16,
    .prog_size = 16,
    .read_buffer = read_cache,
    .prog_buffer = prog_cache,
    .block_size = VFS_CACHE_SIZE,
    .block_count = 512,
    .cache_size = VFS_CACHE_SIZE,
    .lookahead_size = 16,
    .lookahead_buffer = NULL,
    .block_cycles = 500,
};

int lock(const struct lfs_config* c) {
#if VFS_USE_OS == 1
  osSemaphoreAcquire(mutex, portMAX_DELAY);
#else
  __disable_irq();
#endif
  return LFS_ERR_OK;
}

// Unlock the underlying block device. Negative error codes
// are propagated to the user.
int unlock(const struct lfs_config* c) {
#if VFS_USE_OS == 1
  osSemaphoreRelease(mutex);
#else
  __enable_irq();
#endif
  return LFS_ERR_OK;
}

// Read a region in a block. Negative error codes are propagated
// to the user.
int read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) {
  return external_flash_read(buffer, c->block_size * block + off, size) == 0 ? LFS_ERR_OK : LFS_ERR_CORRUPT;
}

// Program a region in a block. The block must have previously
// been erased. Negative error codes are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int prog(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) {
  return external_flash_write((uint8_t*) buffer, c->block_size * block + off, size) == 0 ? LFS_ERR_OK : LFS_ERR_CORRUPT;
}

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int erase(const struct lfs_config* c, lfs_block_t block) {
  uint32_t addr = c->block_size * block;
  external_flash_erase(addr, c->block_size);
  return LFS_ERR_OK;
}

// Sync the state of the underlying block device. Negative error codes
// are propagated to the user.
int sync(const struct lfs_config* c) {
  return LFS_ERR_OK;
}
#else
#endif

int vfs_init(vfs_config_t vfs_config) {
  if (vfs_config.vfs_iwdg != NULL) {
    vfs_.vfs_iwdg = vfs_config.vfs_iwdg;
  }
#if defined(USE_LFS)
#if VFS_USE_OS == 1
  mutex = osSemaphoreNew(1, 1, NULL);
  if (mutex == NULL) {
    printf("vfs mutex init fail");
  }
#endif
  int err = lfs_mount(&lfs, &cfg);
  if (err == 0) {
    printf("vfs mount success\r\n");
    return 0;
  }
  // reformat if we can't mount the filesystem
  // this should only happen on the first boot
  external_flash_chip_erase();
  err = lfs_format(&lfs, &cfg);
  if (err != 0) {
    printf("vfs lfs format success\r\n");
  }
  err = lfs_mount(&lfs, &cfg);
  if (err == 0) {
    printf("vfs mount success\r\n");
  }
  return err;
#endif
  return 0;
}

#if defined(USE_LFS)

/* Open or create a file */
int f_open(lfs_file_t* fp, const char* path, int mode) {
  return lfs_file_open(&lfs, fp, path, mode);
}

/* Close an open file object */
int f_close(lfs_file_t* fp) {
  return lfs_file_close(&lfs, fp);
}
/* Read data from the file */
int f_read(lfs_file_t* fp, void* buff, uint16_t btr, uint16_t* br) {
  return lfs_file_read(&lfs, fp, buff, btr);
}
/* Write data to the file */
int f_write(lfs_file_t* fp, const void* buff, uint16_t btw, uint16_t* bw) {
  return lfs_file_write(&lfs, fp, buff, btw);
}
/* Move file pointer of the file object */
int f_lseek(lfs_file_t* fp, size_t ofs) {
  return lfs_file_seek(&lfs, fp, ofs, LFS_SEEK_CUR);
}

/* Move file pointer of the file object */
int f_seek_start(lfs_file_t* fp) {
  return lfs_file_seek(&lfs, fp, 0, LFS_SEEK_SET);
}

/* Move file pointer to tail of the file object */
int f_seek_end(lfs_file_t* fp) {
  return lfs_file_seek(&lfs, fp, 0, LFS_SEEK_END);
}

/* Truncate the file */
int f_truncate(lfs_file_t* fp, size_t ofs) {
  return lfs_file_truncate(&lfs, fp, ofs);
}
/* Flush cached data of the writing file */
int f_sync(lfs_file_t* fp) {
  return lfs_file_sync(&lfs, fp);
}
/* Open a directory */
int f_opendir(lfs_dir_t* dp, const char* path) {
  return lfs_dir_open(&lfs, dp, path);
}
/* Close an open directory */
int f_closedir(lfs_dir_t* dp) {
  return lfs_dir_close(&lfs, dp);
}
/* Read a directory item */
int f_readdir(lfs_dir_t* dp, struct lfs_info* fno) {
  return lfs_dir_read(&lfs, dp, fno);
}
/* Create a sub directory */
int f_mkdir(const char* path) {
  return lfs_mkdir(&lfs, path);
}
/* Delete an existing file or directory */
int f_unlink(const char* path) {
  return lfs_remove(&lfs, path);
}
/* Rename/Move a file or directory */
int f_rename(const char* path_old, const char* path_new) {
  return lfs_rename(&lfs, path_old, path_new);
}
/* Get file status */
int f_stat(const char* path, struct lfs_info* fno) {
  return lfs_stat(&lfs, path, fno);
}
int save_file(const char* filename, void* buffer, uint32_t size) {
#if VFS_USE_OS == 1
  uint32_t last_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
#endif
  lfs_file_t file;
  int ret = f_open(&file, filename, LFS_O_CREAT | LFS_O_RDWR);
  if (ret < 0) {
    printf("fopen failure:%d\r\n", ret);
    return -1;
  }

  uint32_t packet = 0, cur_len = 0, total_len = 0, packet_len = VFS_WRITE_MAX_SIZE;
  total_len = size;
  packet = total_len % VFS_WRITE_MAX_SIZE == 0 ? total_len / VFS_WRITE_MAX_SIZE : total_len / VFS_WRITE_MAX_SIZE + 1;
  if (total_len < VFS_WRITE_MAX_SIZE) {
    packet_len = total_len;
  }
  // ret = lfs_file_write(&lfs, &file, buffer, size);
  // if (ret < 0) {
  // }
  // printf("[vfs] save file %s. ret is %d\r\n", ret < 0 ? "failure" : "success", ret);
  uint8_t retry = 0;
  for (uint32_t i = 0; i < packet; i++) {
    vfs_call_iwdg();
    ret = lfs_file_write(&lfs, &file, (uint8_t*) buffer + i * VFS_WRITE_MAX_SIZE, packet_len);
    if (ret < 0) {
      i--;
      printf("[vfs] write file failure. %d\r\n", ret);
      if (++retry > 3) {
        printf("[vfs] write file failure. retry >3 %d\r\n", ret);
        f_close(&file);
        return -1;
      }
      continue;
    }
    f_sync(&file);
    // ret = f_lseek(&file, packet_len);
    // if (ret < 0) {
    //   printf("fseek failure. %d\r\n", ret);
    //   f_close(&file);
    //   return -1;
    // }
    retry = 0;
    cur_len += packet_len;
    packet_len = total_len - cur_len >= VFS_WRITE_MAX_SIZE ? VFS_WRITE_MAX_SIZE : total_len - cur_len;
    printf("[vfs] save file seq:%lu,total packet:%lu,need packet:%lu,total size:%lu,cur size:%lu,need size:%lu,start addr:%08X,end addr:%08X\r\n", i, packet,
           packet - i, total_len, cur_len, total_len - cur_len, i * VFS_WRITE_MAX_SIZE, i * VFS_WRITE_MAX_SIZE + packet_len);
#if VFS_USE_OS == 1
    osDelay(1);
#else
    HAL_Delay(1);
#endif
  }
  f_close(&file);
#if VFS_USE_OS == 1
  uint32_t time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  printf("[vfs] save %s %s cost %lu ms\r\n", filename, ret < 0 ? "failure" : "success", time - last_time);
#endif
  return 0;
}

int read_file(const char* filename, void* buffer, uint32_t size, uint32_t* actual_size) {
  lfs_file_t file;
  int ret = f_open(&file, (char*) filename, LFS_O_RDONLY);
  if (ret < 0) {
    printf("open file failure. %d\r\n", ret);
    return -1;
  }
  f_seek_start(&file);
  if (size != 0) {
    ret = lfs_file_read(&lfs, &file, buffer, size);
  } else {
    struct lfs_info nfo;
    ret = lfs_stat(&lfs, filename, &nfo);
    if (ret < 0) {
      printf("read file failure. %d\r\n", ret);
      return -1;
    }
    ret = lfs_file_read(&lfs, &file, buffer, nfo.size);
  }
  if (ret < 0) {
    printf("read file failure. %d\r\n", ret);
    ret = lfs_file_close(&lfs, &file);
    return -1;
  }
  if (actual_size != NULL) {
    *actual_size = ret;
  }
  ret = lfs_file_close(&lfs, &file);
  if (ret < 0) {
    printf("close file failure. %d\r\n", ret);
    return -1;
  }
  return 0;
}

int delete_file(const char* filename) {
  return lfs_remove(&lfs, (char*) filename);
}

int append_file(const char* filename, uint32_t addr, void* buffer, uint32_t size) {
  uint32_t packet = 0, cur_len = 0, total_len = 0, packet_len = VFS_WRITE_MAX_SIZE;
  total_len = size;
  packet = total_len % VFS_WRITE_MAX_SIZE == 0 ? total_len / VFS_WRITE_MAX_SIZE : total_len / VFS_WRITE_MAX_SIZE + 1;
  if (total_len < VFS_WRITE_MAX_SIZE) {
    packet_len = total_len;
  }
  lfs_file_t file;
  int ret = lfs_file_open(&lfs, &file, (char*) filename, LFS_O_CREAT | LFS_O_WRONLY | LFS_O_APPEND);
  if (ret < 0) {
    printf("open file failure. %d\r\n", ret);
    return -1;
  }
  uint8_t retry = 0;
  for (uint32_t i = 0; i < packet; i++) {
    ret = lfs_file_write(&lfs, &file, (void*) ((uint8_t*) buffer + i * VFS_WRITE_MAX_SIZE), packet_len);
    if (ret < 0) {
      printf("[vfs] write file failure. %d\r\n", ret);
      if (++retry > 3) {
        printf("[vfs] write file failure. retry >3 %d\r\n", ret);
        lfs_file_close(&lfs, &file);
        return -1;
      }
      continue;
    }
    retry = 0;
    cur_len += packet_len;
    packet_len = total_len - cur_len >= VFS_WRITE_MAX_SIZE ? VFS_WRITE_MAX_SIZE : total_len - cur_len;
    printf("[vfs] save file seq:%lu,total packet:%lu,need packet:%lu,total size:%lu,cur size:%lu,need size:%lu,start addr:%08X,end addr:%08X", i, packet,
           packet - i, total_len, cur_len, total_len - cur_len, i * VFS_WRITE_MAX_SIZE, i * VFS_WRITE_MAX_SIZE + packet_len);
  }
  lfs_file_close(&lfs, &file);
  return 0;
}

int save_file_end(const char* filename, void* buffer, uint32_t size) {
#if VFS_USE_OS == 1
  uint32_t last_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
#endif
  lfs_file_t file;
  int ret = f_open(&file, filename, LFS_O_CREAT | LFS_O_RDWR | LFS_O_APPEND);
  if (ret < 0) {
    printf("fopen failure:%d\r\n", ret);
    return -1;
  }

  uint32_t packet = 0, cur_len = 0, total_len = 0, packet_len = VFS_WRITE_MAX_SIZE;
  total_len = size;
  packet = total_len % VFS_WRITE_MAX_SIZE == 0 ? total_len / VFS_WRITE_MAX_SIZE : total_len / VFS_WRITE_MAX_SIZE + 1;
  if (total_len < VFS_WRITE_MAX_SIZE) {
    packet_len = total_len;
  }
  uint8_t retry = 0;
  for (uint32_t i = 0; i < packet; i++) {
    vfs_call_iwdg();
    ret = lfs_file_write(&lfs, &file, (uint8_t*) buffer + i * VFS_WRITE_MAX_SIZE, packet_len);
    if (ret < 0) {
      i--;
      printf("[vfs] write file failure. %d\r\n", ret);
      if (++retry > 3) {
        printf("[vfs] write file failure. retry >3 %d\r\n", ret);
        f_close(&file);
        return -1;
      }
      continue;
    }
    f_sync(&file);
    retry = 0;
    cur_len += packet_len;
    packet_len = total_len - cur_len >= VFS_WRITE_MAX_SIZE ? VFS_WRITE_MAX_SIZE : total_len - cur_len;
    printf("[vfs] save file seq:%lu,total packet:%lu,need packet:%lu,total size:%lu,cur size:%lu,need size:%lu,start addr:%08X,end addr:%08X\r\n", i, packet,
           packet - i, total_len, cur_len, total_len - cur_len, i * VFS_WRITE_MAX_SIZE, i * VFS_WRITE_MAX_SIZE + packet_len);
#if VFS_USE_OS == 1
    osDelay(1);
#else
    HAL_Delay(1);
#endif
  }
  f_close(&file);
#if VFS_USE_OS == 1
  uint32_t time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  printf("[vfs] save %s %s cost %lu ms\r\n", filename, ret < 0 ? "failure" : "success", time - last_time);
#endif
  return 0;
}

int read_file_with_addr(const char* filename, uint32_t addr, void* buffer, uint32_t size, uint32_t* actual_size) {
  if (size == 0) {
    return -1;
  }
  lfs_file_t file;
  int ret = f_open(&file, (char*) filename, LFS_O_RDONLY);
  if (ret < 0) {
    printf("open file failure. %d\r\n", ret);
    return -1;
  }
  ret = lfs_file_seek(&lfs, &file, addr, LFS_SEEK_SET);
  if (ret < 0) {
    printf("read seek failure. %d\r\n", ret);
    ret = lfs_file_close(&lfs, &file);
    return -1;
  }
  ret = lfs_file_read(&lfs, &file, buffer, size);
  if (ret < 0) {
    printf("read file failure. %d\r\n", ret);
    ret = lfs_file_close(&lfs, &file);
    return -1;
  }
  printf("read %s. file size:%d\r\n", filename, ret);
  if (actual_size != NULL) {
    *actual_size = ret;
  }
  ret = lfs_file_close(&lfs, &file);
  if (ret < 0) {
    printf("close file failure. %d\r\n", ret);
    return -1;
  }
  return 0;
}

int read_file_size(const char* filename, uint32_t* size) {
  if (size == NULL) {
    return -1;
  }
#if defined(USE_LFS)
  struct lfs_info fno;
  int ret = f_stat(filename, &fno);
  if (ret < 0) {
    printf("f_stat failure:%d\r\n", ret);
    return -1;  // 打开文件失败
  }
  *size = fno.size;
#endif
  return 0;
}
#endif

#endif
