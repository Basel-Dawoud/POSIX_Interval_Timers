#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h> // For using strncpy

#define MAX_MSG_LEN 100

// Structure to hold the ID and message
struct msg_struct {
    int id;
    char msg[MAX_MSG_LEN];
};

// Signal handler for custom signal
void sig_handler(int signo, siginfo_t *info, void *context) {
    // The signal data passed with the notification
    union sigval sig_data = info->si_value;

    // Check if the passed data is a pointer to the struct
    if (sig_data.sival_ptr != NULL) {
        struct msg_struct *msg_data = (struct msg_struct *)sig_data.sival_ptr;
        // Print the ID and message if it is a pointer to msg_struct
        printf("Received signal %d with ID: %d and Message: %s\n", signo, msg_data->id, msg_data->msg);
    } 
    // Otherwise, treat the data as an integer
    else {
        printf("Received signal %d with Integer value: %d\n", signo, sig_data.sival_int);
    }
}

int main() {
    struct sigaction sa;
    union sigval sig_data;
    sigset_t mask;

    // Set up the sigaction struct to specify the handler and signal info
    sa.sa_flags = SA_SIGINFO;  // To get detailed signal info
    sa.sa_sigaction = sig_handler;  // Set the custom signal handler
    sigemptyset(&sa.sa_mask);

    // Install the signal handler for SIGUSR1
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Initialize the structure with ID and message
    struct msg_struct msg_data;
    msg_data.id = 101;  // Set ID
    strncpy(msg_data.msg, "This is a test message.", MAX_MSG_LEN);  // Set message

    // First signal with pointer to the msg_struct
    sig_data.sival_ptr = (void *)&msg_data;  // Set pointer data (msg_struct)
    if (sigqueue(getpid(), SIGUSR1, sig_data) == -1) {
        perror("sigqueue");
        exit(EXIT_FAILURE);
    }

    // Sleep to allow time for signal processing
    sleep(1);

return 0;
}

