#ifndef LOG_H
#define LOG_H
//==============================================================================
#include <stdio.h>

#include "defines.h"
//==============================================================================
#define LOG_INITIAL_PATH    "../logs"
#define REPORT_INITIAL_PATH "../reports"
//==============================================================================
#define LOG_INFO            100
#define LOG_WAIT            101
#define LOG_WARNING         102
#define LOG_ERROR           103
#define LOG_ERROR_CRITICAL  104
#define LOG_ERROR_FATAL     105
//==============================================================================
#define LOG_CMD             10
#define LOG_EXTRA           11
#define LOG_DEBUG           12
#define LOG_DATA            13
#define LOG_RAW_DATA        14
//==============================================================================
void clr_scr();
void log_set_name(char *name);
void log_add(char *message, int log_type);
void log_add_fmt(int log_type, char *message, ...);
//==============================================================================
FILE *report_open(char *report_name);
int report_add(FILE *file, char *message);
void report_close(FILE *file);
//==============================================================================
#endif //LOG_H
