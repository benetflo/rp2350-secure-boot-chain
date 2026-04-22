#pragma once
#include <stddef.h>


// HACL* requires memcpy and memset but they are not available in bare-metal.
// These are minimal implementations to satisfy those dependencies.

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp_ct(const void *a, const void *b, size_t n);