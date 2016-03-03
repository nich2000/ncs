//==============================================================================
//==============================================================================
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "globals.h"
#include "ncs_log.h"
#include "test.h"
#include "utils.h"
#include "socket.h"
#include "webworker.h"
#include "wsworker.h"
#include "cmdworker.h"
//==============================================================================
// "-DCMAKE_BUILD_TYPE=Debug"
//==============================================================================
#define RESULT_UNKNOWN          -1
#define RESULT_NONE              0
#define RESULT_DONE              1
//==============================================================================
#define DEFAULT_CMD_SERVER_PORT  5700
#define DEFAULT_WS_SERVER_PORT   5800
#define DEFAULT_WEB_SERVER_PORT  5900
#define DEFAULT_SERVER_HOST      "127.0.0.1"
//==============================================================================
int   web_server_port = DEFAULT_WEB_SERVER_PORT;
int   ws_server_port  = DEFAULT_WS_SERVER_PORT;
int   cmd_server_port = DEFAULT_CMD_SERVER_PORT;
char *cmd_server_host = DEFAULT_SERVER_HOST;
//==============================================================================
int handle_command(char *command)
{
  char token [128];
  char param1[128];
  char param2[128];
  char param3[128];
  char param4[128];
  char param5[128];

  int res = sscanf(command, "%s %s %s %s %s %s", token, param1, param2, param3, param4, param5);

  if(res == 1)
  {
    if(strcmp(token, "exit") == 0)
    {
      return RESULT_NONE;
    }
    else if(strcmp(token, "all") == 0)
    {
      log_set_name("all_log.txt");

      cmd_server(SOCK_STATE_START, cmd_server_port);
      ws_server (SOCK_STATE_START, ws_server_port);
      web_server(SOCK_STATE_START, web_server_port);

      return RESULT_DONE;
    }
    else if(strcmp(token, "client") == 0)
    {
      log_set_name("client_log.txt");

      cmd_client(SOCK_STATE_START, cmd_server_port, cmd_server_host);

      return RESULT_DONE;
    }
    else if(strcmp(token, "server") == 0)
    {
      log_set_name("server_log.txt");

      cmd_server(SOCK_STATE_START, cmd_server_port);

      return RESULT_DONE;
    }
    else if(strcmp(token, "wsserver") == 0)
    {
      log_set_name("ws_server_log.txt");

      ws_server(SOCK_STATE_START, ws_server_port);

      return RESULT_DONE;
    }
    else if(strcmp(token, "webserver") == 0)
    {
      log_set_name("web_server_log.txt");

      web_server(SOCK_STATE_START, web_server_port);

      return RESULT_DONE;
    }
    else if(strcmp(token, "test") == 0)
    {
      return RESULT_DONE;
    }
    else if(strcmp(token, "clear") == 0)
    {
      clr_scr();
      return RESULT_DONE;
    }
    else if(strcmp(token, "typesinfo") == 0)
    {
      print_types_info();
      return RESULT_DONE;
    }
    else if(strcmp(token, "definesinfo") == 0)
    {
      print_defines_info();
      return RESULT_DONE;
    }
    else if(strcmp(token, "serverinfo") == 0)
    {
      cmd_server_status();
      return RESULT_DONE;
    }
    else if(strcmp(token, "webserverinfo") == 0)
    {
      web_server_status();
      return RESULT_DONE;
    }
    else if(strcmp(token, "wsserverinfo") == 0)
    {
      ws_server_status();
      return RESULT_DONE;
    }
    else if(strcmp(token, "clientinfo") == 0)
    {
      cmd_client_status();
      return RESULT_DONE;
    }
    return RESULT_UNKNOWN;
  }
  else if(res == 2)
  {
    if(strcmp(token, "sndtocl") == 0)
    {
      cmd_server_send(1, param1);
      return RESULT_DONE;
    }
    else if(strcmp(token, "sndtowscl") == 0)
    {
//      sock_server_send_to_ws(_ws_server, 1, param1);
      return RESULT_DONE;
    }
    else if(strcmp(token, "sndtosr") == 0)
    {
      cmd_client_send(1, param1);
      return RESULT_DONE;
    }

    return RESULT_UNKNOWN;
  }
  else if(res == 3)
  {
    if(strcmp(token, "sndtocl") == 0)
    {
//      sock_server_send_cmd(_server, 2, param1, param2);
      return RESULT_DONE;
    }
    else if(strcmp(token, "sndtosr") == 0)
    {
//      sock_client_send_cmd(_client, 2, param1, param2);
      return RESULT_DONE;
    }

    return RESULT_UNKNOWN;
  }
  else if(res == 4)
  {
    return RESULT_UNKNOWN;
  }
  else if(res == 5)
  {
    return RESULT_UNKNOWN;
  }
  else if(res == 6)
  {
    return RESULT_UNKNOWN;
  }
  else
    return RESULT_UNKNOWN;
}
//==============================================================================
int main(int argc, char *argv[])
{
  sock_init();

  if(argc > 1)
  {
    // 0     1  2 3     4 5
    // name -c -p 5700 -h 127.0.0.1
    if(argc > 3)
      if(strcmp(argv[2], "-p") == 0)
        cmd_server_port = atoi(argv[3]);

    if(argc > 5)
      if(strcmp(argv[4], "-h") == 0)
        cmd_server_host = argv[5];

    if(strcmp(argv[1], "-s") == 0)
    {
//      if(_ws_server != 0)
//        free(_ws_server);
//      _ws_server = (sock_server_t*)malloc(sizeof(sock_server_t));

//      if(_web_server != 0)
//        free(_web_server);
//      _web_server = (sock_server_t*)malloc(sizeof(sock_server_t));

//      if(_server != 0)
//        free(_server);
//      _server = (sock_server_t*)malloc(sizeof(sock_server_t));

//      sock_server(ws_server_port,  _ws_server,  SOCK_MODE_WS_SERVER);
//      sock_server(web_server_port, _web_server, SOCK_MODE_WEB_SERVER);
//      sock_server(server_port,     _server,     SOCK_MODE_SERVER);
    }
    else if(strcmp(argv[1], "-c") == 0)
    {
//      sock_client(server_port, server_ip, _client);
    }
  }
  else
  {
    log_add("Command mode", LOG_INFO);

    printf(">");
    char command[128];
    while(1)
    {
      gets(command);

      switch(handle_command(command))
      {
        case RESULT_NONE:
          return 0;
        case RESULT_UNKNOWN:
          printf("unknown: %s\n", command);
          break;
        case RESULT_DONE:
          printf("done: %s\n", command);
          break;
      }
    }
  };

  sock_deinit();

  return 0;
}
//==============================================================================
