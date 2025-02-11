#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void timer_handler(int sig, siginfo_t *si, void *uc){
	printf("Timer expired! Signal received: %d\n", sig);
	printf("Passed value: %d\n", si->si_value.sival_int);
}

int main(){
	timer_t timerid;
	clockid_t clockid = CLOCK_REALTIME;
	struct sigevent sevp;
	struct sigaction sa;
    struct itimerspec ts;
	sa.sa_sigaction = timer_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	
	if(sigaction(SIGUSR1, &sa, NULL) == -1){
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	sevp.sigev_notify = SIGEV_SIGNAL;
	sevp.sigev_signo = SIGUSR1;
	sevp.sigev_value.sival_int = 1234;

	if(timer_create(clockid, &sevp, &timerid) == -1){
		perror("timer_create");
		exit(EXIT_FAILURE);
	}

    // Set the timer to expire after 3 seconds
    ts.it_value.tv_sec = 3;      // First expiration after 3 seconds
    ts.it_value.tv_nsec = 0;     // No additional nanoseconds
    ts.it_interval.tv_sec = 0;   // No periodic expiration
    ts.it_interval.tv_nsec = 0;

    // Start the timer
    if (timer_settime(timerid, 0, &ts, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

    printf("Timer set. Waiting for signal...\n");

    // Wait for the signal (SIGUSR1)
    pause();  // Blocks and waits for signals

    printf("Program exiting.\n");
    return 0;
}

