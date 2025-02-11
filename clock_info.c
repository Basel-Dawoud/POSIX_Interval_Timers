#include <time.h>
#include <stdio.h>

int main() {
    struct timespec ts;
    struct timespec res;

    // CLOCK_REALTIME example
    clock_gettime(CLOCK_REALTIME, &ts);
    clock_getres(CLOCK_REALTIME, &res);

    // Convert seconds to local time
    struct tm *tm_info = localtime(&ts.tv_sec);
    char time_string[100];
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", tm_info);

    // Print the real-time in a human-readable format
    printf("Real-time: %s.%09ld\n", time_string, ts.tv_nsec);
    printf("Real-time resolution: %ld seconds, %ld nanoseconds\n", res.tv_sec, res.tv_nsec);

    // CLOCK_MONOTONIC example
    clock_gettime(CLOCK_MONOTONIC, &ts);
    clock_getres(CLOCK_MONOTONIC, &res);
    printf("Monotonic: %ld seconds since boot\n", ts.tv_sec);
    printf("Monotonic resolution: %ld seconds, %ld nanoseconds\n", res.tv_sec, res.tv_nsec);

    return 0;
}

