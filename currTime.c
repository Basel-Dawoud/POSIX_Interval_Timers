// curr_time.h
#ifndef CURR_TIME_H
#define CURR_TIME_H

#include <time.h>
#include <stdio.h>

// Function to return the current time in a formatted string
static inline const char *currTime(const char *format) {
    static char buffer[100];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), format, timeinfo);
    return buffer;
}

#endif

