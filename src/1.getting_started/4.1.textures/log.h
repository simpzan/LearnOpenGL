#pragma once

#include <unistd.h>
#include <stdio.h>
#include <time.h>
int getTimeString(char *buffer, int length) {
    time_t timer;
    time(&timer);
    struct tm tm_info;
    localtime_r(&timer, &tm_info);
    strftime(buffer, length, "%m-%d %H:%M:%S", &tm_info);
    return 0;
}

int getTimeString(char *buffer, int length);
int64_t gettid() { return 0; }
#define LOG_PRINT(level, format, args...) do { \
    char buffer[16] = {0}; getTimeString(buffer, 16); \
    printf("%s %s %d:%u %s:%d " format "\n", buffer, level, getpid(), \
        (uint32_t)gettid(), __FUNCTION__, __LINE__, ##args); \
} while (0)
#define ERRNO(format, args...) LOG_PRINT("E", format ", errno %d %s", \
    ##args, errno, strerror(errno))
#define ERROR(format, args...) LOG_PRINT("E", format, ##args)
#define WARN(format, args...) LOG_PRINT("W", format, ##args)
#define INFO(format, args...) LOG_PRINT("I", format, ##args)
#define DEBUG(format, args...) LOG_PRINT("D", format, ##args)
#define VERBOSE(format, args...) LOG_PRINT("V", format, ##args)
#define TR() INFO("")
#define TRO() INFO("%p", this) // trace C++ object