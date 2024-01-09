#pragma once
#include <stdint.h>

typedef struct{
    int8_t api; // -1--invalid, 0--gl, 1--gles, 2--vulkan
    int8_t major;
    int8_t minor;
}api_t;
api_t parse_api( const char* str );

uint64_t PerfGetMillisecond();
double PerfGetSecond();
typedef void (*PerfRateFunc)(unsigned count);
double PerfMeasureRate(PerfRateFunc f);
const char* PerfHumanFloat( double d );
