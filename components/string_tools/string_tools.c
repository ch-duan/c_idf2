#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strings.h"

int lastIndex(char *data, char sep, int len) {
  int i = 0;
  for (i = len; i >= 0; i--) {
    if (data[i] == sep) {
      return i;
    }
  }
  return -1;
}

// 从后往前搜索
int indexAny(char *data, char sep, int num, int len) {
  int i = 0;
  int count = 0;
  for (i = len; i >= 0; i--) {
    if (data[i] == sep) {
      count++;
      if (count == num) {
        return i;
      }
    }
  }
  return -1;
}

int hexstringtobyte(char *in, unsigned char *out, int len) {
  char *str = (char *) malloc(len);
  memset(str, 0, len);
  memcpy(str, in, len);
  for (int i = 0; i < len; i += 2) {
    // 小写转大写
    if (str[i] >= 'a' && str[i] <= 'f')
      str[i] = str[i] & ~0x20;
    if (str[i + 1] >= 'a' && str[i] <= 'f')
      str[i + 1] = str[i + 1] & ~0x20;
    // 处理第前4位
    if (str[i] >= 'A' && str[i] <= 'F')
      out[i / 2] = (str[i] - 'A' + 10) << 4;
    else
      out[i / 2] = (str[i] & ~0x30) << 4;
    // 处理后4位, 并组合起来
    if (str[i + 1] >= 'A' && str[i + 1] <= 'F')
      out[i / 2] |= (str[i + 1] - 'A' + 10);
    else
      out[i / 2] |= (str[i + 1] & ~0x30);
  }
  free(str);
  return 0;
}
