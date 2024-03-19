#pragma once

#include <stdint.h>
#include "project_config.h"

/*
 * command line arguments:
 *   --api [glXX | glesXX | vulkanXX]
 *   --draw [0 | 1]                       Enable/Disable draw
 *   --testcase XX                        Set testcase
 */
int argsContain( const char* argName, int argc, const char* argv[] );
const char* stringFromArgs( const char* argName, int argc, const char* argv[] );
int integerFromArgs( const char* argName, int argc, const char* argv[], int *isExist );


#define API_Invalid   (-1)
#define API_GLLegacy  0
#define API_GL        1
#define API_GLES      2
#define API_VULKAN    3

#if IS_GlLegacy
#define API_Current API_GLLegacy
#define API_CurrentName  "glLegacy"
#elif IS_Gl
#define API_Current API_GL
#define API_CurrentName  "gl"
#elif IS_GlEs
#define API_Current API_GLES
#define API_CurrentName  "gles"
#else
#define API_Current API_VULKAN
#define API_CurrentName  "vulkan"
#endif

typedef struct{
    int8_t api;
    int8_t major;
    int8_t minor;
}api_t;

api_t apiDefault( int api_current );
api_t apiFromString( const char* str );
api_t apiInitial( int api_current, int argc, const char* argv[] );
const char* apiName( api_t api );
const char* apiName( int api );

uint64_t PerfGetMillisecond();
double PerfGetSecond();
typedef void (*PerfRateFunc)(unsigned count);
typedef void (*PollEventFunc)(void);
double PerfMeasureRate(PerfRateFunc f, PollEventFunc poolevent = NULL);
const char* PerfHumanFloat( double d );

float DegreeFromRadian( float radian );
float RadianFromDegree( float degree );

uint8_t *GenerateCheckboard_RGBA( int width, int height, int checkSize );
uint8_t *GenerateCheckboard_RGB( int width, int height, int checkSize );

int GenerateNextLevelMipmap_RGB( const uint8_t *src, uint8_t **dst, int srcWidth, int srcHeight, int *dstWidth, int *dstHeight );
