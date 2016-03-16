#ifndef EXEC_H
#define EXEC_H
//==============================================================================
#include "defines.h"
#include "globals.h"
#include "ncs_pack.h"
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
#define MAX_COMMAND_SIZE         128
//==============================================================================
typedef int (*exec_func)(int, ...);
//==============================================================================
int handle_command_str(char *command);
int handle_command_pack(pack_packet_t *packet);
//==============================================================================
#endif //EXEC_H
