//==============================================================================
//==============================================================================
#include <string.h>

#include "log.h"
//==============================================================================
char log_name[64]    = "log.txt";
char report_name[64] = "report.txt";
//==============================================================================
void set_log_name(char *name)
{
  strncpy(log_name, name, 64);
}
//==============================================================================
void set_report_name(char *name)
{
  strncpy(report_name, name, 64);
}
//==============================================================================
void add_to_log(char *message, int logType)
{
  // time_t rawtime;
  // struct tm *timeinfo;
  // time (&rawtime);
  // timeinfo = localtime(&rawtime);

  char *t = "";
  // strftime(t, 128, "%T", timeinfo);

  char *prefix = "";
  switch(logType)
  {
    case LOG_ERROR: prefix = "ERROR";
    break;
    case LOG_WARNING: prefix = "WARNING";
    break;
    case LOG_DEBUG: prefix = "DEBUG";
    break;
    case LOG_INFO: prefix = "INFO";
    break;
  };

  printf("%s [%s] %s\n", t, prefix, message);

  FILE *log;
  log = fopen(log_name, "a");
  fprintf(log, "%s [%s] %s\n", t, prefix, message);
  fclose(log);
}
//==============================================================================
void add_to_report(char *message)
{
  FILE *log;
  log = fopen(report_name, "a");
  fprintf(log, "%s\n", message);
  fclose(log);
}
//==============================================================================
