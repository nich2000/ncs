#ifndef LOG_H
#define LOG_H
//==============================================================================
#include <stdio.h>
#include <time.h>
//==============================================================================
#define LOG_ERROR   0
#define LOG_WARNING 1
#define LOG_DEBUG   2
#define LOG_INFO    3
#define LOG_DATA    4
//==============================================================================
int clrscr();
void skip_lines(size_t count);
//==============================================================================
void log_set_name(char *name);
void log_add(char *message, int logType);
//==============================================================================
void report_set_name(char *name);
void report_add(char *message);
//==============================================================================
#endif //LOG_H
