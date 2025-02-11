#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include <time.h>
#include "curr_time.h"      /* Custom time formatting function */
#include "itimerspec_from_str.h" /* For parsing timer specs */
#include "tlpi_hdr.h"       /* Error handling functions */

#define TIMER_SIG SIGUSR1  /* Change to SIGUSR1 for compatibility */

/* Signal handler for timer expiration */
static void handler(int sig, siginfo_t *si, void *uc)
{
    timer_t *tidptr;

    /* Get timer ID from signal value */
    tidptr = si->si_value.sival_ptr;

    /* Debugging output */
    printf("Received signal %d at %s\n", sig, currTime("%T"));
    printf("Timer ID: %ld\n", (long)*tidptr);
    printf("Overrun: %d\n", timer_getoverrun(*tidptr));
}

int main(int argc, char *argv[])
{
    struct itimerspec ts;       /* Timer interval specification */
    struct sigaction sa;        /* Signal action structure */
    struct sigevent sev;        /* Timer notification spec */
    timer_t *tidlist;           /* Array of timer IDs */
    int j;

    if (argc < 2)
        usageErr("%s secs[/nsecs][:int-secs[/int-nsecs]]...\\n", argv[0]);

    /* Allocate array for timer IDs */
    tidlist = calloc(argc - 1, sizeof(timer_t));
    if (tidlist == NULL)
        errExit("malloc");

    /* Set up signal handler */
    sa.sa_flags = SA_SIGINFO;    /* Use extended signal handler */
    sa.sa_sigaction = handler;   /* Specify handler function */
    sigemptyset(&sa.sa_mask);    /* Don't block other signals */

    if (sigaction(TIMER_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    /* Configure timer notification */
    sev.sigev_notify = SIGEV_SIGNAL;  /* Notify via signal */
    sev.sigev_signo = TIMER_SIG;      /* Use our chosen signal */

    /* Create and start timers for each command-line argument */
    for (j = 0; j < argc - 1; j++) {
        /* Parse timer spec from command-line argument */
        itimerspecFromStr(argv[j + 1], &ts);

        /* Debugging: print the parsed timer spec */
        printf("Setting timer with expiration: %ld sec and %ld nsec\n", ts.it_value.tv_sec, ts.it_value.tv_nsec);

        /* Store timer ID in array (passed to handler via sival_ptr) */
        sev.sigev_value.sival_ptr = &tidlist[j];

        /* Create timer using system clock */
        if (timer_create(CLOCK_REALTIME, &sev, &tidlist[j]) == -1)
            errExit("timer_create");

        printf("Timer %d created with ID: %ld\n", j + 1, (long)tidlist[j]);

        /* Arm (start) the timer */
        if (timer_settime(tidlist[j], 0, &ts, NULL) == -1)
            errExit("timer_settime");
    }

    /* Infinite loop to wait for signals */
    for (;;)
        pause();
}

