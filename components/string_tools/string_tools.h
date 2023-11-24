#ifndef CH_STRING_H
#define CH_STRING_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int lastIndex(char *data, char sep, int len);
int indexAny(char *data, char sep, int num, int len);

int hexstringtobyte(char *in, unsigned char *out, int len);
#ifdef __cplusplus
}
#endif
#endif
