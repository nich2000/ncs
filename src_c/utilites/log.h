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
#define LOG_CRITICAL_ERROR  0
#define LOG_ERROR           1
#define LOG_WARNING         2
#define LOG_DEBUG           3
#define LOG_INFO            4
#define LOG_DATA            5
#define LOG_RAW_DATA        6
//==============================================================================
int clr_scr();
void skip_lines(size_t count);
//==============================================================================
void log_set_name(char *name);
void log_add(char *message, int log_type);
//==============================================================================
void report_set_name(char *name);
void report_add(char *message);
//==============================================================================
#endif //LOG_H
