//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: ncs_log.h
 */
//==============================================================================
#ifndef LOG_H
#define LOG_H
//==============================================================================
#include "defines.h"
#include "globals.h"

// for FILE
#include <stdio.h>
//==============================================================================
#define LOG_NAME_ONLY       0
#define LOG_NAME_DATE       1
#define LOG_NAME_TIME       2
#define LOG_NAME_DATE_TIME  3
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
typedef int log_time_tag_t;
//==============================================================================
void clr_scr();
//==============================================================================
void log_add    (int log_type, const char *message     );
void log_add_fmt(int log_type, const char *message, ...);
//==============================================================================
FILE *stat_open(char *name);
int   stat_add(FILE *file, char *message);
void  stat_close(FILE *file);
//==============================================================================
FILE *report_open(char *name);
int   report_add(FILE *file, char *message);
void  report_close(FILE *file);
//==============================================================================
#endif //LOG_H
