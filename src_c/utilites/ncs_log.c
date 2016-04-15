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
#include <sys/time.h>
#include <windows.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "ncs_log.h"
//==============================================================================
void log_gen_name(log_time_tag_t time_tag, const char *name, char *result);
void make_dir(const char *dir);
void get_cur_date_str(char *result);
void get_cur_time_str(char *result);
const char *log_type_to_string(int log_type);
//==============================================================================
char log_path[256]    = DEFAULT_LOG_PATH;
char stat_path[256]   = DEFAULT_STAT_PATH;
char report_path[265] = DEFAULT_REPORT_PATH;
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
void make_dir(const char *dir)
{
  #ifdef __linux__
  mkdir(dir, 0777);
  #elif _WIN32
  CreateDirectory(dir, NULL);
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
    {
      char date_str[16];
      get_cur_date_str(date_str);
      sprintf(result, "%s_%s", date_str, name);
      break;
    }

    case LOG_NAME_TIME:
    {
      char time_str[16];
      get_cur_time_str(time_str);
      sprintf(result, "%s_%s", time_str, name);
      break;
    }

    case LOG_NAME_DATE_TIME:
    {
      char date_str[16];
      get_cur_date_str(date_str);
      char time_str[16];
      get_cur_time_str(time_str);
      sprintf(result, "%s_%s_%s", date_str, time_str, name);
      break;
    }

    case LOG_NAME_ONLY:
    default:
    {
      sprintf(result, "%s", name);
      break;
    }
  }
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

  char full_file_name[256];
  char file_name[64];
  log_gen_name(LOG_NAME_DATE, DEFAULT_LOG_NAME, file_name);
  sprintf(full_file_name, "%s/%s", log_path, file_name);

  make_dir(log_path);

  FILE *log = fopen(full_file_name, "a");
  if(log != NULL)
  {
    char time_str[16];
    get_cur_time_str(time_str);

    fprintf(log, "%s %s %s\n", time_str, log_type_str, message);

    fclose(log);
  }
  else
    printf("[WARNING] Can not open log file, %s\n", full_file_name);
}
//==============================================================================
FILE *stat_open(char *name)
{
  char full_file_name[256];
  log_gen_name(LOG_NAME_DATE_TIME, name, full_file_name);
  sprintf(full_file_name, "%s/%s",
          stat_path, full_file_name);

  make_dir(stat_path);

  FILE *result = fopen(full_file_name, "a");

  return result;
}
//==============================================================================
int stat_add(FILE *file, char *message)
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
void  stat_close(FILE *file)
{
  if(file != NULL)
    fclose(file);
}
//==============================================================================
FILE *report_open(char *name)
{
  char full_file_name[256];
  log_gen_name(LOG_NAME_DATE_TIME, name, full_file_name);
  sprintf(full_file_name, "%s/%s",
          report_path, full_file_name);

  make_dir(report_path);

  FILE *result = fopen(full_file_name, "a");

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
    fclose(file);
}
//==============================================================================
