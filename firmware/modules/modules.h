#ifndef MODULES_H
#define MODULES_H

#include <stdint.h>
#include <stdio.h>

#ifdef NDEBUG
    #define LOG(...) do {} while (0)
    #define LOG_C(...) do {} while (0)
    #define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#else
    #define LOG(...) printf(__VA_ARGS__)
    #define LOG_C(c) putchar(c)
    #define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#endif

int wifi_connect(char * ssid, char * password);
int http_connect(char * host, char * url_request);

#endif