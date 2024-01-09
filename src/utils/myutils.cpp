#include <time.h>
#include <stdio.h>
#include "myutils.h"

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
