#ifndef EXEC_H
#define EXEC_H
//==============================================================================
#include "defines.h"
//==============================================================================
#define EXEC_UNKNOWN            -1
#define EXEC_NONE                0
#define EXEC_DONE                1
//==============================================================================
#define DEFAULT_CMD_SERVER_PORT  5700
#define DEFAULT_WS_SERVER_PORT   5800
#define DEFAULT_WEB_SERVER_PORT  5900
#define DEFAULT_SERVER_HOST      "127.0.0.1"
//==============================================================================
typedef int (*exec_func)(int, ...);
//==============================================================================
int handle_command(char *command);
//==============================================================================
#endif //EXEC_H
