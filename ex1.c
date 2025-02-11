#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

void timer_handler(int sig, siginfo_t *si, void * cu){
	printf("Timer Expired With Signal Number: %d\n", sig);
	printf("Value Delivered: %d\n", si->si_value.sival_int);
}

int main(){
	timer_t timerid;
	struct sigevent sevp;
	struct sigaction sa;
	struct itimerspec ts;

	/* Signal Handler */
	sa.sa_sigaction = timer_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sa, NULL);
	
	/* Timer Configurations */
	sevp.sigev_notify = SIGEV_SIGNAL;
	sevp.sigev_signo = SIGUSR1;
	sevp.sigev_value.sival_int = 12512;\
	
	timer_create(CLOCK_REALTIME, &sevp, &timerid);
	
	/* Setting Time for The Timer */
	ts.it_value.tv_sec = 5;
	ts.it_value.tv_nsec = 5000000;
	ts.it_interval.tv_sec = 2;
	ts.it_interval.tv_nsec = 0;
	/* Start The Timer */
	timer_settime(timerid, 0, &ts, NULL);
	
	while(1){
		pause();
	}

	return 0;
}
