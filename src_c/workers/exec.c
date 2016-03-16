//==============================================================================
/*
 * -- cmd exapmles --
 * server
 * server 5600
 * client
 * client 127.0.0.1
 * client 127.0.0.1 5600
 * stream on
*/
//==============================================================================
#include <string.h>

#include "test.h"
#include "utils.h"
#include "socket.h"
#include "webworker.h"
#include "wsworker.h"
#include "cmdworker.h"
#include "streamer.h"
#include "ncs_log.h"
#include "ncs_error.h"
#include "exec.h"
//==============================================================================
#define CMD_TEST            "test"           // 0
#define CMD_CLEAR           "clear"          // 0
#define CMD_EXIT            "exit"           // 0
#define CMD_ALL             "all"            // 0 start all default servers
#define CMD_SERVER          "server"         // 1 - 2(state, port)
#define CMD_WEB_SERVER      "webserver"      // 1 - 2(state, port)
#define CMD_WS_SERVER       "wsserver"       // 1 - 2(state, port)
#define CMD_CLIENT          "client"         // 1 - 3(state, host, port)
#define CMD_SND_TO_SERVER   "sndtosr"        // 1 - n
#define CMD_SND_TO_WSSERVER "sndtows"        // 1 - n
#define CMD_SND_TO_CLIENT   "sndtocl"        // 1 - n
#define CMD_STREAM          "stream"         // 1(on off pause resume)
#define CMD_TYPES_INFO      "typesinfo"      // 0
#define CMD_DEFINES_INFO    "definesinfo"    // 0
#define CMD_SERVER_INFO     "serverinfo"     // 0
#define CMD_WEB_SERVER_INFO "webserverinfo"  // 0
#define CMD_WS_SERVER_INFO  "wsserverinfo"   // 0
#define CMD_CLIENT_INFO     "clientinfo"     // 0
//==============================================================================
#define CMD_START           "on"
#define CMD_STOP            "off"
#define CMD_PAUSE           "pause"
#define CMD_RESUME          "resume"
//==============================================================================
sock_state_t cmd_state(char *cmd)
{
  if(cmd[strlen(cmd)-1] == '\n')
    cmd[strlen(cmd)-1] = '\0';

  if(strcmp(CMD_START, cmd) == 0)
  {
    return STATE_START;
  }
  else if(strcmp(CMD_STOP, cmd) == 0)
  {
    return STATE_STOP;
  }
  else if(strcmp(CMD_PAUSE, cmd) == 0)
  {
    return STATE_PAUSE;
  }
  else if(strcmp(CMD_RESUME, cmd) == 0)
  {
    return STATE_RESUME;
  }
  else
    return STATE_NONE;
}
//==============================================================================
int handle_command(char *command)
{
  char *token = strtok(command, " ");
  if(token != NULL)
  {
    if(token[strlen(token)-1] == '\n')
      token[strlen(token)-1] = '\0';
    //--------------------------------------------------------------------------
    if(strcmp(token, CMD_TEST) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_TEST);
      test();
      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CLEAR) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_CLEAR);
      clr_scr();
      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_EXIT) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_EXIT);
      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_ALL) == 0)
    {
      cmd_server(STATE_START, DEFAULT_CMD_SERVER_PORT);
      web_server(STATE_START, DEFAULT_WEB_SERVER_PORT);
      ws_server (STATE_START, DEFAULT_WS_SERVER_PORT);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SERVER) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_SERVER);

      log_set_name("server_log.txt");

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      sock_port_t port = DEFAULT_CMD_SERVER_PORT;
      char *port_str = strtok(NULL, " ");
      if(port_str != NULL)
        port = atoi(port_str);

      cmd_server(state, port);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WEB_SERVER) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_WEB_SERVER);

      log_set_name("web_server_log.txt");

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      sock_port_t port = DEFAULT_WEB_SERVER_PORT;
      char *port_str = strtok(NULL, " ");
      if(port_str != NULL)
        port = atoi(port_str);

      web_server(state, port);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WS_SERVER) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_WS_SERVER);

      log_set_name("ws_server_log.txt");

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      sock_port_t port = DEFAULT_WS_SERVER_PORT;
      char *port_str = strtok(NULL, " ");
      if(port_str != NULL)
        port = atoi(port_str);

      ws_server(state, port);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CLIENT) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_CLIENT);

      log_set_name("client_log.txt");

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      sock_port_t port = DEFAULT_CMD_SERVER_PORT;
      char *port_str = strtok(NULL, " ");
      if(port_str != NULL)
        port = atoi(port_str);

      sock_host_t host = DEFAULT_SERVER_HOST;
      char *host_str = strtok(NULL, " ");
      if(host_str != NULL)
        strcpy((char*)host, host_str);

      int count = 1;
      char *count_str = strtok(NULL, " ");
      if(count_str != NULL)
        count = atoi(count_str);

      cmd_client(state, port, host, count);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SND_TO_SERVER) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_SND_TO_SERVER);
      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SND_TO_WSSERVER) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_SND_TO_WSSERVER);
      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SND_TO_CLIENT) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_SND_TO_CLIENT);
      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_STREAM) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_STREAM);

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      cmd_streamer(state);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_TYPES_INFO) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_TYPES_INFO);

      print_types_info();

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_DEFINES_INFO) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_DEFINES_INFO);

      print_defines_info();

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SERVER_INFO) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_SERVER_INFO);

      cmd_server_status();

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WEB_SERVER_INFO) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_WEB_SERVER_INFO);

      web_server_status();

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WS_SERVER_INFO) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_WS_SERVER_INFO);

      ws_server_status();

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CLIENT_INFO) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_CLIENT_INFO);

      cmd_client_status();

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
  }
  return EXEC_UNKNOWN;
}
//==============================================================================
/*
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

//      cmd_server(SOCK_STATE_START, cmd_server_port);
//      ws_server (SOCK_STATE_START, ws_server_port);
//      web_server(SOCK_STATE_START, web_server_port);

    return RESULT_DONE;
  }
  else if(strcmp(token, "client") == 0)
  {
    log_set_name("client_log.txt");

//      cmd_client(SOCK_STATE_START, cmd_server_port, cmd_server_host, 1);

    return RESULT_DONE;
  }
  else if(strcmp(token, "server") == 0)
  {
    log_set_name("server_log.txt");

//      cmd_server(SOCK_STATE_START, cmd_server_port);

    return RESULT_DONE;
  }
  else if(strcmp(token, "wsserver") == 0)
  {
    log_set_name("ws_server_log.txt");

//      ws_server(SOCK_STATE_START, ws_server_port);

    return RESULT_DONE;
  }
  else if(strcmp(token, "webserver") == 0)
  {
    log_set_name("web_server_log.txt");

//      web_server(SOCK_STATE_START, web_server_port);

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
  else if(strcmp(token, "sndtosr") == 0)
  {
    cmd_client_send(1, param1);
    return RESULT_DONE;
  }
  else if(strcmp(token, "sndtows") == 0)
  {
    ws_server_send_cmd(1, param1);
    return RESULT_DONE;
  }
  else if(strcmp(token, "client") == 0)
  {
    log_set_name("client_log.txt");

//      cmd_client(SOCK_STATE_START, cmd_server_port, cmd_server_host, atoi(param1));

    return RESULT_DONE;
  }
  else if(strcmp(token, "stream") == 0)
  {
    if(strcmp(param1, "on") == 0)
      cmd_streamer(SOCK_STATE_START);
    else if(strcmp(param1, "off") == 0)
      cmd_streamer(SOCK_STATE_STOP);
    else if(strcmp(param1, "pause") == 0)
      cmd_streamer(SOCK_STATE_PAUSE);
    else if(strcmp(param1, "resume") == 0)
      cmd_streamer(SOCK_STATE_RESUME);
    else
      return RESULT_UNKNOWN;

    return RESULT_DONE;
  }

  return RESULT_UNKNOWN;
}
else if(res == 3)
{
  if(strcmp(token, "sndtocl") == 0)
  {
    cmd_server_send(2, param1, param2);
    return RESULT_DONE;
  }
  else if(strcmp(token, "sndtosr") == 0)
  {
    cmd_client_send(2, param1, param2);
    return RESULT_DONE;
  }
  else if(strcmp(token, "sndtows") == 0)
  {
    ws_server_send_cmd(2, param1, param2);
    return RESULT_DONE;
  }

  return RESULT_UNKNOWN;
}
else if(res == 4)
{
  if(strcmp(token, "sndtocl") == 0)
  {
    cmd_server_send(3, param1, param2, param3);
    return RESULT_DONE;
  }
  else if(strcmp(token, "sndtosr") == 0)
  {
    cmd_client_send(3, param1, param2, param3);
    return RESULT_DONE;
  }
  else if(strcmp(token, "sndtows") == 0)
  {
    ws_server_send_cmd(3, param1, param2, param3);
    return RESULT_DONE;
  }

  return RESULT_UNKNOWN;
}
else if(res == 5)
{
  if(strcmp(token, "sndtocl") == 0)
  {
    cmd_server_send(4, param1, param2, param3, param4);
    return RESULT_DONE;
  }
  else if(strcmp(token, "sndtosr") == 0)
  {
    cmd_client_send(4, param1, param2, param3, param4);
    return RESULT_DONE;
  }

  return RESULT_UNKNOWN;
}
else if(res == 6)
{
  if(strcmp(token, "sndtocl") == 0)
  {
    cmd_server_send(5, param1, param2, param3, param4, param5);
    return RESULT_DONE;
  }
  else if(strcmp(token, "sndtosr") == 0)
  {
    cmd_client_send(5, param1, param2, param3, param4, param5);
    return RESULT_DONE;
  }

  return RESULT_UNKNOWN;
}
else
  return RESULT_UNKNOWN;
*/
//==============================================================================
