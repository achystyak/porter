#pragma once
#include <stdio.h>
#include <chrono>
#include <ctime>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define CC_RESET "\033[0m"
#define CC_BLACK "\033[30m"              /* Black */
#define CC_RED "\033[31m"                /* Red */
#define CC_GREEN "\033[32m"              /* Green */
#define CC_YELLOW "\033[33m"             /* Yellow */
#define CC_BLUE "\033[34m"               /* Blue */
#define CC_MAGENTA "\033[35m"            /* Magenta */
#define CC_CYAN "\033[36m"               /* Cyan */
#define CC_WHITE "\033[37m"              /* White */
#define CC_BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define CC_BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define CC_BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define CC_BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define CC_BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define CC_BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define CC_BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define CC_BOLDWHITE "\033[1m\033[37m"   /* Bold White */

#define SET_STYLE(CC_COLOR) printf(CC_COLOR)
#define RESET_STYLE() SET_STYLE(CC_RESET)

static std::string CURRENT_TIME()
{
    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto str = std::string(ctime(&time));
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    return str;
}

#define Log_Stack()                                                            \
    SET_STYLE(CC_BOLDWHITE);                                                   \
    printf("%s: %s:%d: ", CURRENT_TIME().c_str(), __FILENAME__, __LINE__); \
    RESET_STYLE();

#define Log_Base(color, fmt, ...) \
    Log_Stack();                  \
    SET_STYLE(color);             \
    printf(fmt, __VA_ARGS__);     \
    RESET_STYLE();                \
    printf("%s%s", CC_RESET, "\n\n");

#define Log_Info(fmt, ...) Log_Base(CC_RESET, fmt, __VA_ARGS__)

#define Log_Success(fmt, ...) Log_Base(CC_BOLDGREEN, fmt, __VA_ARGS__)

#define Log_Debug(fmt, ...) Log_Base(CC_BOLDCYAN, fmt, __VA_ARGS__)

#define Log_Warning(fmt, ...) Log_Base(CC_BOLDYELLOW, fmt, __VA_ARGS__)

#define Log_Error(fmt, ...) Log_Base(CC_BOLDRED, fmt, __VA_ARGS__)
