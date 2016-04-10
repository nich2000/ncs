//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
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
// atoi
#include <stdlib.h>

#include "exec.h"

#include "test.h"
#include "utils.h"
#include "socket.h"
#include "webworker.h"
#include "wsworker.h"
#include "cmdworker.h"
#include "streamer.h"
#include "ncs_log.h"
#include "ncs_error.h"
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
#define CMD_CMD_REGISTER     "register"       //
#define CMD_CMD_ACTIVATE     "activate"       //
#define CMD_WS_REGISTER      "ws_register"    //
#define CMD_WS_ACTIVATE      "ws_activate"    //
//==============================================================================
#define CMD_START           "on"
#define CMD_STOP            "off"
#define CMD_PAUSE           "pause"
#define CMD_RESUME          "resume"
#define CMD_STEP            "step"
//==============================================================================
#define CMD_FIRST           "first"
#define CMD_SECOND          "second"
#define CMD_NEXT            "next"
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
  else if(strcmp(CMD_STEP, cmd) == 0)
  {
    return STATE_STEP;
  }
  else
    return STATE_NONE;
}
//==============================================================================
sock_active_t cmd_active(char *cmd)
{
  if(cmd[strlen(cmd)-1] == '\n')
    cmd[strlen(cmd)-1] = '\0';

  if(strcmp(CMD_FIRST, cmd) == 0)
  {
    return ACTIVE_FIRST;
  }
  else if(strcmp(CMD_SECOND, cmd) == 0)
  {
    return ACTIVE_SECOND;
  }
  else if(strcmp(CMD_NEXT, cmd) == 0)
  {
    return ACTIVE_NEXT;
  }
  else
    return ACTIVE_NONE;
}
//==============================================================================
int handle_command_pack(void *sender, pack_packet_t *packet)
{
  pack_value_t cmd;
  if(pack_command(packet, cmd) == ERROR_NONE)
  {
    char command[1024];
    sprintf(command, "%s", cmd);

    pack_value_t value;
    pack_index_t index = 1;
    while(pack_next_param(packet, &index, value) == ERROR_NONE)
    {
      char tmp[128];
      sprintf(tmp, " %s", value);
      strcat(command, tmp);
    }

    return handle_command_str(sender, command);
  };

  return ERROR_NORMAL;
}
//==============================================================================
int handle_command_str(void *sender, char *command)
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
      log_add_fmt(LOG_INFO, "token: %s", CMD_ALL);

      log_set_name("all_log.txt");

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      cmd_server(state, DEFAULT_CMD_SERVER_PORT);
      web_server(state, DEFAULT_WEB_SERVER_PORT);
      ws_server (state, DEFAULT_WS_SERVER_PORT);

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

      pack_packet_t tmp_packet;
      pack_init(&tmp_packet);
      int cnt = 0;
      char *arg = strtok(NULL, " ");
      while(arg != NULL)
      {
        if(cnt == 0)
          pack_add_cmd(&tmp_packet, (unsigned char*)arg);
        else
          pack_add_param(&tmp_packet, (unsigned char*)arg);
        arg = strtok(NULL, " ");
        cnt++;
      }
      cmd_client_send_pack(&tmp_packet);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SND_TO_WSSERVER) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_SND_TO_WSSERVER);

      int tmp_session_id = SOCK_SEND_TO_ALL;
      char *arg = strtok(NULL, " ");
      if(arg != NULL)
        tmp_session_id = atoi(arg);

      pack_packet_t tmp_packet;
      pack_init(&tmp_packet);
      int cnt = 0;
      arg = strtok(NULL, " ");
      while(arg != NULL)
      {
        if(cnt == 0)
          pack_add_cmd(&tmp_packet, (unsigned char*)arg);
        else
          pack_add_param(&tmp_packet, (unsigned char*)arg);
        arg = strtok(NULL, " ");
        cnt++;
      }

      ws_server_send_pack(tmp_session_id, &tmp_packet);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SND_TO_CLIENT) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_SND_TO_CLIENT);

      pack_packet_t tmp_packet;
      pack_init(&tmp_packet);
      int cnt = 0;
      char *arg = strtok(NULL, " ");
      while(arg != NULL)
      {
        if(cnt == 0)
          pack_add_cmd(&tmp_packet, (unsigned char*)arg);
        else
          pack_add_param(&tmp_packet, (unsigned char*)arg);
        arg = strtok(NULL, " ");
        cnt++;
      }
      cmd_server_send_pack(&tmp_packet);

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
    else if(strcmp(token, CMD_CMD_ACTIVATE) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_CMD_ACTIVATE);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CMD_REGISTER) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_CMD_REGISTER);

      char *name_str = strtok(NULL, " ");
      if(name_str != NULL)
      {
        sock_id_t id = ((custom_worker_t*)sender)->id;

        cmd_remote_client_register(id, (unsigned char*)name_str);
      }

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WS_ACTIVATE) == 0)
    {
      char *name_str = strtok(NULL, " ");
      log_add_fmt(LOG_INFO, "token: %s, sender: %s", CMD_WS_ACTIVATE, name_str);

      sock_id_t id = -1;
      char *id_str = strtok(NULL, " ");
      if(id_str != NULL)
        id = atoi(id_str);

      sock_active_t active = ACTIVE_NONE;
      char *active_str = strtok(NULL, " ");
      if(active_str != NULL)
        active = cmd_active(active_str);

      switch(active)
      {
        case ACTIVE_FIRST:
          cmd_remote_client_activate_all(ACTIVE_NONE, ACTIVE_SECOND);
          break;
        case ACTIVE_SECOND:
          cmd_remote_client_activate_all(ACTIVE_NONE, ACTIVE_FIRST);
          break;
        case ACTIVE_NEXT:
          cmd_remote_client_activate_all(ACTIVE_NONE, ACTIVE_NEXT);
          break;
        case ACTIVE_NONE:
        default:
          cmd_remote_client_activate_all(ACTIVE_NONE, ACTIVE_NONE);
          break;
      }

      cmd_remote_client_activate(id, active);

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WS_REGISTER) == 0)
    {
      log_add_fmt(LOG_INFO, "token: %s", CMD_WS_REGISTER);

      char *name_str = strtok(NULL, " ");
      if(name_str != NULL)
      {
        sock_id_t id = ((custom_worker_t*)sender)->id;

        ws_remote_clients_register(id, (unsigned char*)name_str);
      }

      return EXEC_DONE;
    }
    //--------------------------------------------------------------------------
  }
  return EXEC_UNKNOWN;
}
//==============================================================================
