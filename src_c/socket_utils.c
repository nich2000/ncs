//==============================================================================
//==============================================================================
#include "socket_utils.h"
#include "log.h"
#include "protocol.h"
#include "socket.h"
//==============================================================================
const char *sock_mode_to_string(sock_mode_t mode)
{
  switch (mode) {
  case SOCK_MODE_UNKNOWN:
    return "SOCK_MODE_UNKNOWN";
    break;
  case SOCK_MODE_CMD_CLIENT:
    return "SOCK_MODE_CMD_CLIENT";
    break;
  case SOCK_MODE_CMD_SERVER:
    return "SOCK_MODE_CMD_SERVER";
    break;
  case SOCK_MODE_WS_SERVER:
    return "SOCK_MODE_WS_SERVER";
    break;
  case SOCK_MODE_WEB_SERVER:
    return "SOCK_MODE_WEB_SERVER";
    break;
  default:
    break;
  }
}
//==============================================================================
const char *sock_type_to_string(sock_type_t type)
{
  switch (type) {
  case SOCK_TYPE_UNKNOWN:
    return "SOCK_TYPE_UNKNOWN";
    break;
  case SOCK_TYPE_CLIENT:
    return "SOCK_TYPE_CLIENT";
    break;
  case SOCK_TYPE_SERVER:
    return "SOCK_TYPE_SERVER";
    break;
  case SOCK_TYPE_REMOTE_CLIENT:
    return "SOCK_TYPE_REMOTE_CLIENT";
    break;
  default:
    break;
  }
}
//==============================================================================
const char *sock_state_to_string(sock_state_t state)
{
  switch (state) {
  case SOCK_STATE_NONE:
    return "SOCK_STATE_NONE";
    break;
  case SOCK_STATE_START:
    return "SOCK_STATE_START";
    break;
    case SOCK_STATE_STARTING:
      return "SOCK_STATE_STARTING";
      break;
  case SOCK_STATE_STOP:
    return "SOCK_STATE_STOP";
    break;
    case SOCK_STATE_STOPPING:
      return "SOCK_STATE_STOPPING";
      break;
  case SOCK_STATE_PAUSE:
    return "SOCK_STATE_PAUSE";
    break;
    case SOCK_STATE_PAUSING:
      return "SOCK_STATE_PAUSING";
      break;
  default:
    break;
  }
}
//==============================================================================
int sock_print_server_header(sock_mode_t mode, sock_port_t port)
{
  char tmp[128];

  log_add("----------", LOG_INFO);

  sprintf(tmp, "Sock version: %s, Pack version: %s", sock_version(), _pack_version());
  log_add(tmp, LOG_INFO);
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
  log_add(tmp, LOG_INFO);

  log_add("----------", LOG_INFO);
}
//==============================================================================
int sock_print_client_header(sock_port_t port, sock_host_t host)
{
  char tmp[128];

  log_add("----------", LOG_INFO);

  sprintf(tmp, "Sock version: %s, Pack version: %s", sock_version(), _pack_version());
  log_add(tmp, LOG_INFO);
  sprintf(tmp, "Client, port: %d, host: %s", port, host);
  log_add(tmp, LOG_INFO);

  log_add("----------", LOG_INFO);
}
//==============================================================================
int sock_print_custom_worker_info(custom_worker_t *worker, char *prefix)
{
  if(worker == NULL)
    return 1;

  char tmp_info[1024];

  sprintf(tmp_info,
          "%s\n"                                  \
          "addr:                            %d\n" \
          "id:                              %d\n" \
          "type:                            %s\n" \
          "mode:                            %s\n" \
          "port:                            %d\n" \
          "host:                            %s\n" \
          "sock:                            %d\n" \
          "state:                           %s\n" \
          "is_locked:                       %d\n" \
          "work_thread:                     %d",
          prefix,
          &worker,
          worker->id,
          sock_type_to_string(worker->type),
          sock_mode_to_string(worker->mode),
          worker->port,
          worker->host,
          worker->sock,
          sock_state_to_string(worker->state),
          worker->is_locked,
          worker->work_thread);

  log_add(tmp_info, LOG_INFO);

  return 0;
}
//==============================================================================
/*
int sock_print_worker_info(sock_worker_t *worker, char *prefix)
{
  if(worker == NULL)
    return 1;

  print_custom_worker_info(&worker->custom_worker, "custom_worker");

  char tmp_info[1024];

  sprintf(tmp_info,
          "%s\n"                                  \
          "addr:                            %d\n" \
          "protocol.in_packets_list.index:  %d\n" \
          "protocol.in_packets_list.count:  %d\n" \
          "protocol.out_packets_list.index: %d\n" \
          "protocol.out_packets_list.count: %d",
          prefix,
          &worker,
          worker->protocol.in_packets_list.index,
          worker->protocol.in_packets_list.count,
          worker->protocol.out_packets_list.index,
          worker->protocol.out_packets_list.count);

  log_add(tmp_info, LOG_INFO);

  return 0;
}
*/
//==============================================================================
/*
int sock_print_server_info(sock_server_t *server)
{
  if(server == NULL)
    return 1;

  clr_scr();

  char tmp[256];

  sprintf(tmp,
          "server\n"                              \
          "addr:                            %u\n" \
          "clients.last_id:                 %d\n" \
          "clients.index:                   %d",
          server,
          server->clients.last_id,
          server->clients.index);
  log_add(tmp, LOG_INFO);

  print_worker_info(&server->worker, "server");

  for(int i = 0; i < server->clients.index; i++)
  {
    sock_worker_t tmp_worker = server->clients.items[i];
    print_worker_info(&tmp_worker, "remote client");
  }

  return 0;
}
*/
//==============================================================================
