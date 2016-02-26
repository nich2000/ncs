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

  char tmp_pack_version[PACK_VALUE_SIZE];
  pack_version(tmp_pack_version);
  char tmp_sock_version[SOCK_VERSION_SIZE];
  sock_version(tmp_sock_version);
  sprintf(tmp, "Sock version: %s, Pack version: %s", tmp_sock_version, tmp_pack_version);
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

  char tmp_pack_version[PACK_VERSION_SIZE];
  pack_version(tmp_pack_version);
  char tmp_sock_version[SOCK_VERSION_SIZE];
  sock_version(tmp_sock_version);
  sprintf(tmp, "Sock version: %s, Pack version: %s", tmp_sock_version, tmp_pack_version);
  log_add(tmp, LOG_INFO);
  sprintf(tmp, "Client, port: %d, host: %s", port, host);
  log_add(tmp, LOG_INFO);

  log_add("----------", LOG_INFO);
}
//==============================================================================
