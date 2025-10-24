#include "gettime.h"
#include <time.h>

double gettime_monotonic() {

    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        return -1;
    }

    return ts.tv_sec + ts.tv_nsec / 1E+9;

}

double gettime_cpu_thread() {

    struct timespec ts;

    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) == -1) {
        return -1;
    }

    return ts.tv_sec + ts.tv_nsec / 1E+9;

}

double gettime_cpu_process() {

    struct timespec ts;

    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == -1) {
        return -1;
    }

    return ts.tv_sec + ts.tv_nsec / 1E+9;

}






























