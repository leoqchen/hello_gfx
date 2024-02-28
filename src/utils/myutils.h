#pragma once
#include <stdint.h>
#include "project_config.h"

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

const char* apiName( api_t api );

uint64_t PerfGetMillisecond();
double PerfGetSecond();
typedef void (*PerfRateFunc)(unsigned count);
double PerfMeasureRate(PerfRateFunc f);
const char* PerfHumanFloat( double d );

float DegreeFromRadian( float radian );
float RadianFromDegree( float degree );

uint8_t *GenerateCheckboard_RGBA( int width, int height, int checkSize );
uint8_t *GenerateCheckboard_RGB( int width, int height, int checkSize );

int GenerateNextLevelMipmap_RGB( const uint8_t *src, uint8_t **dst, int srcWidth, int srcHeight, int *dstWidth, int *dstHeight );
