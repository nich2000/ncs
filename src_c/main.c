//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#include "main.h"
//==============================================================================
// "-DCMAKE_BUILD_TYPE=Debug"
//==============================================================================
extern sock_port_t web_server_port;
extern sock_port_t ws_server_port;
extern sock_port_t cmd_server_port;
extern sock_host_t cmd_server_host;
extern char log_path[256];
extern char stat_path[256];
extern char report_path[265];
extern char map_path[256];
extern char map_file[64];
extern char session_path[256];
extern char session_file[64];
//==============================================================================
int read_config()
{
  dictionary *config = iniparser_load("../config/config.ini");
  if(config == NULL)
    return make_last_error(ERROR_CRITICAL, errno, "read_config, iniparser_load");

  cmd_server_port = iniparser_getint(config, "worker:cmd_server_port", DEFAULT_CMD_SERVER_PORT);
  ws_server_port  = iniparser_getint(config, "worker:ws_server_port",  DEFAULT_WS_SERVER_PORT);
  web_server_port = iniparser_getint(config, "worker:web_server_port", DEFAULT_WEB_SERVER_PORT);

  strcpy((char*)cmd_server_host, iniparser_getstring(config, "worker:cmd_server_host", DEFAULT_SERVER_HOST));

  strcpy((char*)log_path,        iniparser_getstring(config, "log:log_path",           DEFAULT_LOG_PATH));

  strcpy((char*)stat_path,       iniparser_getstring(config, "stat:stat_path",         DEFAULT_STAT_PATH));
  strcpy((char*)report_path,     iniparser_getstring(config, "report:report_path",     DEFAULT_REPORT_PATH));

  strcpy((char*)session_path,    iniparser_getstring(config, "session:session_path",   DEFAULT_SESSION_PATH));
  strcpy((char*)session_file,    iniparser_getstring(config, "session:session_file",   DEFAULT_SESSION_NAME));

  strcpy((char*)map_path,        iniparser_getstring(config, "map:map_path",           DEFAULT_MAP_PATH));
  strcpy((char*)map_file,        iniparser_getstring(config, "map:map_file",           DEFAULT_MAP_NAME));

  iniparser_freedict(config);

  return ERROR_NONE;
}
//==============================================================================
int main(int argc, char *argv[])
{
  if(read_config() != ERROR_NONE)
    goto exit;

  log_add(LOG_INFO, "-------------------");
  log_add(LOG_INFO, "application started");
  log_add(LOG_INFO, "-------------------");

  if(sock_init() != ERROR_NONE)
    goto exit;

  char command[256];
  if(argc > 1)
  {
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
      log_add(LOG_INFO, "unknown mode");
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
          goto exit;
        case EXEC_UNKNOWN:
          log_add_fmt(LOG_CMD, "unknown command: %s", command);
          break;
        case EXEC_DONE:
          log_add_fmt(LOG_CMD, "done command: %s", command);
          break;
      }
    }
  }

  exit:
  sock_deinit();
  log_add(LOG_INFO, "application finished\n");

  return 0;
}
//==============================================================================
