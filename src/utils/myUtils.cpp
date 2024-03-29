#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "myUtils.h"


static int isnumber( const char* str )
{
    int len = strlen( str );
    for( int i=0; i < len; i++ ){
        if( isdigit(str[i]) == 0 )
            return 0; // is not a number
    }

    return 1; // is a number
}

int argsContain( const char* argName, int argc, const char* argv[] )
{
    for( int i=0; i < argc; i++ ){
        if( (strcmp( argName, argv[i] ) == 0) && ((i+1) < argc) ){
            //if found
            return 1;
        }
    }

    return 0;
}

const char* stringFromArgs( const char* argName, int argc, const char* argv[] )
{
    for( int i=0; i < argc; i++ ){
        if( (strcmp( argName, argv[i] ) == 0) && ((i+1) < argc) ){
            //if found
            return argv[i+1];
        }
    }

    return NULL;
}

int integerFromArgs( const char* argName, int argc, const char* argv[], int *isExist )
{
    for( int i=0; i < argc; i++ ){
        if( (strcmp( argName, argv[i] ) == 0) && ((i+1) < argc) && isnumber(argv[i+1]) ){
            //if found
            if( isExist != NULL )
                *isExist = 1;
            return atoi( argv[i+1] );
        }
    }

    if( isExist != NULL )
        *isExist = 0;
    return -1;
}

api_t apiDefault( int api_current )
{
    api_t api;

    switch( api_current ){
        case API_GLLegacy:
            api.api = API_GLLegacy;
            api.major = 3;
            api.minor = 0;
            break;

        case API_GL:
            api.api = API_GL;
            api.major = 3;
            api.minor = 3;
            break;

        case API_GLES:
            api.api = API_GLES;
            api.major = 3;
            api.minor = 2;
            break;

        case API_VULKAN:
            api.api = API_VULKAN;
            api.major = 1;
            api.minor = 3;
            break;

        default:
            api.api = API_Invalid;
            api.major = 0;
            api.minor = 0;
            break;
    }

    return api;
}

api_t apiFromString( const char* str )
{
    api_t api;
    api.api = API_Invalid;
    api.major = 0;
    api.minor = 0;

    int len = strlen( str );
    if( len == 4
        && strncmp(str, "gl", 2) == 0
        && isdigit(str[2]) && isdigit(str[3]) )
    {
        api.api = API_GL;
        api.major = str[2] - '0';
        api.minor = str[3] - '0';

        if( api.major == 3 && api.minor == 0 )
            api.api = API_GLLegacy;
    }
    else if( len == 6
             && strncmp(str, "gles", 4) == 0
             && isdigit(str[4]) && isdigit(str[5]) )
    {
        api.api = API_GLES;
        api.major = str[4] - '0';
        api.minor = str[5] - '0';
    }
    else if( len == 8
             && strncmp(str, "vulkan", 6) == 0
             && isdigit(str[6]) && isdigit(str[7]) )
    {
        api.api = API_VULKAN;
        api.major = str[6] - '0';
        api.minor = str[7] - '0';
    }

    return api;
}


api_t apiInitial( int api_current, int argc, const char* argv[] )
{
    // default value
    api_t api = apiDefault( api_current );

    // may change by command line
    const char* apiValue = stringFromArgs( "--api", argc, argv );
    if( apiValue != NULL ){
        api = apiFromString( apiValue );

        if( api.api != api_current ){
            printf("%s: \"--api %s\" is invalid for \"%s\"\n", __func__, apiValue, apiName(api_current));
            exit( 1 );
        }
    }

    return api;
}

const char* apiName( api_t api )
{
    char name[32];
    sprintf( name, "%s %d.%d",
             (api.api == API_GLLegacy) ? "glLegacy" : (api.api == API_GL) ? "gl" : (api.api == API_GLES) ? "gles" : "vulkan",
             api.major, api.minor);
    return strdup( name );
}

const char* apiName( int api )
{
    switch( api ){
        case API_GLLegacy: return "glLegacy";
        case API_GL: return "gl";
        case API_GLES: return "gles";
        case API_VULKAN: return "vulkan";
        default:
            return "";
    }
}

uint64_t PerfGetMillisecond()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_nsec/1000000 + now.tv_sec*1000;
}

double PerfGetSecond()
{
    //return glutGet(GLUT_ELAPSED_TIME) * 0.001;
    return (double)PerfGetMillisecond() * 0.001;
}

/**
 * Run function 'f' for enough iterations to reach a steady state.
 * Return the rate (iterations/second).
 */
