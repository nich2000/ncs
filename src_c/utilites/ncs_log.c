//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#ifdef __linux__
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#elif _WIN32
#include <windows.h>
#endif

#include <stdio.h>
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
void get_cur_date_str(char *result)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  time_t rawtime = tv.tv_sec;
  struct tm * timeinfo = localtime(&rawtime);

  strftime(result, sizeof(result), "%y%m%d", timeinfo);
}
//==============================================================================
void get_cur_time_str(char *result)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  time_t rawtime = tv.tv_sec;
  struct tm * timeinfo = localtime(&rawtime);

  char tmp[16];
  strftime(tmp, 16, "%H:%M:%S", timeinfo);
  snprintf(result, 16, "%s.%03li", tmp, tv.tv_usec/1000);
}
//==============================================================================
void log_gen_name(log_time_tag_t time_tag, const char *name, char *result)
{
  switch(time_tag)
  {
    case LOG_NAME_DATE:
    break;
    case LOG_NAME_TIME:
    break;
    case LOG_NAME_DATE_TIME:
    break;
    case LOG_NAME_ONLY:
    default:
    break;
  }

  char date_str[16];
  get_cur_date_str(date_str);

  sprintf(result, "%s_%s", date_str, name);
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
void log_set_name(const char *name)
{
  strncpy(log_def_name, name, 64);
}
//==============================================================================
void log_add_fmt(int log_type, const char *message, ...)
{
  char tmp[102400];

  va_list params;
  va_start(params, message);
  vsprintf(tmp, message, params);
  va_end(params);

  log_add(log_type, tmp);
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
void log_add(int log_type, const char *message)
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

  const char *log_type_str = log_type_to_string(log_type);

  if(
     (log_type == LOG_INFO)
     ||
     (log_type == LOG_CMD)
     #ifndef SILENT_MODE
     || (log_type >= LOG_ERROR_CRITICAL)
     #endif
    )
    printf("%s %s\n", log_type_str, message);

  char log_full_name[256];
  log_gen_name(LOG_NAME_DATE, log_def_name, log_full_name);
  sprintf(log_full_name, "%s/%s",
          LOG_INITIAL_PATH, log_full_name);

  FILE *log = fopen(log_full_name, "a");
  if(log != NULL)
  {
    char time_str[16];
    get_cur_time_str(time_str);

    fprintf(log, "%s %s %s\n", time_str, log_type_str, message);

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
  log_gen_name(LOG_NAME_DATE_TIME, report_name, report_full_name);
  sprintf(report_full_name, "%s/%s",
          REPORT_INITIAL_PATH, report_full_name);

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
