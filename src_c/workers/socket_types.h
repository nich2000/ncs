//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: socket_types.h
 */
//==============================================================================
#ifndef SOCKET_TYPES_H
#define SOCKET_TYPES_H
//==============================================================================
#ifdef __linux__
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/ip.h>
  #include <arpa/inet.h>
#elif _WIN32
  #include <winsock.h>
#else
#endif

#include <time.h>

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
#define SOCK_HOST_SIZE           15  // 255.255.255.255
#define SOCK_ERRORS_COUNT        10
//==============================================================================
#ifdef SAFE_MODE
  #define SOCK_WORKERS_COUNT     4
  #define SOCK_BUFFER_SIZE       (1024 * 512 )
  #define SOCK_WEB_REQUEST_SIZE  (1024 * 1   )
  #define SOCK_WEB_RESPONSE_SIZE (1024 * 512 )
  #define SOCK_WS_BUFFER_SIZE    (1024 * 1   )
#else
  #define SOCK_WORKERS_COUNT     256
  #define SOCK_BUFFER_SIZE       (1024 * 1024)
  #define SOCK_WEB_REQUEST_SIZE  (1024 * 1   )
  #define SOCK_WEB_RESPONSE_SIZE (1024 * 1024)
  #define SOCK_WS_BUFFER_SIZE    (1024 * 1   )
#endif
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
typedef unsigned short sock_connect_t;
typedef unsigned short sock_state_t;
typedef          short sock_active_t;
typedef unsigned short sock_register_t;
typedef unsigned char  sock_buffer_t[SOCK_BUFFER_SIZE];
typedef time_t         sock_time_t;
//==============================================================================

#endif //SOCKET_TYPES_H
