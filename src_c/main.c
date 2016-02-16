#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "socket.h"
#include "test.h"

#include "protocol.h"
#include "gps_parse.h"
//==============================================================================
// "-DCMAKE_BUILD_TYPE=Debug"
//==============================================================================
int port   = 5700;
char *host = "127.0.0.1";
//==============================================================================
//extern "C" __declspec(dllexport) void __cdecl CreateServer(int ID, char* Port, char* RootLocation, HWND SendMsgHandler, bool UseSSL);
//extern "C" __declspec(dllexport) void __cdecl StopServer(int ID);
//==============================================================================
typedef void (*_func)(int, char*, char*, int, short);
//==============================================================================
int web_server()
{
  HMODULE handle = LoadLibrary("libWebServer.dll");
  if (handle != 0)
  {
    _func CreateServer = GetProcAddress(handle, "CreateServer");
    CreateServer(0, "8080", "../www/", 0, 0);
  }
}
//==============================================================================
void print_types_info()
{
  char tmp[256];

  sprintf(
    tmp,
    "\r\n" \
    "pack_word: %d, \r\n" \
    "pack_words: %d, \r\n" \
    "pack_packet: %d, \r\n" \
    "pack_out_packets: %d, \r\n" \
    "pack_in_packets: %d, \r\n" \
    "pack_queue_packets: %d, \r\n" \
    "pack_validation_buffer: %d, \r\n" \
    "pack_out_packets_list: %d, \r\n" \
    "pack_in_packets_list: %d, \r\n" \
    "pack_protocol: %d, \r\n" \
    "pack_queue: %d",
    sizeof(pack_word),
    sizeof(pack_words),
    sizeof(pack_packet),
    sizeof(pack_out_packets),
    sizeof(pack_in_packets),
    sizeof(pack_queue_packets),
    sizeof(pack_validation_buffer),
    sizeof(pack_out_packets_list),
    sizeof(pack_in_packets_list),
    sizeof(pack_protocol),
    sizeof(pack_queue)
  );
  log_add(tmp, LOG_INFO);

  sprintf(
    tmp,
    "\r\n" \
    "sock_worker: %d, \r\n" \
    "sock_worker_list: %d",
    sizeof(sock_worker),
    sizeof(sock_worker_list)
  );
  log_add(tmp, LOG_INFO);
}
//==============================================================================
int handle_command(char *command)
{
//  printf("echo: %s\n", command);

  char token[128];
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
      sock_exit();
      return 0;
    }
    else if(strcmp(token, "webserver") == 0)
    {
      web_server();
    }
    else if(strcmp(token, "server") == 0)
    {
      sock_server(port);
      return 1;
    }
    else if(strcmp(token, "client") == 0)
    {
      sock_client(port, host);
      return 1;
    }
    else if(strcmp(token, "test") == 0)
    {
      return 1;
    }
    else if(strcmp(token, "clear") == 0)
    {
      clrscr();
      return 1;
    }
    else if(strcmp(token, "typeinfo") == 0)
    {
      print_types_info();
      return 1;
    }
  }
  else if(res == 2)
  {
    if(strcmp(token, "send") == 0)
    {
      sock_send_cmd(1, param1);
    }
    return 1;
  }
  else if(res == 3)
  {
    if(strcmp(token, "send") == 0)
    {
      sock_send_cmd(2, param1, param2);
    }
    return 1;
  }
  else if(res == 4)
  {
    if(strcmp(token, "send") == 0)
    {
      sock_send_cmd(3, param1, param2, param3);
    }
    return 1;
  }
  else if(res == 5)
  {
    if(strcmp(token, "send") == 0)
    {
      sock_send_cmd(4, param1, param2, param3, param4);
    }
    return 1;
  }
  else if(res == 6)
  {
    if(strcmp(token, "send") == 0)
    {
      sock_send_cmd(5, param1, param2, param3, param4, param5);
    }
    return 1;
  }
  else
    return -1;
}
//==============================================================================
int main(int argc, char *argv[])
{
  if(argc > 1)
  {
    // 0     1  2 3     4 5
    // name -c -p 5600 -h 127.0.0.1
    if(argc > 3)
      if(strcmp(argv[2], "-p") == 0)
        port = atoi(argv[3]);

    if(argc > 5)
      if(strcmp(argv[4], "-h") == 0)
        host = argv[5];

    if(strcmp(argv[1], "-s") == 0)
    {
      sock_server(port);
    }
    else if(strcmp(argv[1], "-c") == 0)
    {
      sock_client(port, host);
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
        case 0:
          return 0;
          break;
        case -1:
          printf("unknown command: %s\n", command);
          break;
      }

//      printf(">");
    }
  };

  return 0;
}
//==============================================================================
