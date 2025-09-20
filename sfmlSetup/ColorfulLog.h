#pragma once
#include <cstdio>
#include <cstdarg>
#ifdef _WIN32
    #include<Windows.h>
#endif

#define WHITE   "\033[1;37m"
#define GRAY    "\033[1;30m"
#define RED     "\033[0;31m"
#define YELLOW  "\033[1;33m"
#define GREEN   "\033[1;32m"
#define RESET   "\033[0m"
#define COLOR_OFF "\033[0m"

//一些其他的颜色
#define BLACK "\033[0;30m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define WHITE "\033[0;37m"

#define BOLD_BLACK "\033[1;30m"
#define BOLD_RED "\033[1;31m"
#define BOLD_GREEN "\033[1;32m"
#define BOLD_YELLOW "\033[1;33m"
#define BOLD_BLUE "\033[1;34m"
#define BOLD_PURPLE "\033[1;35m"
#define BOLD_CYAN "\033[1;36m"
#define BOLD_WHITE "\033[1;37m"

#define UNDERLINE_BLACK "\033[4;30m"
#define UNDERLINE_RED "\033[4;31m"
#define UNDERLINE_GREEN "\033[4;32m"
#define UNDERLINE_YELLOW "\033[4;33m"
#define UNDERLINE_BLUE "\033[4;34m"
#define UNDERLINE_PURPLE "\033[4;35m"
#define UNDERLINE_CYAN "\033[4;36m"
#define UNDERLINE_WHITE "\033[4;37m"

#define BACKGROUND_BLACK "\033[40m"
#define BACKGROUND_RED "\033[41m"
#define BACKGROUND_GREEN "\033[42m"
#define BACKGROUND_YELLOW "\033[43m"
#define BACKGROUND_BLUE "\033[44m"
#define BACKGROUND_PURPLE "\033[45m"
#define BACKGROUND_CYAN "\033[46m"
#define BACKGROUND_WHITE "\033[47m"

#define INTENSE_BLACK "\033[0;90m"
#define INTENSE_RED "\033[0;91m"
#define INTENSE_GREEN "\033[0;92m"
#define INTENSE_YELLOW "\033[0;93m"
#define INTENSE_BLUE "\033[0;94m"
#define INTENSE_PURPLE "\033[0;95m"
#define INTENSE_CYAN "\033[0;96m"
#define INTENSE_WHITE "\033[0;97m"

#define BOLD_INTENSE_BLACK "\033[1;90m"
#define BOLD_INTENSE_RED "\033[1;91m"
#define BOLD_INTENSE_GREEN "\033[1;92m"
#define BOLD_INTENSE_YELLOW "\033[1;93m"
#define BOLD_INTENSE_BLUE "\033[1;94m"
#define BOLD_INTENSE_PURPLE "\033[1;95m"
#define BOLD_INTENSE_CYAN "\033[1;96m"
#define BOLD_INTENSE_WHITE "\033[1;97m"

#define BACKGROUND_INTENSE_BLACK "\033[0;100m"
#define BACKGROUND_INTENSE_RED "\033[0;101m"
#define BACKGROUND_INTENSE_GREEN "\033[0;102m"
#define BACKGROUND_INTENSE_YELLOW "\033[0;103m"
#define BACKGROUND_INTENSE_BLUE "\033[0;104m"
#define BACKGROUND_INTENSE_PURPLE "\033[10;95m"
#define BACKGROUND_INTENSE_CYAN "\033[0;106m"
#define BACKGROUND_INTENSE_WHITE "\033[0;107m"

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

inline void setConsoleTitle(const std::string& title) {
    #ifdef _WIN32
        SetConsoleTitleA(title.c_str());
    #else
        std::cout << "\033]0;" << title << "\007";
    #endif
}

#define INFO(fmt, ...)      logMessage(WHITE,  "INFO",    __FILE__, __LINE__, fmt, false, ##__VA_ARGS__)
#define DEBUG(fmt, ...)     logMessage(GRAY,   "DEBUG",   __FILE__, __LINE__, fmt, false, ##__VA_ARGS__)
#define WARN(fmt, ...)      logMessage(YELLOW, "WARN",    __FILE__, __LINE__, fmt, true,  ##__VA_ARGS__)
#define ERROR(fmt, ...)     logMessage(RED,    "ERROR",   __FILE__, __LINE__, fmt, true,  ##__VA_ARGS__)
#define SUCCESS(fmt, ...)   logMessage(GREEN,  "SUCCESS", __FILE__, __LINE__, fmt, false, ##__VA_ARGS__) 