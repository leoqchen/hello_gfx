#pragma once
#include <stdint.h>


#define API_Invalid   (-1)
#define API_GLLegacy  0
#define API_GL        1
#define API_GLES      2
#define API_VUKAN     3

typedef struct{
    int8_t api;
    int8_t major;
    int8_t minor;
}api_t;

uint64_t PerfGetMillisecond();
double PerfGetSecond();
typedef void (*PerfRateFunc)(unsigned count);
double PerfMeasureRate(PerfRateFunc f);
const char* PerfHumanFloat( double d );
