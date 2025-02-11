#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

void timer_handler(int sig, siginfo_t * si, void * uc){
	printf("Timer expired! with signal NO: %d\n", sig);
	printf("Value delivered: %d\n", si->si_value.sival_int);
}


int main(){
	timer_t timerid;
	struct sigevent sevp;
	struct sigaction sa;
	struct itimerspec ts;

	/* Signal Handler */
	sa.sa_sigaction = timer_handler;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);	
	sigaction(SIGUSR1, &sa, NULL);
	
	/* Timer Configurations */
	sevp.sigev_notify = SIGEV_SIGNAL;
	sevp.sigev_signo = SIGUSR1;
	sevp.sigev_value.sival_int = 1234;
	timer_create(CLOCK_REALTIME, &sevp, &timerid);

	/* Starting Tiemr */
	ts.it_value.tv_sec = 5;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 2;
	ts.it_interval.tv_nsec = 0;

	timer_settime(timerid, 0, &ts, NULL);
	
	while(1){
		pause();
	}

	return 0;
}
