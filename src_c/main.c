#include <stdlib.h>
#include <string.h>

#include "streamer.h"
#include "log.h"
#include "socket.h"
#include "test.h"
#include "socket_utils.h"
#include "utils.h"
#include "protocol.h"
#include "gps_parse.h"
//==============================================================================
// "-DCMAKE_BUILD_TYPE=Debug"
//==============================================================================
#define RESULT_UNKNOWN -1
#define RESULT_NONE     0
#define RESULT_DONE     1
//==============================================================================
#define DEFAULT_SERVER_PORT      5700
#define DEFAULT_WS_SERVER_PORT   5800
#define DEFAULT_WEB_SERVER_PORT  5900
#define DEFAULT_SERVER_IP        "127.0.0.1"
//==============================================================================
int   web_server_port      = DEFAULT_WEB_SERVER_PORT;
int   ws_server_port       = DEFAULT_WS_SERVER_PORT;
int   server_port          = DEFAULT_SERVER_PORT;
char *server_ip            = DEFAULT_SERVER_IP;
//==============================================================================
sock_server_t *_web_server = NULL;
sock_server_t *_ws_server  = NULL;
sock_server_t *_server     = NULL;
sock_worker_t *_client     = NULL;
//==============================================================================
typedef void (*_CreateServer)(int, char*, char*, int, short);
typedef void (*_StopServer)(int);
//==============================================================================
int web_server(int create)
{
  HMODULE handle = LoadLibrary("libWebServer.dll");
  if (handle != 0)
  {
    if(create)
    {
      _CreateServer CreateServer = (_CreateServer)GetProcAddress(handle, "CreateServer");
      CreateServer(0, "8080", "../www/", 0, 0);
    }
    {
      _StopServer StopServer = (_StopServer)GetProcAddress(handle, "StopServer");
      StopServer(0);
    }
  }
}
//==============================================================================
int handle_command(char *command)
{
//  printf("echo: %s\n", command);

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
//      sock_exit(_worker);
      return RESULT_NONE;
    }
    else if(strcmp(token, "server") == 0)
    {
      log_set_name("server_log.txt");

      if(_server != 0)
        free(_server);
      _server = (sock_server_t*)malloc(sizeof(sock_server_t));

//      char tmp[128];
//      sprintf(tmp, "handle_command, server.addr: %u", _server);
//      log_add(tmp, LOG_INFO);

      sock_server(server_port, _server, SOCK_MODE_SERVER);

      return RESULT_DONE;
    }
    else if(strcmp(token, "wsserver") == 0)
    {
      log_set_name("ws_server_log.txt");

      if(_ws_server != 0)
        free(_ws_server);
      _ws_server = (sock_server_t*)malloc(sizeof(sock_server_t));

      sock_server(ws_server_port, _ws_server, SOCK_MODE_WS_SERVER);

      return RESULT_DONE;
    }
    else if(strcmp(token, "webserver") == 0)
    {
      log_set_name("web_server_log.txt");

//      if(_ws_server != 0)
//        free(_ws_server);
//      _ws_server = (sock_server_t*)malloc(sizeof(sock_server_t));
//      sock_server(ws_server_port, _ws_server, SOCK_MODE_WS_SERVER);

      if(_web_server != 0)
        free(_web_server);
      _web_server = (sock_server_t*)malloc(sizeof(sock_server_t));
      sock_server(web_server_port, _web_server, SOCK_MODE_WEB_SERVER);

      return RESULT_DONE;
    }
    else if(strcmp(token, "client") == 0)
    {
      log_set_name("client_log.txt");

      if(_client != 0)
        free(_client);
      _client = (sock_worker_t*)malloc(sizeof(sock_worker_t));

      sock_client(server_port, server_ip, _client);

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
      print_server_info(_server);
      return RESULT_DONE;
    }
    else if(strcmp(token, "webserverinfo") == 0)
    {
      print_server_info(_web_server);
      return RESULT_DONE;
    }
    else if(strcmp(token, "wsserverinfo") == 0)
    {
      print_server_info(_ws_server);
      return RESULT_DONE;
    }
    else if(strcmp(token, "clientinfo") == 0)
    {
      print_worker_info(_client, "client");
      return RESULT_DONE;
    }
    return RESULT_UNKNOWN;
  }
  else if(res == 2)
  {
    if(strcmp(token, "webserver") == 0)
    {
      if(strcmp(param1, "on") == 0)
        web_server(1);
      if(strcmp(param1, "off") == 0)
        web_server(0);
      return RESULT_DONE;
    }
    else if(strcmp(token, "sndtocl") == 0)
    {
      sock_server_send_cmd(_server, 1, param1);
      return RESULT_DONE;
    }
    else if(strcmp(token, "sndtowscl") == 0)
    {
      sock_server_send_to_ws(_ws_server, 1, param1);
      return RESULT_DONE;
    }
    else if(strcmp(token, "sndtosr") == 0)
    {
      sock_client_send_cmd(_client, 1, param1);
      return RESULT_DONE;
    }

    return RESULT_UNKNOWN;
  }
  else if(res == 3)
  {
    if(strcmp(token, "sndtocl") == 0)
    {
      sock_server_send_cmd(_server, 2, param1, param2);
      return RESULT_DONE;
    }
    else if(strcmp(token, "sndtosr") == 0)
    {
      sock_client_send_cmd(_client, 2, param1, param2);
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
  if(argc > 1)
  {
    // 0     1  2 3     4 5
    // name -c -p 5700 -h 127.0.0.1
    if(argc > 3)
      if(strcmp(argv[2], "-p") == 0)
        server_port = atoi(argv[3]);

    if(argc > 5)
      if(strcmp(argv[4], "-h") == 0)
        server_ip = argv[5];

    if(strcmp(argv[1], "-s") == 0)
    {
      if(_ws_server != 0)
        free(_ws_server);
      _ws_server = (sock_server_t*)malloc(sizeof(sock_server_t));

      if(_web_server != 0)
        free(_web_server);
      _web_server = (sock_server_t*)malloc(sizeof(sock_server_t));

      if(_server != 0)
        free(_server);
      _server = (sock_server_t*)malloc(sizeof(sock_server_t));

      sock_server(ws_server_port,  _ws_server,  SOCK_MODE_WS_SERVER);
      sock_server(web_server_port, _web_server, SOCK_MODE_WEB_SERVER);
      sock_server(server_port,     _server,     SOCK_MODE_SERVER);
    }
    else if(strcmp(argv[1], "-c") == 0)
    {
      sock_client(server_port, server_ip, _client);
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
          printf("unknown command: %s\n", command);
          break;
        case RESULT_DONE:
          printf("done command: %s\n", command);
          break;
      }
    }
  };
  return 0;
}
//==============================================================================
