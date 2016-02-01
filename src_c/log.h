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
//==============================================================================
void set_log_name(char *name);
void set_report_name(char *name);
void add_to_log(char *message, int logType);
void add_to_report(char *message);
//==============================================================================
#endif //LOG_H
