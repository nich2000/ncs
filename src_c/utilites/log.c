//==============================================================================
//==============================================================================
#if defined(__linux__) || defined(_WIN32)
#include <unistd.h>
#endif

#include <string.h>

#include "log.h"
//==============================================================================
char log_name[64]    = "log.txt";
char report_name[64] = "report.txt";
//==============================================================================
int clrscr()
{
#ifdef __linux__
  system("clear");
#elif _WIN32
  system("cls");
#else
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
void log_add(char *message, int logType)
{
#if defined(__linux__) || defined(_WIN32)
  // time_t rawtime;
  // struct tm *timeinfo;
  // time (&rawtime);
  // timeinfo = localtime(&rawtime);

  char *t = "";
  // strftime(t, 128, "%T", timeinfo);

  char *prefix = "";
  switch(logType)
  {
    case LOG_ERROR: prefix = "[ERROR]";
    break;
    case LOG_WARNING: prefix = "[WARNING]";
    break;
    case LOG_DEBUG: prefix = "[DEBUG]";
    break;
    case LOG_INFO: prefix = "[INFO]";
    break;
    case LOG_DATA:
    break;
  };

  printf("%s %s %s\n", t, prefix, message);

  FILE *log;
  log = fopen(log_name, "a");
  fprintf(log, "%s %s %s\n", t, prefix, message);
  fclose(log);
#else
#endif
}
//==============================================================================
void report_add(char *message)
{
#if defined(__linux__) || defined(_WIN32)
  FILE *log;
  log = fopen(report_name, "a");
  fprintf(log, "%s\n", message);
  fclose(log);
#else
#endif
}
//==============================================================================
void skip_lines(size_t count)
{
  for(size_t i = 0; i < count; i++)
    printf("\n");
}
//==============================================================================