double PerfMeasureRate(PerfRateFunc f, PollEventFunc poolevent)
{
    const double minDuration = 1.0;
    double rate = 0.0, prevRate = 0.0;
    unsigned subiters;

    /* Compute initial number of iterations to try.
     * If the test function is pretty slow this helps to avoid
     * extraordarily long run times.
     */
    subiters = 2;
    {
        if( poolevent )
            poolevent();

        const double t0 = PerfGetSecond();
        double t1;
        do {
            f(subiters); /* call the rendering function */
            t1 = PerfGetSecond();
            subiters *= 2;
        } while (t1 - t0 < 0.1 * minDuration);
    }
    //printf("initial subIters = %u\n", subiters);

    while (1) {
        if( poolevent )
            poolevent();

        const double t0 = PerfGetSecond();
        unsigned iters = 0;
        double t1;

        do {
            //printf("f( subiters=%u )\n", subiters);//XXX
            f(subiters); /* call the rendering function */
            t1 = PerfGetSecond();
            iters += subiters;
        } while (t1 - t0 < minDuration);

        rate = iters / (t1 - t0);

        //printf("prevRate %f  rate  %f  ratio %f  iters %u\n", prevRate, rate, rate/prevRate, iters);

        /* Try and speed the search up by skipping a few steps:
         */
        if (rate > prevRate * 1.6)
            subiters *= 8;
        else if (rate > prevRate * 1.2)
            subiters *= 4;
        else if (rate > prevRate * 1.05)
            subiters *= 2;
        else
            break;

        prevRate = rate;
    }

    //printf("%s returning iters %u  rate %f\n", __FUNCTION__, subiters, rate);
    return rate;
}

const char* PerfHumanFloat( double d )
{
    char buf[128];

    if (d > 1000000000.0)
        snprintf(buf, sizeof(buf), "%.1f billion", d / 1000000000.0);
    else if (d > 1000000.0)
        snprintf(buf, sizeof(buf), "%.1f million", d / 1000000.0);
    else if (d > 1000.0)
        snprintf(buf, sizeof(buf), "%.1f thousand", d / 1000.0);
    else
        snprintf(buf, sizeof(buf), "%.1f", d);

    return strdup( buf );
}

float DegreeFromRadian( float radian )
{
    return radian * 180 / M_PI;
}

float RadianFromDegree( float degree )
{
    return degree * M_PI / 180;
}


uint8_t *GenerateCheckboard_RGBA( int width, int height, int checkSize )
{
    uint8_t *pixels = (uint8_t*) malloc( width * height * 4 );
    if( pixels == NULL )
        return NULL;

    int k = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t color;
            if (((y / checkSize) ^ (x / checkSize)) & 1)
                color = 0xff;
            else
                color = 0x0;

            pixels[k++] = color;
            pixels[k++] = color;
            pixels[k++] = color;
            pixels[k++] = color;
        }
    }

    return pixels;
}

uint8_t *GenerateCheckboard_RGB( int width, int height, int checkSize )
{
    uint8_t *pixels = (uint8_t*) malloc( width * height * 3 );
    if( pixels == NULL )
        return NULL;

    for (int y = 0; y < height; y++ )
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t rColor = 0;
            uint8_t bColor = 0;

            if ((x / checkSize) % 2 == 0) {
                rColor = 255 * ((y / checkSize) % 2);
                bColor = 255 * (1 - ((y / checkSize) % 2));
            } else {
                bColor = 255 * ((y / checkSize) % 2);
                rColor = 255 * (1 - ((y / checkSize) % 2));
            }

            pixels[(y * width + x) * 3] = rColor;
            pixels[(y * width + x) * 3 + 1] = 0;
            pixels[(y * width + x) * 3 + 2] = bColor;
        }
    }

    return pixels;
}

int GenerateNextLevelMipmap_RGB( const uint8_t *src, uint8_t **dst, int srcWidth, int srcHeight, int *dstWidth, int *dstHeight )
{
    const int texelSize = 3;

    *dstWidth = srcWidth / 2;
    if ( *dstWidth <= 0 )
    {
        *dstWidth = 1;
    }

    *dstHeight = srcHeight / 2;
    if ( *dstHeight <= 0 )
    {
        *dstHeight = 1;
    }

    *dst = (uint8_t*) malloc( sizeof (uint8_t) * texelSize * (*dstWidth) * (*dstHeight) );
    if ( *dst == NULL )
    {
        return 0;
    }

    for (int y = 0; y < *dstHeight; y++ )
    {
        for (int x = 0; x < *dstWidth; x++ )
        {
            int srcIndex[4];
            float r = 0.0f;
            float g = 0.0f;
            float b = 0.0f;
            int sample;

            // Compute the offsets for 2x2 grid of pixels in previous
            // image to perform box filter
            srcIndex[0] = ( (((y * 2) + 0) * srcWidth) + (x * 2 + 0) ) * texelSize;
            srcIndex[1] = ( (((y * 2) + 0) * srcWidth) + (x * 2 + 1) ) * texelSize;
            srcIndex[2] = ( (((y * 2) + 1) * srcWidth) + (x * 2 + 0) ) * texelSize;
            srcIndex[3] = ( (((y * 2) + 1) * srcWidth) + (x * 2 + 1) ) * texelSize;

            // Sum all pixels
            for ( sample = 0; sample < 4; sample++ )
            {
                r += src[ srcIndex[sample] + 0];
                g += src[ srcIndex[sample] + 1];
                b += src[ srcIndex[sample] + 2];
            }

            // Average results
            r /= 4.0;
            g /= 4.0;
            b /= 4.0;

            // Store resulting pixels
            (*dst)[ (y * ( *dstWidth ) + x) * texelSize + 0] = (uint8_t) r;
            (*dst)[ (y * ( *dstWidth ) + x) * texelSize + 1] = (uint8_t) g;
            (*dst)[ (y * ( *dstWidth ) + x) * texelSize + 2] = (uint8_t) b;
        }
    }

    return 1;
}

