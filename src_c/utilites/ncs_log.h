#ifndef LOG_H
#define LOG_H
//==============================================================================
#include <stdio.h>
#include <time.h>

#include "defines.h"
//==============================================================================
#define LOG_INITIAL_PATH    "../logs"
#define REPORT_INITIAL_PATH "../reports"
//==============================================================================
#define LOG_INFO            100
#define LOG_WARNING         101
#define LOG_ERROR           102
#define LOG_ERROR_CRITICAL  103
#define LOG_ERROR_FATAL     104
//==============================================================================
#define LOG_EXTRA           10
#define LOG_DEBUG           11
#define LOG_DATA            12
#define LOG_RAW_DATA        13
//==============================================================================
void log_clr_scr();
void log_set_name(char *name);
void log_add(char *message, int log_type);
void log_add_fmt(int log_type, char *message, ...);
//==============================================================================
void report_set_name(char *name);
void report_add(char *message);
//==============================================================================
#endif //LOG_H
