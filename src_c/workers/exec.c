//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: exec.c
 */
//==============================================================================
/**
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
sock_port_t web_server_port   = DEFAULT_WEB_SERVER_PORT;
sock_port_t ws_server_port    = DEFAULT_WS_SERVER_PORT;
sock_port_t cmd_server_port   = DEFAULT_CMD_SERVER_PORT;
sock_host_t cmd_server_host   = DEFAULT_SERVER_HOST;
int         cmd_clients_count = 1;
//==============================================================================
extern char worker_name[16];
extern BOOL session_relay_to_web;
extern BOOL log_enable;
extern char log_path[256];
extern BOOL stat_enable;
extern char stat_path[256];
extern BOOL report_enable;
extern char report_path[265];
extern BOOL session_enable;
extern char session_path[256];
extern char session_file[64];
extern BOOL map_enable;
extern char map_path[256];
extern char map_file[64];
extern char log_prefix[8];
extern char web_path[256];
//==============================================================================
static pthread_mutex_t mutex_handle_command = PTHREAD_MUTEX_INITIALIZER;
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
int read_config()
{
  dictionary *config = iniparser_load("../config/config.ini");
  if(config == NULL)
    return make_last_error(ERROR_CRITICAL, errno, "read_config, iniparser_load");

  cmd_server_port =              iniparser_getint   (config, "worker:cmd_server_port",      DEFAULT_CMD_SERVER_PORT);
  ws_server_port  =              iniparser_getint   (config, "worker:ws_server_port",       DEFAULT_WS_SERVER_PORT);
  web_server_port =              iniparser_getint   (config, "worker:web_server_port",      DEFAULT_WEB_SERVER_PORT);
  strcpy((char*)cmd_server_host, iniparser_getstring(config, "worker:cmd_server_host",      DEFAULT_SERVER_HOST));
  strcpy((char*)worker_name,     iniparser_getstring(config, "worker:worker_name",          DEFAULT_SOCK_NAME));
  session_relay_to_web =         iniparser_getint   (config, "worker:session_relay_to_web", DEFAULT_CMD_SERVER_PORT);

  log_enable =                   iniparser_getint   (config, "log:log_enable",              TRUE);
  strcpy((char*)log_path,        iniparser_getstring(config, "log:log_path",                DEFAULT_LOG_PATH));

  stat_enable =                  iniparser_getint   (config, "stat:stat_enable",            TRUE);
  strcpy((char*)stat_path,       iniparser_getstring(config, "stat:stat_path",              DEFAULT_STAT_PATH));

  report_enable =                iniparser_getint   (config, "report:report_enable",        TRUE);
  strcpy((char*)report_path,     iniparser_getstring(config, "report:report_path",          DEFAULT_REPORT_PATH));

  session_enable =               iniparser_getint   (config, "session:session_enable",      TRUE);
  strcpy((char*)session_path,    iniparser_getstring(config, "session:session_path",        DEFAULT_SESSION_PATH));
  strcpy((char*)session_file,    iniparser_getstring(config, "session:session_file",        DEFAULT_SESSION_NAME));

  session_enable =               iniparser_getint   (config, "map:map_enable",              TRUE);
  strcpy((char*)map_path,        iniparser_getstring(config, "map:map_path",                DEFAULT_MAP_PATH));
  strcpy((char*)map_file,        iniparser_getstring(config, "map:map_file",                DEFAULT_MAP_NAME));

  strcpy((char*)web_path,        iniparser_getstring(config, "web:web_path",                DEFAULT_WEB_PATH));

  iniparser_freedict(config);

  return ERROR_NONE;
}
//==============================================================================
int handle_command_ajax(void *sender, char *command)
{
  char *token = strtok(command, "&");
  if(token == NULL)
    goto exit;

  char tmp_command[1024];
  int i = 0;

  while(token != NULL)
  {
    char *tmp = strstr(token, "=");
    if(tmp != NULL)
    {
      if(i == 0)
      {
        sprintf(tmp_command, "%s", &tmp[1]);
        i++;
      }
      else
      {
        char tmp1[128];
        sprintf(tmp1, " %s", &tmp[1]);
        strcat(tmp_command, tmp1);
      }
    }

    token = strtok(NULL, "&");
  }

  log_add(LOG_INFO, tmp_command);
  return handle_command_str(sender, tmp_command);

  exit:
  return make_last_error_fmt(ERROR_NORMAL, errno, "handle_command_ajax,\nmessage: %s",
                             "ajax is not the command");
}
//==============================================================================
int handle_command_pack(void *sender, pack_packet_t *packet)
{
  pack_value_t cmd;
  if(pack_command(packet, cmd) == ERROR_NONE)
  {
    char command[1024];
    sprintf(command, "%s", cmd);
//    log_add_fmt(LOG_INFO, "handle_command_pack, command: %s", command);

    pack_value_t value;
    pack_index_t index = 1;
    while(pack_next_param(packet, &index, value) == ERROR_NONE)
    {
      char tmp[128];
      sprintf(tmp, " %s", value);
      strcat(command, tmp);
    }

    return handle_command_str(sender, command);
  }

  return make_last_error_fmt(ERROR_NORMAL, errno, "handle_command_pack,\nmessage: %s",
                             "pack is not the commnad");
}
//==============================================================================
int handle_command_str_fmt(void *sender, char *command, ...)
{
  return ERROR_NONE;
}
//==============================================================================
int handle_command_str(void *sender, char *command)
{
   #ifndef DEMS_DEVICE
   pthread_mutex_lock(&mutex_handle_command);
   #endif

  int result = EXEC_UNKNOWN;

  char *token = strtok(command, " ");
  if(token != NULL)
  {
    if(token[strlen(token)-1] == '\n')
      token[strlen(token)-1] = '\0';
    //--------------------------------------------------------------------------
    if(strcmp(token, CMD_HELP) == 0)
    {
      print_help(0);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_VERSION) == 0)
    {
      print_version();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CONFIG) == 0)
    {
      print_config();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_TEST) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_TEST);
      test();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CLEAR) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_CLEAR);
      clr_scr();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_EXIT) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_EXIT);

      result = EXEC_NONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_ALL) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_ALL);

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      cmd_server(state, cmd_server_port);
      web_server(state, web_server_port);
      ws_server (state, ws_server_port);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SERVER) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_SERVER);

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      sock_port_t port = cmd_server_port;
      char *port_str = strtok(NULL, " ");
      if(port_str != NULL)
        port = atoi(port_str);

      cmd_server(state, port);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WEB_SERVER) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_WEB_SERVER);

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      sock_port_t port = web_server_port;
      char *port_str = strtok(NULL, " ");
      if(port_str != NULL)
        port = atoi(port_str);

      web_server(state, port);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WS_SERVER) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_WS_SERVER);

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      sock_port_t port = ws_server_port;
      char *port_str = strtok(NULL, " ");
      if(port_str != NULL)
        port = atoi(port_str);

      ws_server(state, port);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CLIENT) == 0)
    {
      strcpy(log_prefix, "cl.txt");
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_CLIENT);

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      sock_port_t port = cmd_server_port;
      char *port_str = strtok(NULL, " ");
      if(port_str != NULL)
        port = atoi(port_str);

      sock_host_t host;
      strcpy((char*)host, (char*)cmd_server_host);
      char *host_str = strtok(NULL, " ");
      if(host_str != NULL)
        strcpy((char*)host, host_str);

      int count = cmd_clients_count;
      char *count_str = strtok(NULL, " ");
      if(count_str != NULL)
        count = atoi(count_str);

      cmd_client(state, port, host, count);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SND_TO_SERVER) == 0)
    {
      log_add_fmt(LOG_DEBUG, "token: %s, full: %s",
                  CMD_SND_TO_SERVER, command);

      sock_id_t id = -1;
      char *arg = strtok(NULL, " ");
      if(arg != NULL)
        id = atoi(arg);

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

      if(tmp_packet.words_count == 0)
        log_add_fmt(LOG_ERROR, "handle_command_str, token: %s, words count = 0",
                    CMD_SND_TO_SERVER);
      else
        cmd_client_send_pack(id, &tmp_packet);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SND_TO_WSSERVER) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_SND_TO_WSSERVER);

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

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SND_TO_CLIENT) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_SND_TO_CLIENT);

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

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_STREAM) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_STREAM);

      sock_state_t state = STATE_START;
      char *state_str = strtok(NULL, " ");
      if(state_str != NULL)
        state = cmd_state(state_str);

      int interval = 1000;
      char *interval_str = strtok(NULL, " ");
      if(interval_str != NULL)
        interval = atoi(interval_str);

       cmd_streamer(state, interval);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_TYPES_INFO) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_TYPES_INFO);

      print_types_info();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_DEFINES_INFO) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_DEFINES_INFO);

      print_defines_info();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_SERVER_INFO) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_SERVER_INFO);

      cmd_server_status();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WEB_SERVER_INFO) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_WEB_SERVER_INFO);

      web_server_status();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WS_SERVER_INFO) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_WS_SERVER_INFO);

      ws_server_status();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CLIENT_INFO) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_CLIENT_INFO);

      cmd_client_status();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CMD_ACTIVATE) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_CMD_ACTIVATE);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_CMD_REGISTER) == 0)
    {
      custom_worker_t *worker = (custom_worker_t*)sender;
      char *session_id = strtok(NULL, " ");

      log_add_fmt(LOG_DEBUG, "token: %s, sender id: %d, sender name: %s, session_id: %s",
                  CMD_CMD_REGISTER, worker->id,  worker->name, session_id);

      // TODO: excess search by id...need use sender as worker
      if(session_id != NULL)
        cmd_remote_client_register(worker->id, (unsigned char*)session_id);

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WS_ACTIVATE) == 0)
    {
      char *name_str = strtok(NULL, " ");
      log_add_fmt(LOG_EXTRA, "token: %s, sender: %s", CMD_WS_ACTIVATE, name_str);

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

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_WS_REGISTER) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_WS_REGISTER);

      char *name_str = strtok(NULL, " ");
      if(name_str != NULL)
      {
        sock_id_t id = ((custom_worker_t*)sender)->id;

        ws_remote_clients_register(id, (unsigned char*)name_str);
      }

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
    else if(strcmp(token, CMD_RECONFIG) == 0)
    {
      log_add_fmt(LOG_EXTRA, "token: %s", CMD_RECONFIG);

      read_config();

      result = EXEC_DONE;
      goto exit;
    }
    //--------------------------------------------------------------------------
  }

  exit:
   #ifndef DEMS_DEVICE
   pthread_mutex_unlock(&mutex_handle_command);
   #endif
  return result;
}
//==============================================================================
