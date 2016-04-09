//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#if defined(__linux__) || defined(_WIN32)
#include <unistd.h>
#endif

#ifdef __linux__
#include <sys/stat.h>
#include <stdlib.h>
#elif _WIN32
#include <windows.h>
#endif

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "ncs_log.h"
//==============================================================================
static char log_def_name[64] = "log.txt";
//==============================================================================
void clr_scr()
{
  #ifdef __linux__
    system("clear");
  #elif _WIN32
    system("cls");
  #endif
}
//==============================================================================
void log_make_dir()
{
  #ifdef __linux__
  mkdir(LOG_INITIAL_PATH, 0777);
  #elif _WIN32
  CreateDirectory(LOG_INITIAL_PATH, NULL);
  #endif
}
//==============================================================================
void log_set_name(char *name)
{
  strncpy(log_def_name, name, 64);
}
//==============================================================================
void log_add_fmt(int log_type, char *message, ...)
{
  char tmp[102400];

  va_list params;
  va_start(params, message);
  vsprintf(tmp, message, params);
  va_end(params);

  log_add(tmp, log_type);
}
//==============================================================================
const char *log_type_to_string(int log_type)
{
  switch(log_type)
  {
    case LOG_INFO:           return "[INFO]";
    break;
    case LOG_WAIT:           return "[WAIT]";
    break;
    case LOG_WARNING:        return "[WARNING]";
    break;
    case LOG_ERROR:          return "[ERROR]";
    break;
    case LOG_ERROR_CRITICAL: return "[ERROR_CRITICAL]";
    break;
    case LOG_ERROR_FATAL:    return "[ERROR_FATAL]";
    break;
    case LOG_CMD:            return "[CMD]";
    break;
    case LOG_EXTRA:          return "[EXTRA]";
    break;
    case LOG_DEBUG:          return "[DEBUG]";
    break;
    case LOG_DATA:           return "[DATA]";
    break;
    case LOG_RAW_DATA:       return "";
    break;
  }

  return "";
}
//==============================================================================
void log_add(char *message, int log_type)
{
  #ifndef DEBUG_MODE
  if(log_type == LOG_DEBUG)
    return;
  #endif

  #ifndef USE_EXTRA_LOGS
    if(log_type == LOG_EXTRA)
      return;
  #endif

  #ifndef USE_WAIT_LOGS
    if(log_type == LOG_WAIT)
      return;
  #endif

  log_make_dir();

  char *t = "";
#if defined(__linux__) || defined(_WIN32)
//  time_t rawtime;
//  struct tm *timeinfo;

//  time(&rawtime);
//  timeinfo = localtime(&rawtime);

//  strftime(t, 128, "%T", timeinfo);
#endif

  const char *prefix = log_type_to_string(log_type);

  if(
     (log_type == LOG_INFO)
     ||
     (log_type == LOG_CMD)
     #ifndef SILENT_MODE
     || (log_type >= LOG_ERROR_CRITICAL)
     #endif
    )
    printf("%s %s\n", prefix, message);

  FILE *log;
  char log_full_name[128];
  sprintf(log_full_name, "%s/%s", LOG_INITIAL_PATH, log_def_name);
  log = fopen(log_full_name, "a");
  if(log != NULL)
  {
    fprintf(log, "%s %s %s\n", t, prefix, message);
    fclose(log);
  }
  else
    printf("[WARNING] Can not open log file, %s\n", log_full_name);
}
//==============================================================================
void report_make_dir()
{
  #ifdef __linux__
  mkdir(REPORT_INITIAL_PATH, 0777);
  #elif _WIN32
  CreateDirectory(REPORT_INITIAL_PATH, NULL);
  #endif
}
//==============================================================================
FILE *report_open(char *report_name)
{
  char report_full_name[256];
  sprintf(report_full_name, "%s/%s", REPORT_INITIAL_PATH, report_name);

  report_make_dir();

  FILE *result = fopen(report_full_name, "a");

  return result;
}
//==============================================================================
int report_add(FILE *file, char *message)
{
  int res = -1;
  if(file != NULL)
  {
//    res = fputs(message, file);
    res = fprintf(file, "%s\n", message);
    fflush(file);
  }
  return res;
}
//==============================================================================
void report_close(FILE *file)
{
  if(file != NULL)
  {
    fclose(file);
  }
}
//==============================================================================
