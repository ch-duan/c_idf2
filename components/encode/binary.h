#ifndef H_C_IDF_ENCODE_BINARY_H
#define H_C_IDF_ENCODE_BINARY_H
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
long byteToInt(uint8_t *b, int byteLen, bool isBigEndian);
void intToByte(int64_t v, uint8_t *out, int byteLen, bool isBigEndian);
uint64_t byte8ToInt(uint8_t *b, int byteLen, bool isBigEndian);
void intTo8Byte(uint64_t v, int byteLen, uint8_t *result, bool isBigEndian);
void byte_copy(uint8_t *out, uint8_t *in, int len, bool BigEndian);
void float2Byte(uint8_t *u8Arry, float floatdata, bool BigEndian);
float Byte2float(uint8_t *data, bool BigEndian);
#ifdef __cplusplus
}
#endif
#endif
