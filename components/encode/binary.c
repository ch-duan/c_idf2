#include "binary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long byteToInt(uint8_t *b, int byteLen, bool isBigEndian) {
  long result = 0;
  int i = 0;
  if (isBigEndian) {
    for (i = byteLen - 1; i >= 0; i--) {
      result += (long) b[i] << (byteLen - 1 - i) * 8;
    }
  } else {
    for (i = 0; i < byteLen; i++) {
      result += (long) b[i] << i * 8;
    }
  }
  return result;
}

void intToByte(int64_t v, uint8_t *out, int byteLen, bool isBigEndian) {
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

uint64_t byte8ToInt(uint8_t *b, int byteLen, bool isBigEndian) {
  uint64_t result = 0;
  int i = 0;
  if (isBigEndian) {
    for (i = byteLen - 1; i >= 0; i--) {
      result += (uint64_t) b[i] << (byteLen - 1 - i) * 8;
    }
  } else {
    for (i = 0; i < byteLen; i++) {
      result += (uint64_t) b[i] << i * 8;
    }
  }
  return result;
}

void intTo8Byte(uint64_t v, int byteLen, uint8_t *result, bool isBigEndian) {
  int i = 0;
  if (isBigEndian) {
    for (i = byteLen - 1; i >= 0; i--) {
      result[i] = (uint8_t) (v >> (byteLen - 1 - i) * 8);
    }
  } else {
    for (i = 0; i < byteLen; i++) {
      result[i] = (uint8_t) (v >> i * 8);
    }
  }
}

void byte_copy(uint8_t *out, uint8_t *in, int len, bool BigEndian) {
  if (BigEndian) {
    while (len--) {
      *out++ = *(in + len);
    }
  } else {
    while (len--) {
      *out++ = *in++;
    }
  }
}

void float2Byte(uint8_t *u8Arry, float *floatdata, bool BigEndian) {
  uint8_t farray[4];
  *(float *) farray = *floatdata;
  if (BigEndian) {
    u8Arry[3] = farray[0];
    u8Arry[2] = farray[1];
    u8Arry[1] = farray[2];
    u8Arry[0] = farray[3];
  } else {
    u8Arry[0] = farray[0];
    u8Arry[1] = farray[1];
    u8Arry[2] = farray[2];
    u8Arry[3] = farray[3];
  }
}

float Byte2float(uint8_t *data, bool BigEndian) {
  float fa = 0;
  uint8_t uc[4];
  if (BigEndian == true) {
    uc[3] = data[0];
    uc[2] = data[1];
    uc[1] = data[2];
    uc[0] = data[3];
  } else {
    uc[0] = data[0];
    uc[1] = data[1];
    uc[2] = data[2];
    uc[3] = data[3];
  }

  memcpy(&fa, uc, 4);
  return fa;
}
