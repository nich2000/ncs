//==============================================================================
//==============================================================================
#if defined(__linux__) || defined(_WIN32)
#include <unistd.h>
#endif

#ifdef __linux__
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <string.h>

#include "log.h"
//==============================================================================
#if defined(__linux__) || defined(_WIN32)
  time_t rawtime;
  struct tm *timeinfo;
#endif
//==============================================================================
char log_name[64]    = "log.txt";
char report_name[64] = "report.txt";
//==============================================================================
int clr_scr()
{
  #ifdef __linux__
    system("clear");
  #elif _WIN32
    system("cls");
  #endif

  return 0;
}
//==============================================================================
int make_log_dir()
{
  #ifdef __linux__
  mkdir(LOG_INITIAL_PATH, 0777);
  #elif _WIN32
  CreateDirectory(LOG_INITIAL_PATH, NULL);
  #endif

  return 0;
}
//==============================================================================
void log_set_name(char *name)
{
  strncpy(log_name, name, 64);
}
//==============================================================================
void report_set_name(char *name)
{
  strncpy(report_name, name, 64);
}
//==============================================================================
void log_add(char *message, int log_type)
{
#if defined(__linux__) || defined(_WIN32)

  #ifndef DEBUG_MODE
  if(log_type == LOG_DEBUG)
    return;
  #endif

  make_log_dir();

//   time(&rawtime);
//   timeinfo = localtime(&rawtime);

  char *t = "";
//  strftime(t, 128, "%T", timeinfo);

  char *prefix = "";
  switch(log_type)
  {
    case LOG_CRITICAL_ERROR: prefix = "[CRITICAL_ERROR]";
    break;
    case LOG_ERROR:          prefix = "[ERROR]";
    break;
    case LOG_WARNING:        prefix = "[WARNING]";
    break;
    case LOG_DEBUG:          prefix = "[DEBUG]";
    break;
    case LOG_INFO:           prefix = "[INFO]";
    break;
    case LOG_DATA:           prefix = "[DATA]";
    break;
    case LOG_RAW_DATA:       prefix = "[RAW_DATA]";
    break;
  };

  if(
     (log_type == LOG_INFO)
     #ifndef SILENT_MODE
     || (log_type == LOG_CRITICAL_ERROR)
     #endif
    )
    printf("%s %s\n", prefix, message);

  FILE *log;
  char log_full_name[128];
  sprintf(log_full_name, "%s/%s", LOG_INITIAL_PATH, log_name);
  log = fopen(log_full_name, "a");
  if(log != NULL)
  {
    fprintf(log, "%s %s %s\n", t, prefix, message);
    fclose(log);
  }
  else
    printf("[WARNING] Can not open log file, %s\n", log_full_name);
#endif
}
//==============================================================================
void report_add(char *message)
{
#if defined(__linux__) || defined(_WIN32)
  FILE *report;
  char report_full_name[128];
  sprintf(report_full_name, "%s/%s", REPORT_INITIAL_PATH, report_name);
  report = fopen(report_full_name, "a");
  fprintf(report, "%s\n", message);
  fclose(report);
#endif
}
//==============================================================================
void skip_lines(size_t count)
{
  for(size_t i = 0; i < count; i++)
    printf("\n");
}
//==============================================================================
