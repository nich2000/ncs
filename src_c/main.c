//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: main.c
 */
//==============================================================================
#include "main.h"
//==============================================================================
// "-DCMAKE_BUILD_TYPE=Debug"
//==============================================================================
extern sock_port_t cmd_server_port;
extern sock_port_t ws_server_port;
extern sock_port_t web_server_port;
extern sock_host_t cmd_server_host;
//==============================================================================
int main(int argc, char *argv[])
{
  if(read_config() >= ERROR_WARNING)
    goto exit;

  log_add(LOG_INFO, "application started");
  log_add_fmt(LOG_INFO, "application version: %s", APPLICATION_VERSION);
  log_add(LOG_INFO, "-------------------");

  if(sock_init() >= ERROR_WARNING)
    goto exit;

  load_coords();
//  load_session();

  char command[256];
  if(argc > 1)
  {
    if((strcmp(argv[1], "-v") == 0) || ((strcmp(argv[1], "--version") == 0))
      goto exit;

    // 0     1  2 3     4 5
    // name -c -p 5700 -h 127.0.0.1
    if(argc > 3)
      if(strcmp(argv[2], "-p") == 0)
        cmd_server_port = atoi(argv[3]);

    if(strcmp(argv[1], "-s") == 0)
    {
      log_add(LOG_INFO, "server mode");

      sprintf(command, "server on %d", cmd_server_port);
      handle_command_str(NULL, command);

      sprintf(command, "webserver on %d", web_server_port);
      handle_command_str(NULL, command);

      sprintf(command, "wsserver on %d", ws_server_port);
      handle_command_str(NULL, command);
    }
    else if(strcmp(argv[1], "-c") == 0)
    {
      log_add(LOG_INFO, "client mode");

      if(argc > 5)
        if(strcmp(argv[4], "-h") == 0)
          strcpy((char*)cmd_server_host, argv[5]);

      sprintf(command, "client on %d %s 1", cmd_server_port, cmd_server_host);
      handle_command_str(NULL, command);
    }
    else
    {
      make_last_error(ERROR_NONE, errno, "unknown mode");
      goto exit;
    }
  }
  else
  {
    log_add(LOG_INFO, "command mode");
    while(1)
    {
      fgets(command, sizeof(command), stdin);
      switch(handle_command_str(NULL, command))
      {
        case EXEC_NONE:
        {
          make_last_error(ERROR_NONE, errno, "exit by user command");
          goto exit;
        }
        case EXEC_UNKNOWN:
          log_add_fmt(LOG_CMD, "unknown command: %s",
                      command);
          break;
        case EXEC_DONE:
          log_add_fmt(LOG_CMD, "done command: %s",
                      command);
          break;
      }
    }
  }

  exit:
  sock_deinit();
  log_add_fmt(LOG_INFO, "application finished, result: %s\n", last_error()->message);

  return 0;
}
//==============================================================================
