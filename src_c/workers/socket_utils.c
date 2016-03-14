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
