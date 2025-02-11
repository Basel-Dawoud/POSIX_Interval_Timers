#ifndef TLPI_HDR_H
#define TLPI_HDR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

// Prints an error message based on errno and exits the program
#define errExit(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0)

// Prints a formatted error message and exits the program
#define errMsg(msg) \
    do { \
        fprintf(stderr, "%s: %s\n", msg, strerror(errno)); \
        exit(EXIT_FAILURE); \
    } while (0)

// Prints a usage error message and exits the program
#define usageErr(fmt, ...) \
    do { \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)

// Checks if a function call failed, and if so, calls errExit()
#define checkError(fn) \
    if ((fn) == -1) \
        errExit(#fn " failed")

#endif

