#ifndef WSWORKER_H
#define WSWORKER_H
//==============================================================================
#include "defines.h"
#include "globals.h"
#include "protocol_types.h"
#include "socket_types.h"
#include "customworker.h"
//==============================================================================
typedef enum
{
  ERROR_FRAME=0xFF00,
  INCOMPLETE_FRAME=0xFE00,

  OPENING_FRAME=0x3300,
  CLOSING_FRAME=0x3400,

  INCOMPLETE_TEXT_FRAME=0x01,
  INCOMPLETE_BINARY_FRAME=0x02,

  TEXT_FRAME=0x81,
  BINARY_FRAME=0x82,

  PING_FRAME=0x19,
  PONG_FRAME=0x1A
} WSFrame_t;
//==============================================================================
typedef struct
{
  custom_server_t              custom_server;
  custom_remote_clients_list_t custom_remote_clients_list;
}ws_server_t;
//==============================================================================
int ws_server(sock_state_t state, sock_port_t port);
int ws_server_status();
int ws_server_route_pack(pack_packet_t *packet);
//==============================================================================
#endif //WSWORKER_H