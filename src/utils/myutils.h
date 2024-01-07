#pragma once
#include <stdint.h>

uint64_t PerfGetMillisecond();
double PerfGetSecond();
typedef void (*PerfRateFunc)(unsigned count);
double PerfMeasureRate(PerfRateFunc f);
const char* PerfHumanFloat( double d );
