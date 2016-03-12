//==============================================================================
//==============================================================================
#include "socket_utils.h"
#include "ncs_log.h"
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
    return "SOCK_MODE_UNKNOWN";
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
    return "SOCK_TYPE_UNKNOWN";
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
    return "SOCK_STATE_UNKNOWN";
    break;
  }
}
//==============================================================================
int sock_print_server_header(sock_mode_t mode, sock_port_t port)
{
  char tmp[128];

  log_add("----------", LOG_INFO);

  log_add_fmt(LOG_INFO, "Sock version: %s, Pack version: %s", _sock_version(), _pack_version());

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

  return ERROR_NONE;
}
//==============================================================================
int sock_print_client_header(sock_port_t port, sock_host_t host)
{
  log_add("----------", LOG_INFO);

  log_add_fmt(LOG_INFO, "Sock version: %s, Pack version: %s", _sock_version(), _pack_version());
  log_add_fmt(LOG_INFO, "Client, port: %d, host: %s", port, host);

  log_add("----------", LOG_INFO);

  return ERROR_NONE;
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
          "is_locked:                       %d\n",
          prefix,
          (int)&worker,
          worker->id,
          sock_type_to_string(worker->type),
          sock_mode_to_string(worker->mode),
          worker->port,
          worker->host,
          worker->sock,
          sock_state_to_string(worker->state),
          worker->is_locked);

  log_add(tmp_info, LOG_INFO);

  return 0;
}
//==============================================================================
int sock_print_custom_remote_clients_list_info(custom_remote_clients_list_t *clients_list, char *prefix)
{
  if(clients_list == 0)
    return 1;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(clients_list->items[i].custom_worker.state == SOCK_STATE_START)
      sock_print_custom_worker_info(&clients_list->items[i].custom_worker, prefix);

  return 0;
}
//==============================================================================
