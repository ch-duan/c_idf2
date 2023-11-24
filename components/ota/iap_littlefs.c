#include "iap.h"

#if defined(USE_LITTLEFS)
#include "vfs.h"
#endif

#if defined(USE_LITTLEFS)

int load_app_update_status(update_pack_t *update_buffer) {
  unsigned long size = 0;
  int ret = read_file(UPDATE_STATUS_FILE_NAME, update_buffer, sizeof(update_pack_t), &size);
  if (ret != 0) {
    printf("load app update status is failure.\r\n");
    return -1;
  }
  printf_iap_info(*update_buffer);
  return 0;
}

#endif
