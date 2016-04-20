//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: ncs_error.c
 */
//==============================================================================
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ncs_error.h"
#include "ncs_log.h"
//==============================================================================
static ncs_error_t _error_;
//==============================================================================
const char *error_type_to_string(int error_type)
{
  switch(error_type)
  {
    case ERROR_NONE:     return "[ERROR_NONE]";
    break;
    case ERROR_WAIT:     return "[ERROR_WAIT]";
    break;
    case ERROR_WARNING:  return "[ERROR_WARNING]";
    break;
    case ERROR_NORMAL:   return "[ERROR_NORMAL]";
    break;
    case ERROR_CRITICAL: return "[ERROR_CRITICAL]";
    break;
    case ERROR_FATAL:    return "[ERROR_FATAL]";
    break;
    default:             return "[ERROR_UNKNOWN]";
    break;
  }
}
//==============================================================================
ncs_error_t make_error(int level, int number, const char *message)
{
  ncs_error_t tmp_error;

  tmp_error.level = level;
  tmp_error.number = number;
  strcpy(tmp_error.message, message);

  return tmp_error;
}
//==============================================================================
int make_last_error(int level, int number, const char *message)
{
  _error_.level = level;
  _error_.number = number;
  strcpy(_error_.message, message);

  return level;
}
//==============================================================================
int make_last_error_fmt(int level, int number, const char *message, ...)
{
  char tmp[1024];

  va_list params;
  va_start(params, message);
  vsprintf(tmp, message, params);
  va_end(params);

  _error_.level = level;
  _error_.number = number;
  strcpy(_error_.message, tmp);

  return level;
}
//==============================================================================
void print_last_error(const char *sender, int result)
{
  if(result != ERROR_NONE)
    log_add_fmt(result, "%s, message: %s", sender, last_error()->message);
}
//==============================================================================
ncs_error_t *last_error()
{
  return &_error_;
}
//==============================================================================
