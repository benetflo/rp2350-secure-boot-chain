#include "mem.h"
#include <stdint.h>

// HACL* requires memcpy and memset but they are not available in bare-metal.
// These are minimal implementations to satisfy those dependencies.

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = dst;
    const uint8_t *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

void *memset(void *dst, int c, size_t n) {
    uint8_t *d = dst;
    while (n--) *d++ = (uint8_t)c;
    return dst;
}

int memcmp_ct(const void *a, const void *b, size_t n) 
{
    const uint8_t *pa = a;
    const uint8_t *pb = b;
    uint8_t diff = 0;
    while (n--) diff |= *pa++ ^ *pb++;
    return diff; // 0 == lika, annat == skiljer sig
}