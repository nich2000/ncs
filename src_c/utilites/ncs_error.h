#ifndef ERROR_H
#define ERROR_H
//==============================================================================
#ifndef DEMS_DEVICE
#include <unistd.h>
#endif

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
typedef int (*on_error_msg_t) (void *sender, int level, int number, const char *message);
typedef int (*on_error_t)     (void *sender, error_t *error);
//==============================================================================
error_t make_error (int level, int number, const char *message);
//==============================================================================
int make_last_error(int level, int number, const char *message);
error_t *last_error();
//==============================================================================
#endif //ERROR_H
