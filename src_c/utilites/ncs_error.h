#ifndef ERROR_H
#define ERROR_H
//==============================================================================
#include "defines.h"
//==============================================================================
#define ERROR_NONE       100
#define ERROR_WARNING    101
#define ERROR_NORMAL     102
#define ERROR_CRITICAL   103
#define ERROR_FATAL      104
//==============================================================================
typedef struct
{
  int  level;
  int  number;
  char message[2048];
}error_t;
//==============================================================================
error_t make_error (int level, int number, char *message);
//==============================================================================
int make_last_error(int level, int number, char *message);
error_t *last_error();
//==============================================================================
#endif //ERROR_H
