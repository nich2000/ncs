//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: socket_utils.c
 */
//==============================================================================
#include "socket_utils.h"

#include "ncs_log.h"
#include "protocol.h"
#include "socket.h"
#include "utils.h"
//==============================================================================
int sock_print_server_header(sock_mode_t mode, sock_port_t port)
{
  log_add_fmt(LOG_INFO, "sock version: %s, pack version: %s",
              _sock_version(), _pack_version());

  char tmp[128];

  switch(mode)
  {
    case SOCK_MODE_CMD_SERVER:
    sprintf(tmp, "Server(CMD_SERVER), port: %d", port);
    break;
    case SOCK_MODE_WS_SERVER:
    sprintf(tmp, "Server(WS_SERVER), port: %d", port);
    break;
    case SOCK_MODE_WEB_SERVER:
    sprintf(tmp, "Server(WEB_SERVER), port: %d", port);
    break;
  }
  log_add(LOG_INFO, tmp);

  return ERROR_NONE;
}
//==============================================================================
int sock_print_client_header(sock_port_t port, sock_host_t host)
{
  log_add_fmt(LOG_INFO, "sock version: %s, pack version: %s",
              _sock_version(), _pack_version());
  log_add_fmt(LOG_INFO, "Client, port: %d, host: %s",
              port, host);

  return ERROR_NONE;
}
//==============================================================================
int print_custom_worker_info(custom_worker_t *worker, char *prefix)
{
  if(worker == NULL)
    return 1;

  char tmp[1024];

  #ifdef PRINT_ALL_INFO
  sprintf(tmp,
          "%s\n"                                  \
          "addr:                            %d\n" \
          "id:                              %d\n" \
          "name:                            %s\n" \
          "type:                            %s\n" \
          "mode:                            %s\n" \
          "port:                            %d\n" \
          "host:                            %s\n" \
          "sock:                            %d\n" \
          "state:                           %s\n" \
          "is_locked:                       %d",
          prefix,
          (int)&worker,
          worker->id,
          worker->name,
          sock_type_to_string(worker->type),
          sock_mode_to_string(worker->mode),
          worker->port,
          worker->host,
          worker->sock,
          state_to_string(worker->state),
          worker->is_locked);
  #else
  sprintf(tmp,
          "%s\n"                                         \
          "       id:                              %d\n" \
          "       sock:                            %d\n" \
          "       name:                            %s\n" \
          "       type:                            %s\n" \
          "       mode:                            %s\n" \
          "       state:                           %s",
          prefix,
          worker->id,
          worker->sock,
          worker->session_id,
          sock_type_to_string(worker->type),
          sock_mode_to_string(worker->mode),
          state_to_string(worker->state));
  #endif

  log_add(LOG_INFO, tmp);

  return ERROR_NONE;
}
//==============================================================================
int print_remote_client_info(custom_remote_client_t *remote_client, char *prefix)
{
  if(remote_client == NULL)
    return 1;

  char tmp[1024];

  sprintf(tmp,
          "%s\n"                                         \
          "       connect_time:                    %s\n" \
          "       active_state:                    %s\n" \
          "       active_time:                     %s\n" \
          "       register_state:                  %s\n" \
          "       register_time:                   %s",
          prefix,
          time_to_string(remote_client->connect_time),
          active_to_string(remote_client->active_state),
          time_to_string(remote_client->active_time),
          register_to_string(remote_client->register_state),
          time_to_string(remote_client->register_time));

  log_add(LOG_INFO, tmp);

  return ERROR_NONE;
}
//==============================================================================
int print_custom_remote_clients_list_info(custom_remote_clients_list_t *clients_list, char *prefix)
{
  if(clients_list == 0)
    return 1;

  log_add(LOG_INFO, "---------");
  log_add_fmt(LOG_INFO,
              "clients                        \n" \
              "connected:                   %d\n" \
              "registered:                  %d\n" \
              "active:                      %d",
              _custom_remote_clients_count_con(clients_list),
              _custom_remote_clients_count_reg(clients_list),
              _custom_remote_clients_count_act(clients_list));
  log_add(LOG_INFO, "---------");

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_t *tmp_remote_client = &clients_list->items[i];
    custom_worker_t *tmp_worker = &tmp_remote_client->custom_worker;

    if(tmp_worker->state == STATE_START)
    {
      print_custom_worker_info(tmp_worker, "remote client");
      print_remote_client_info(tmp_remote_client, "worker");
      log_add(LOG_INFO, "---------");
    }
  }

  return ERROR_NONE;
}
//==============================================================================
