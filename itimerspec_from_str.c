// itimerspec_from_str.c (Ensure this file exists and is compiled)
#include "itimerspec_from_str.h"

void itimerspecFromStr(char *str, struct itimerspec *tsp) {
    char *cptr, *sptr;

    // Locate the ':' separator (if any)
    cptr = strchr(str, ':');
    if (cptr != NULL)
        *cptr = '\0';  // Null-terminate the initial expiration part

    // Locate the '/' separator in the initial expiration part (if any)
    sptr = strchr(str, '/');
    if (sptr != NULL)
        *sptr = '\0';  // Null-terminate the seconds part of it_value

    // Parse the initial expiration time (it_value)
    tsp->it_value.tv_sec = atoi(str);               // Parse seconds
    tsp->it_value.tv_nsec = (sptr != NULL) ? atoi(sptr + 1) : 0; // Parse nanoseconds (default: 0)

    // Parse the interval time (it_interval), if provided
    if (cptr == NULL) {  // No ':' separator means no interval part
        tsp->it_interval.tv_sec = 0;
        tsp->it_interval.tv_nsec = 0;
    } else {  // Interval part exists
        sptr = strchr(cptr + 1, '/'); // Locate '/' in the interval part
        if (sptr != NULL)
            *sptr = '\0';  // Null-terminate the seconds part of it_interval

        tsp->it_interval.tv_sec = atoi(cptr + 1);               // Parse seconds
        tsp->it_interval.tv_nsec = (sptr != NULL) ? atoi(sptr + 1) : 0; // Parse nanoseconds (default: 0)
    }
}

