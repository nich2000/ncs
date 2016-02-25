//==============================================================================
//==============================================================================
#include "socket_utils.h"
#include "socket.h"
//==============================================================================
const char *sock_mode_to_string(int mode)
{
  switch (mode) {
  case SOCK_MODE_UNKNOWN:
    return "SOCK_MODE_UNKNOWN";
    break;
  case SOCK_MODE_CLIENT:
    return "SOCK_MODE_CLIENT";
    break;
  case SOCK_MODE_SERVER:
    return "SOCK_MODE_SERVER";
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
const char *sock_type_to_string(int type)
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
