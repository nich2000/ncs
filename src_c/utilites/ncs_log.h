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
#define LOG_LONG_FORMAT          0
#define LOG_SHORT_FORMAT         1
//==============================================================================
#define LOG_NAME_ONLY            0
#define LOG_NAME_DATE            1
#define LOG_NAME_DATE_S          2
#define LOG_NAME_TIME            3
#define LOG_NAME_TIME_S          4
#define LOG_NAME_DATE_TIME       5
#define LOG_NAME_DATE_TIME_S     6
//==============================================================================
#define LOG_INFO                 100
#define LOG_WAIT                 101
#define LOG_WARNING              102
#define LOG_ERROR                103
#define LOG_ERROR_CRITICAL       104
#define LOG_ERROR_FATAL          105
//==============================================================================
#define LOG_CMD                  10
#define LOG_EXTRA                11
#define LOG_DEBUG                12
#define LOG_DATA                 13
#define LOG_RAW_DATA             14
//==============================================================================
#define DEFAULT_LOG_PATH         "../logs"
#define DEFAULT_LOG_NAME         "log.txt"
//==============================================================================
#define DEFAULT_STAT_PATH        "../stats"
#define DEFAULT_STAT_NAME        "stat.txt"
//==============================================================================
#define DEFAULT_REPORT_PATH      "../reports"
#define DEFAULT_REPORT_NAME      "report.txt"
//==============================================================================
#define DEFAULT_SESSION_PATH     "../sessions"
#define DEFAULT_SESSION_NAME     "session.txt"
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
FILE *session_open(char *name);
int   session_add(FILE *file, char *message);
void  session_close(FILE *file);
//==============================================================================
FILE *report_open(char *name);
int   report_add(FILE *file, char *message);
void  report_close(FILE *file);
//==============================================================================
#endif //LOG_H
