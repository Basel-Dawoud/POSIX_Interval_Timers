#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

// Signal handler for SIGALRM
void sigalrm_handler(int signum) {
    printf("SIGALRM received: Timer expired!\n");
}

int main() {
    // Set up signal handler for SIGALRM
    signal(SIGALRM, sigalrm_handler);

    timer_t timerid;
    struct itimerspec its;

    // Set timer expiration to 2 seconds
    its.it_value.tv_sec = 2;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;  // One-time timer
    its.it_interval.tv_nsec = 0;

    // Create timer without sevp (NULL, using default SIGALRM behavior)
    if (timer_create(CLOCK_REALTIME, NULL, &timerid) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    // Set the timer
    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

    // Sleep for 3 seconds to allow the timer to expire
    printf("Waiting for timer to expire...\n");
    sleep(3); // Allow time for the timer to expire

    return 0;
}

