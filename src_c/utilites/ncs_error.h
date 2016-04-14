//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#ifndef ERROR_H
#define ERROR_H
//==============================================================================
#include "errno.h"
#include "defines.h"
#include "globals.h"
//==============================================================================
#define ERROR_NONE       100
#define ERROR_WAIT       101
#define ERROR_WARNING    102
#define ERROR_NORMAL     103
#define ERROR_CRITICAL   104
#define ERROR_FATAL      105
//==============================================================================
typedef struct
{
  int  level;
  int  number;
  char message[2048];
}error_t;
//==============================================================================
typedef int (*on_error_msg_t) (void *sender, int level, int number, const char *message);
typedef int (*on_error_t)     (void *sender, error_t *error);
//==============================================================================
error_t make_error (int level, int number, const char *message);
//==============================================================================
int make_last_error(int level, int number, const char *message);
int make_last_error_fmt(int level, int number, const char *message, ...);
//==============================================================================
error_t *last_error();
//==============================================================================
#endif //ERROR_H
