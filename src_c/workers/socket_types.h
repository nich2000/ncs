#ifndef SOCKET_TYPES_H
#define SOCKET_TYPES_H
//==============================================================================
#ifdef __linux__
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/ip.h>
  #include <arpa/inet.h>
  #include <errno.h>
#elif _WIN32
  #include <winsock.h>
#else
#endif
//==============================================================================
#include "defines.h"
#include "globals.h"
#include "ncs_pack.h"
//==============================================================================
#ifdef __linux__
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR   -1

  #define closesocket close

  typedef int SOCKET;
#elif _WIN32
  #define WINSOCK_VERSION 0x0101

  typedef int socklen_t;
#else
#endif
//==============================================================================
#define SOCK_TYPE_UNKNOWN        0
#define SOCK_TYPE_CLIENT         1
#define SOCK_TYPE_SERVER         2
#define SOCK_TYPE_REMOTE_CLIENT  3
//==============================================================================
#define SOCK_MODE_UNKNOWN        0
#define SOCK_MODE_CMD_CLIENT     1
#define SOCK_MODE_CMD_SERVER     2
#define SOCK_MODE_WS_SERVER      3
#define SOCK_MODE_WEB_SERVER     4
//==============================================================================
#define SOCK_VERSION             "SOCK001\0"
#define SOCK_NAME_DEFAULT        "DEVICE\0"
#define SOCK_VERSION_SIZE        8
#define SOCK_HOST_SIZE           15  // 255.255.255.255
//==============================================================================
#define SOCK_PART_SIZE           100
#define SOCK_BUFFER_SIZE         102400
#define SOCK_WORKERS_COUNT       256
#define SOCK_ERRORS_COUNT        10
//==============================================================================
#define SOCK_WAIT_SELECT         5
#define SOCK_WAIT_CONNECT        5
//==============================================================================
#define SOCK_SEND_TO_ALL        -1
//==============================================================================
typedef unsigned short sock_port_t;
typedef unsigned char  sock_host_t[SOCK_HOST_SIZE];
typedef unsigned short sock_mode_t;
typedef unsigned short sock_type_t;
typedef unsigned short sock_id_t;
typedef unsigned char  sock_name_t[PACK_VALUE_SIZE];
typedef unsigned char  sock_data_t[PACK_VALUE_SIZE];
typedef unsigned short sock_index_t;
typedef unsigned short sock_state_t;
typedef unsigned char  sock_buffer_t[SOCK_BUFFER_SIZE];
//==============================================================================

#endif //SOCKET_TYPES_H
