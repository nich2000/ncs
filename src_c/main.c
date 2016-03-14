//==============================================================================
//==============================================================================
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "globals.h"
#include "socket.h"
#include "ncs_log.h"
#include "ncs_error.h"
#include "exec.h"
//==============================================================================
// "-DCMAKE_BUILD_TYPE=Debug"
//==============================================================================
sock_port_t web_server_port = DEFAULT_WEB_SERVER_PORT;
sock_port_t ws_server_port  = DEFAULT_WS_SERVER_PORT;
sock_port_t cmd_server_port = DEFAULT_CMD_SERVER_PORT;
sock_host_t cmd_server_host = DEFAULT_SERVER_HOST;
//==============================================================================
int main(int argc, char *argv[])
{
  char command[256];

  sock_init();

  if(argc > 1)
  {
    // 0     1  2 3     4 5
    // name -c -p 5700 -h 127.0.0.1
    if(argc > 3)
      if(strcmp(argv[2], "-p") == 0)
        cmd_server_port = atoi(argv[3]);

    if(strcmp(argv[1], "-s") == 0)
    {
      log_add("server mode", LOG_INFO);

      sprintf(command, "server on %d", cmd_server_port);
      handle_command(command);

      sprintf(command, "webserver on %d", web_server_port);
      handle_command(command);

      sprintf(command, "wsserver on %d", ws_server_port);
      handle_command(command);
    }
    else if(strcmp(argv[1], "-c") == 0)
    {
      log_add("client mode", LOG_INFO);

      if(argc > 5)
        if(strcmp(argv[4], "-h") == 0)
          strcpy((char*)cmd_server_host, argv[5]);

      sprintf(command, "client on %d %s 1", cmd_server_port, cmd_server_host);
      handle_command(command);
    }
    else
    {
      log_add_fmt(LOG_INFO, "unknown mode", LOG_INFO);
      return 1;
    }
  }
  else
  {
    log_add("command mode", LOG_INFO);
    printf(">");
    while(1)
    {
      fgets(command, sizeof(command), stdin);
      switch(handle_command(command))
      {
        case EXEC_NONE:
          return 0;
        case EXEC_UNKNOWN:
          printf("unknown: %s\n", command);
          break;
        case EXEC_DONE:
          printf("done: %s\n", command);
          break;
      }
    }
  }

  sock_deinit();

  return 0;
}
//==============================================================================
