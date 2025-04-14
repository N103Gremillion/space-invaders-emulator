#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <string.h>
#include <time.h>

char* get_formated_time();

// remove path from filename
#ifdef UNIX
#define __SHORT_FILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#elif defined(WINDOWS)
#define __SHORT_FILE__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __SHORT_FILE__ __FILE__
#endif

// helper macro that can be used safely in ternaries
#define _LOG_EXPR(...) ((void)(printf(__VA_ARGS__), fflush(stdout)))

// main log macros (single expression, ternary-safe)
#define __LOG__(format, loglevel, ...) \
    _LOG_EXPR("%s %-5s [%s] [%s:%d] " format "\n", get_formated_time(), loglevel, __func__, __SHORT_FILE__, __LINE__, ##__VA_ARGS__)

#define __LOG_NONEWL__(format, loglevel, ...) \
    _LOG_EXPR("%s %-5s [%s] [%s:%d] " format, get_formated_time(), loglevel, __func__, __SHORT_FILE__, __LINE__, ##__VA_ARGS__)

#define __LOG_BARE__(format, ...) \
    _LOG_EXPR(format "\n", ##__VA_ARGS__)

#define __LOG_BARE_NONEWL_(format, ...) \
    _LOG_EXPR(format, ##__VA_ARGS__)

// specific log macros
#define log_debug(format, ...) __LOG__(format, "DEBUG", ##__VA_ARGS__)
#define log_trace(format, ...) __LOG__(format, "TRACE", ##__VA_ARGS__)
#define log_warn(format, ...)  __LOG__(format, "WARN",  ##__VA_ARGS__)
#define log_error(format, ...) __LOG__(format, "ERROR", ##__VA_ARGS__)
#define log_info(format, ...)  __LOG__(format, "INFO",  ##__VA_ARGS__)
#define log_log(format, ...)   __LOG_BARE__(format, ##__VA_ARGS__)

// specific log macros with no new line
#define log_debug_nonewl(format, ...) __LOG_NONEWL__(format, "DEBUG", ##__VA_ARGS__)
#define log_trace_nonewl(format, ...) __LOG_NONEWL__(format, "TRACE", ##__VA_ARGS__)
#define log_warn_nonewl(format, ...)  __LOG_NONEWL__(format, "WARN",  ##__VA_ARGS__)
#define log_error_nonewl(format, ...) __LOG_NONEWL__(format, "ERROR", ##__VA_ARGS__)
#define log_info_nonewl(format, ...)  __LOG_NONEWL__(format, "INFO",  ##__VA_ARGS__)
#define log_log_nonewl(format, ...)   __LOG_BARE_NONEWL_(format, ##__VA_ARGS__)

#endif // LOG_H

