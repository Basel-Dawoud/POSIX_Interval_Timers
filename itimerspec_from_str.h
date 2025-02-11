// itimerspec_from_str.h
#ifndef ITIMERSPEC_FROM_STR_H
#define ITIMERSPEC_FROM_STR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void itimerspecFromStr(char *str, struct itimerspec *tsp);

#endif

