#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "myutils.h"

#if 0
api_t parse_api( const char* str )
{
    api_t api;
    api.api = -1;
    api.major = 0;
    api.minor = 0;

    int len = strlen( str );
    if( len == 4
        && strncmp(str, "gl", 2) == 0
        && isdigit(str[2]) && isdigit(str[3]) )
    {
        api.api = 0;
        api.major = str[2] - '0';
        api.minor = str[3] - '0';
    }
    else if( len == 6
             && strncmp(str, "gles", 4) == 0
             && isdigit(str[4]) && isdigit(str[5]) )
    {
        api.api = 1;
        api.major = str[4] - '0';
        api.minor = str[5] - '0';
    }
    else if( len == 8
             && strncmp(str, "vulkan", 6) == 0
             && isdigit(str[6]) && isdigit(str[7]) )
    {
        api.api = 3;
        api.major = str[4] - '0';
        api.minor = str[5] - '0';
    }

    return api;
}
#endif

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
double PerfMeasureRate(PerfRateFunc f)
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
        glfwPollEvents();

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
        glfwPollEvents();

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

/* Note static buffer, can only use once per printf.
 */
const char* PerfHumanFloat( double d )
{
    static char buf[80];

    if (d > 1000000000.0)
        snprintf(buf, sizeof(buf), "%.1f billion", d / 1000000000.0);
    else if (d > 1000000.0)
        snprintf(buf, sizeof(buf), "%.1f million", d / 1000000.0);
    else if (d > 1000.0)
        snprintf(buf, sizeof(buf), "%.1f thousand", d / 1000.0);
    else
        snprintf(buf, sizeof(buf), "%.1f", d);

    return buf;
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
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
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

    for (int y = 0; y < height; y++ ) {
        for (int x = 0; x < width; x++) {
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
