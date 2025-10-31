#pragma once

#define _POSIX_C_SOURCE 199309L // _POSIX_C_SOURCE should be >= 199309L for clock_gettime

double gettime_monotonic();
double gettime_cpu_thread();
double gettime_cpu_process();
