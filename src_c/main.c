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
int getcode()
{
  struct termios oldt, newt;
  int ch;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
}
//==============================================================================
void exec_interactive_interpreter(int argc, char** argv)
{
  Py_Initialize();
  Py_Main(argc, argv);
  Py_Finalize();
}
//==============================================================================
int main(int argc, char *argv[])
{
  //---------------------------------------------------------------------------
  //exec_interactive_interpreter(argc, argv);
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
      if(read_config() >= ERROR_WARNING)
        log_add_fmt(LOG_ERROR, "main,\nmessage: %s",
                    last_error()->message);
      print_config();
      return 0;
    }
  }
  //---------------------------------------------------------------------------
  print_version();
  //---------------------------------------------------------------------------
  if(read_config() >= LOG_ERROR)
    log_add_fmt(LOG_ERROR, "main,\nmessage: %s",
                last_error()->message);
  print_config();
  //---------------------------------------------------------------------------
  log_add(LOG_INFO, "application started");
  log_add(LOG_INFO, "-------------------");
  //---------------------------------------------------------------------------
  if(sock_init() >= ERROR_NORMAL)
    goto exit;
  //---------------------------------------------------------------------------
  #ifdef USE_PYTHON
  if(py_init() >= ERROR_NORMAL)
    goto exit;
  #endif
  //---------------------------------------------------------------------------
  #ifdef PI_DEVICE
  if(gpio_init() >= ERROR_NORMAL)
    goto exit;
  #endif
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
  //---------------------------------------------------------------------------
  if(history_load() >= ERROR_NORMAL)
    log_add_fmt(LOG_ERROR, "main,\nmessage: %s",
                last_error()->message);
  log_add(LOG_INFO, "command mode");
  //---------------------------------------------------------------------------
  strcpy(command, "\0");
  while(TRUE)
  {
//    char ch = getcode();
//    if(ch == '\033')
//    {
//      getchar(); // skip the [
//      ch = getchar();
//      switch(ch)
//      {
//        case 'A': // arrow up
//        {
//          strcpy(command, history_prev());
//          break;
//        }
//        case 'B': // arrow down
//        {
//          strcpy(command, history_next());
//          break;
//        }
//      }
//      if(strlen(command) != 0)
//      {
//        printf("%s\n", command);
//        continue;
//      }
//    }

//    if(strlen(command) == 0)
//    {
//      printf("manual input\n");
      fgets(command, sizeof(command), stdin);
      history_add(command);
//    };

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
    strcpy(command, "\0");
  }
  //---------------------------------------------------------------------------
  exit:
  sock_deinit();
  #ifdef PI_DEVICE
  gpio_close();
  #endif
  //---------------------------------------------------------------------------
  #ifdef USE_PYTHON
  py_final();
  #endif
  //---------------------------------------------------------------------------
  log_add_fmt(LOG_INFO, "application finished, result: %s\n", last_error()->message);
  //---------------------------------------------------------------------------
  return 0;
  //---------------------------------------------------------------------------
}
//==============================================================================
