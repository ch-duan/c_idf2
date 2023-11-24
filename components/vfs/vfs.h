#pragma once

#ifndef USE_VFS
#define USE_VFS
#endif

#ifndef USE_LFS
#define USE_LFS
#endif

#ifndef VFS_USE_OS
#define VFS_USE_OS 1
#endif

#ifdef USE_VFS
#define VFS_WRITE_MAX_SIZE 4096
#define VFS_CACHE_SIZE     4096
#include <stdint.h>

typedef void (*vfs_cb_t)(void);

typedef struct {
  vfs_cb_t vfs_iwdg;
} vfs_config_t;

#ifdef __cplusplus
extern "C" {
#endif

int vfs_init(vfs_config_t vfs_config);
int save_file(const char* filename, void* buffer, uint32_t size);
int append_file(const char* filename, uint32_t addr, void* buffer, uint32_t size);
int save_file_end(const char* filename, void* buffer, uint32_t size);
int read_file(const char* filename, void* buffer, uint32_t size, uint32_t* actual_size);
int read_file_with_addr(const char* filename, uint32_t addr, void* buffer, uint32_t size, uint32_t* actual_size);
int read_file_size(const char* filename, uint32_t* size);
int delete_file(const char* filename);

#if defined(USE_LFS)
#include "lfs.h"
int f_open(lfs_file_t* fp, const char* path, int mode);                                      /* Open or create a file */
int f_close(lfs_file_t* fp);                                                                 /* Close an open file object */
int f_read(lfs_file_t* fp, void* buff, uint16_t btr, uint16_t* br);                          /* Read data from the file */
int f_write(lfs_file_t* fp, const void* buff, uint16_t btw, uint16_t* bw);                   /* Write data to the file */
int f_lseek(lfs_file_t* fp, size_t ofs);                                                     /* Move file pointer of the file object */
int f_seek_start(lfs_file_t* fp);                                                            /* Move file pointer to head of the file object */
int f_seek_end(lfs_file_t* fp);                                                              /* Move file pointer to tail of the file object */
int f_truncate(lfs_file_t* fp, size_t ofs);                                                  /* Truncate the file */
int f_sync(lfs_file_t* fp);                                                                  /* Flush cached data of the writing file */
int f_opendir(lfs_dir_t* dp, const char* path);                                              /* Open a directory */
int f_closedir(lfs_dir_t* dp);                                                               /* Close an open directory */
int f_readdir(lfs_dir_t* dp, struct lfs_info* fno);                                          /* Read a directory item */
int f_findfirst(lfs_dir_t* dp, struct lfs_info* fno, const char* path, const char* pattern); /* Find first file */
int f_findnext(lfs_dir_t* dp, struct lfs_info* fno);                                         /* Find next file */
int f_mkdir(const char* path);                                                               /* Create a sub directory */
int f_unlink(const char* path);                                                              /* Delete an existing file or directory */
int f_rename(const char* path_old, const char* path_new);                                    /* Rename/Move a file or directory */
int f_stat(const char* path, struct lfs_info* fno);                                          /* Get file status */
int f_chmod(const char* path, uint8_t attr, uint8_t mask);                                   /* Change attribute of a file/dir */
int f_utime(const char* path, const struct lfs_info* fno);                                   /* Change timestamp of a file/dir */
int f_chdir(const char* path);                                                               /* Change current directory */
int f_chdrive(const char* path);                                                             /* Change current drive */
int f_getcwd(char* buff, uint16_t len);                                                      /* Get current directory */
// int f_getfree(const char* path, uint32_t* nclst, lfs_t** fatfs);                                  /* Get number of free clusters on the drive */
int f_getlabel(const char* path, char* label, uint32_t* vsn);                                          /* Get volume label */
int f_setlabel(const char* label);                                                                     /* Set volume label */
int f_forward(lfs_file_t* fp, uint16_t (*func)(const uint8_t*, uint16_t), uint16_t btf, uint16_t* bf); /* Forward data to the stream */
int f_expand(lfs_file_t* fp, size_t szf, uint8_t opt);                                                 /* Allocate a contiguous block to the file */
int f_mount(lfs_t* fs, const char* path, uint8_t opt);                                                 /* Mount/Unmount a logical drive */
int f_mkfs(const char* path, uint8_t opt, uint32_t au, void* work, uint16_t len);                      /* Create a FAT volume */
int f_fdisk(uint8_t pdrv, const uint32_t* szt, void* work);                                            /* Divide a physical drive into some partitions */
int f_putc(char c, lfs_file_t* fp);                                                                    /* Put a character to the file */
int f_puts(const char* str, lfs_t* cp);                                                                /* Put a string to the file */
int f_printf(lfs_file_t* fp, const char* str, ...);                                                    /* Put a formatted string to the file */
char* f_gets(char* buff, int len, lfs_file_t* fp);                                                     /* Get a string from the file */
#endif

#ifdef __cplusplus
}
#endif

#endif
