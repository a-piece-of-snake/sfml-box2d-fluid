#pragma once
#include <cstdio>
#include <cstdarg>

#define WHITE   "\033[1;37m"
#define GRAY    "\033[1;30m"
#define RED     "\033[0;31m"
#define YELLOW  "\033[1;33m"
#define GREEN   "\033[1;32m"
#define RESET   "\033[0m"

inline void logMessage(const char* color, const char* level,
    const char* file, int line,
    const char* fmt, bool flush, ...)
{
    std::printf("%s[%s](%s %d) ", color, level, file, line);

    va_list args;
    va_start(args, flush);
    std::vprintf(fmt, args);
    va_end(args);

    std::printf("%s\n", RESET);

    if (flush) {
        std::fflush(stdout);
    }
}

#define INFO(fmt, ...)      logMessage(WHITE,  "INFO",    __FILE__, __LINE__, fmt, false, ##__VA_ARGS__)
#define DEBUG(fmt, ...)     logMessage(GRAY,   "DEBUG",   __FILE__, __LINE__, fmt, false, ##__VA_ARGS__)
#define WARN(fmt, ...)      logMessage(YELLOW, "WARN",    __FILE__, __LINE__, fmt, true,  ##__VA_ARGS__)
#define ERROR(fmt, ...)     logMessage(RED,    "ERROR",   __FILE__, __LINE__, fmt, true,  ##__VA_ARGS__)
#define SUCCESS(fmt, ...)   logMessage(GREEN,  "SUCCESS", __FILE__, __LINE__, fmt, false, ##__VA_ARGS__) 