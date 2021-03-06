//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: ncs_log.c
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
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>

#include "ncs_log.h"
//==============================================================================
void log_gen_name(log_time_tag_t time_tag, const char *name, char *result);
void make_dir(const char *dir);
void get_cur_date_str(char *result, int short_format);
void get_cur_time_str(char *result, int short_format);
const char *log_type_to_string(int log_type);
//==============================================================================
BOOL log_enable        = DEFAULT_LOG_ENABLE;
char log_path[256]     = DEFAULT_LOG_PATH;
BOOL stat_enable       = DEFAULT_STAT_ENABLE;
char stat_path[256]    = DEFAULT_STAT_PATH;
BOOL session_enable    = DEFAULT_SESSION_ENABLE;
char session_path[256] = DEFAULT_SESSION_PATH;
BOOL report_enable     = DEFAULT_REPORT_ENABLE;
char report_path[265]  = DEFAULT_REPORT_PATH;
char log_prefix[16]    = DEFAULT_LOG_NAME;
//==============================================================================
static pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;
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
void get_cur_date_str(char *result, int short_format)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  time_t rawtime = tv.tv_sec;
  struct tm * timeinfo = localtime(&rawtime);

  if(short_format)
  {
    strftime(result, 16, "%y%m%d", timeinfo);
  }
  else
  {
    strftime(result, 16, "%y.%m.%d", timeinfo);
  }
}
//==============================================================================
void get_cur_time_str(char *result, int short_format)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  time_t rawtime = tv.tv_sec;
  struct tm * timeinfo = localtime(&rawtime);

  if(short_format)
  {
    char tmp[16];
    strftime(tmp, 16, "%H%M%S", timeinfo);
    snprintf(result, 16, "%s%03li", tmp, tv.tv_usec/1000);
  }
  else
  {
    char tmp[16];
    strftime(tmp, 16, "%H:%M:%S", timeinfo);
    snprintf(result, 16, "%s.%03li", tmp, tv.tv_usec/1000);
  }
}
//==============================================================================
void log_gen_name(log_time_tag_t time_tag, const char *name, char *result)
{
  switch(time_tag)
  {
    case LOG_NAME_DATE:
    {
      char date_str[16];
      get_cur_date_str(date_str, LOG_LONG_FORMAT);
      sprintf(result, "%s_%s", date_str, name);
      break;
    }

    case LOG_NAME_DATE_S:
    {
      char date_str[16];
      get_cur_date_str(date_str, LOG_SHORT_FORMAT);
      sprintf(result, "%s_%s", date_str, name);
      break;
    }

    case LOG_NAME_TIME:
    {
      char time_str[16];
      get_cur_time_str(time_str, LOG_LONG_FORMAT);
      sprintf(result, "%s_%s", time_str, name);
      break;
    }

    case LOG_NAME_TIME_S:
    {
      char time_str[16];
      get_cur_time_str(time_str, LOG_SHORT_FORMAT);
      sprintf(result, "%s_%s", time_str, name);
      break;
    }

    case LOG_NAME_DATE_TIME:
    {
      char date_str[16];
      get_cur_date_str(date_str, LOG_LONG_FORMAT);
      char time_str[16];
      get_cur_time_str(time_str, LOG_LONG_FORMAT);
      sprintf(result, "%s_%s_%s", date_str, time_str, name);
      break;
    }

    case LOG_NAME_DATE_TIME_S:
    {
      char date_str[16];
      get_cur_date_str(date_str, LOG_SHORT_FORMAT);
      char time_str[16];
      get_cur_time_str(time_str, LOG_SHORT_FORMAT);
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
    case LOG_INFO:           return "[INFO] ";
    case LOG_WAIT:           return "[WAIT] ";
    case LOG_WARNING:        return "[WARNING] ";
    case LOG_ERROR:          return "[ERROR] ";
    case LOG_ERROR_CRITICAL: return "[ERROR_CRITICAL] ";
    case LOG_ERROR_FATAL:    return "[ERROR_FATAL] ";
    case LOG_CMD:            return "[COMMAND] ";
    case LOG_EXTRA:          return "[EXTRA] ";
    case LOG_DEBUG:          return "[DEBUG] ";
    case LOG_DATA:           return "[DATA] ";
    case LOG_NO_IDENT:
    case LOG_RAW_DATA:
    default:                 return "";
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
  #ifndef DEMS_DEVICE
  pthread_mutex_lock(&mutex_log);
  #endif

  #ifndef DEBUG_MODE
  if(log_type == LOG_DEBUG)
    goto exit;
  #endif

  #ifndef USE_EXTRA_LOGS
    if(log_type == LOG_EXTRA)
      goto exit;
  #endif

  #ifndef USE_WAIT_LOGS
    if(log_type == LOG_WAIT)
      goto exit;
  #endif

  const char *log_type_str = log_type_to_string(log_type);

  if(
     (log_type == LOG_INFO)
     ||
     (log_type == LOG_CMD)
     ||
     (log_type == LOG_NO_IDENT)
     #ifndef SILENT_MODE
     || (log_type >= LOG_ERROR_CRITICAL)
     #endif
    )
    printf("%s%s\n", log_type_str, message);

  char full_file_name[256];
  char file_name[64];
  log_gen_name(LOG_NAME_DATE_S, log_prefix, file_name);
  sprintf(full_file_name, "%s/%s", log_path, file_name);

  make_dir(log_path);

  if(log_enable)
  {
    FILE *log = fopen(full_file_name, "a");
    if(log != NULL)
    {
      char time_str[16];
      get_cur_time_str(time_str, LOG_LONG_FORMAT);

      fprintf(log, "%s %s%s\n", time_str, log_type_str, message);

      fclose(log);
    }
    else
      printf("[WARNING] Can not open log file, %s\n", full_file_name);
  }

  exit:
  #ifndef DEMS_DEVICE
  pthread_mutex_unlock(&mutex_log);
  #endif
  return;
}
//==============================================================================
FILE *stat_open(char *name)
{
  char tmp_file_name[256];
  log_gen_name(LOG_NAME_DATE_TIME_S, name, tmp_file_name);

  char full_file_name[256];
  sprintf(full_file_name, "%s/%s_%s",
          stat_path, tmp_file_name, DEFAULT_STAT_NAME);

  make_dir(stat_path);

  FILE *result = fopen(full_file_name, "a");

  return result;
}
//==============================================================================
int stat_add(FILE *file, char *message)
{
  int res = -1;
  if((stat_enable) && (file != NULL))
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
//==============================================================================
FILE *session_open(char *name)
{
  char tmp_file_name[256];
  log_gen_name(LOG_NAME_DATE_TIME_S, name, tmp_file_name);

  char full_file_name[256];
  sprintf(full_file_name, "%s/%s_%s",
          session_path, tmp_file_name, DEFAULT_SESSION_STREAM_NAME);

  make_dir(session_path);

  FILE *result = fopen(full_file_name, "a");

  return result;
}
//==============================================================================
int session_add(FILE *file, char *message)
{
  int res = -1;
  if((session_enable) && (file != NULL))
  {
    res = fprintf(file, "%s\n", message);
    fflush(file);
  }
  return res;
}
//==============================================================================
void  session_close(FILE *file)
{
  if(file != NULL)
    fclose(file);
}
//==============================================================================
//==============================================================================
FILE *report_open(char *name)
{
  char tmp_file_name[256];
  log_gen_name(LOG_NAME_DATE_TIME_S, name, tmp_file_name);

  char full_file_name[256];
  sprintf(full_file_name, "%s/%s_%s",
          report_path, tmp_file_name, DEFAULT_REPORT_NAME);

  make_dir(report_path);

  FILE *result = fopen(full_file_name, "a");

  return result;
}
//==============================================================================
int report_add(FILE *file, char *message)
{ 
  int res = -1;
  if((report_enable) && (file != NULL))
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
