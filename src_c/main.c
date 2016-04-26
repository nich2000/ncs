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
extern int         cmd_clients_count;
//==============================================================================
int main(int argc, char *argv[])
{
  //---------------------------------------------------------------------------
  if(argc > 1)
  {
    if((strcmp(argv[1], CMD_S_VERSION) == 0) || ((strcmp(argv[1], CMD_VERSION) == 0)))
    {
      print_version();
      return 0;
    }
    if((strcmp(argv[1], CMD_S_HELP) == 0) || ((strcmp(argv[1], CMD_HELP) == 0)))
    {
      print_help(1);
      return 0;
    }
    if((strcmp(argv[1], CMD_S_CONFIG) == 0) || ((strcmp(argv[1], CMD_CONFIG) == 0)))
    {
      read_config();
      print_config();
      return 0;
    }
  }
  //---------------------------------------------------------------------------
  print_version();
  //---------------------------------------------------------------------------
  if(read_config() >= ERROR_WARNING)
    goto exit;
  print_config();
  //---------------------------------------------------------------------------
  log_add(LOG_INFO, "application started");
  log_add_fmt(LOG_INFO, "application version: %s", APPLICATION_VERSION);
  log_add(LOG_INFO, "-------------------");
  //---------------------------------------------------------------------------
  if(sock_init() >= ERROR_WARNING)
    goto exit;
  //---------------------------------------------------------------------------
  load_coords();
//  load_session();
  //---------------------------------------------------------------------------
  char command[256];
  if(argc > 1)
  {
    // Read params
    for(int i = 1; i < argc; i++)
    {
      if((strcmp(argv[i], PARAM_S_PORT) == 0) || ((strcmp(argv[i], PARAM_PORT) == 0)))
      {
        cmd_server_port = atoi(argv[++i]);
      }
      else if((strcmp(argv[i], PARAM_S_WEB_PORT) == 0) || ((strcmp(argv[i], PARAM_WEB_PORT) == 0)))
      {
        web_server_port = atoi(argv[++i]);
      }
      else if((strcmp(argv[i], PARAM_S_WS_PORT) == 0) || ((strcmp(argv[i], PARAM_WS_PORT) == 0)))
      {
        ws_server_port = atoi(argv[++i]);
      }
      else if((strcmp(argv[i], PARAM_S_HOST) == 0) || ((strcmp(argv[i], PARAM_HOST) == 0)))
      {
        strcpy((char*)cmd_server_host, argv[++i]);
      }
      else if((strcmp(argv[i], PARAM_S_COUNT) == 0) || ((strcmp(argv[i], PARAM_COUNT) == 0)))
      {
        cmd_clients_count = atoi(argv[++i]);
      }
    }
    // Read cmd
    for(int i = 1; i < argc; i++)
    {
      if((strcmp(argv[i], CMD_S_ALL) == 0) || ((strcmp(argv[i], CMD_ALL) == 0)))
      {
        strcpy(command, CMD_ALL);
      }
      else if((strcmp(argv[i], CMD_S_SERVER) == 0) || ((strcmp(argv[i], CMD_SERVER) == 0)))
      {
        strcpy(command, CMD_SERVER);
      }
      else if((strcmp(argv[i], CMD_S_WEB_SERVER) == 0) || ((strcmp(argv[i], CMD_WEB_SERVER) == 0)))
      {
        strcpy(command, CMD_WEB_SERVER);
      }
      else if((strcmp(argv[i], CMD_S_WS_SERVER) == 0) || ((strcmp(argv[i], CMD_WS_SERVER) == 0)))
      {
        strcpy(command, CMD_WS_SERVER);
      }
      else if((strcmp(argv[i], CMD_S_CLIENT) == 0) || ((strcmp(argv[i], CMD_CLIENT) == 0)))
      {
        strcpy(command, CMD_CLIENT);
      }
    }

    handle_command_str(NULL, command);
  }
  else
  {
    log_add(LOG_INFO, "command mode");
  }
  //---------------------------------------------------------------------------
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
  //---------------------------------------------------------------------------
  exit:
  sock_deinit();
  log_add_fmt(LOG_INFO, "application finished, result: %s\n", last_error()->message);
  //---------------------------------------------------------------------------
  return 0;
  //---------------------------------------------------------------------------
}
//==============================================================================
