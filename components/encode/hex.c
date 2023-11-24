#include "hex.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void u8Increase(uint8_t v[2]) {
  if (v[1] < 255) {
    v[1]++;
  } else {
    v[0]++;
    v[1] = 0;
  }
}

uint8_t DecToBCD(uint8_t v) {
  return ((v % 10) & 0x0f) | (((v / 10 % 10) & 0x0f) << 4);
}

uint8_t BCDToDec(uint8_t v) {
  return ((v % 10) & 0x0f) | (((v / 10) & 0x0f) << 4);
}

uint8_t HEXBCDToDec(uint8_t v) {
  return ((v % 16) & 0x0f) + (((v / 16) & 0x0f) * 10);
}

void IEE754_binary32_encode(float x, uint8_t out[4], bool isBigEndian) {
  bool sign = x < 0;
  uint8_t exponent;
  uint32_t fraction;
  if (isinf(x)) {
    exponent = 0xFF;
    fraction = 0;
  } else if (isnan(x)) {  // nan check
    exponent = 0xFF;
    fraction = 0x7FFFFF;
  } else {
    if (sign)
      x = -x;
    int e = 0;
    fraction = frexp(x, &e) * ((uint32_t) 2 << 23);
    exponent = e + 126;
    if (e + 126 > 0xFF) {
      exponent = 0xFF;
      fraction = 0;
    }
  }
  if (isBigEndian) {
    out[0] = ((sign << 7) & 0x80) | ((exponent >> 1) & 0x7F);
    out[1] = ((exponent << 7) & 0x80) | ((fraction >> 16) & 0x7F);
    out[2] = (fraction >> 8) & 0xFF;
    out[3] = fraction & 0xFF;
  } else {
    out[3] = ((sign << 7) & 0x80) | ((exponent >> 1) & 0x7F);
    out[2] = ((exponent << 7) & 0x80) | ((fraction >> 16) & 0x7F);
    out[1] = (fraction >> 8) & 0xFF;
    out[0] = fraction & 0xFF;
  }
}

float hexToIEEEFloat(uint32_t value) {
  IEEEFLOAT_T new_float;
  float new_value = 1;

  /* Store value into struct */
  new_float.value = value;
  new_value = (1.0 + new_float.mantissa * pow(2.0, -23)) * pow(2.0, new_float.exponent - 127);

  /* Return negative value if sign bit is 1 */
  return new_float.sign ? -new_value : new_value;
}

// fromHexChar converts a hex character into its value and return a success flag.
uint8_t FromHexChar(uint8_t *c) {
  if (*c >= '0' && *c <= '9') {
    *c = *c - '0';
    return 1;
  } else if (*c >= 'a' && *c <= 'f') {
    *c = *c - 'a' + 10;
    return 1;
  } else if (*c >= 'A' && *c <= 'F') {
    *c = *c - 'A' + 10;
    return 1;
  }
  return 0;
}

uint8_t HexStringToDecString(uint8_t *dst, uint8_t *src, int len) {
  int i = 0, j = 1;
  uint8_t a, b;

  if (len % 2 != 0) {
    return 0;
  }
  for (j = 1; j < len; j += 2) {
    a = *(src + j - 1);
    b = *(src + j);
    if (FromHexChar(&a) == 0) {
      return 0;
    }
    if (FromHexChar(&b) == 0) {
      return 0;
    }
    *(dst + i) = (a << 4) | b;
    i++;
  }
  return 1;
}
