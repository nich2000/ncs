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
extern char *log_path;
extern char *stat_path;
extern char *map_path;
extern char *map_file;
//==============================================================================

//==============================================================================
int read_config()
{
  dictionary *config = iniparser_load("../config/config.ini");
  if(config == NULL)
    return make_last_error(ERROR_NORMAL, errno, "read_config, iniparser_load");

  log_add(LOG_INFO, "read_config");

  int s_count = iniparser_getnsec(config);
  for(int i = 0; i < s_count; i++)
  {
    const char *s_name = iniparser_getsecname(config, i);
    log_add_fmt(LOG_INFO, "  section: %s", s_name);

    int k_count = iniparser_getsecnkeys(config, s_name);

    const char
    const char **k_names = iniparser_getseckeys(config, s_name);

    for(int j = 0; j < k_count; j++)
    {

      log_add_fmt(LOG_INFO, "  section: %s", s_name);
    }
  }

  iniparser_freedict(config);

  return ERROR_NONE;
}
//==============================================================================
int main(int argc, char *argv[])
{
  log_add(LOG_INFO, "-------------------");
  log_add(LOG_INFO, "application started");

  sock_init();

  read_config();

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
      return 1;
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
          return 0;
        case EXEC_UNKNOWN:
          log_add_fmt(LOG_CMD, "unknown command: %s\n", command);
          break;
        case EXEC_DONE:
          log_add_fmt(LOG_CMD, "done command: %s\n", command);
          break;
      }
    }
  }

  sock_deinit();

  return 0;
}
//==============================================================================
