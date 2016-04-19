//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: wsworker.h
 */
//==============================================================================
#ifndef WSWORKER_H
#define WSWORKER_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "protocol_types.h"
#include "socket_types.h"
#include "worker_types.h"
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
}WSFrame_t;
//==============================================================================
typedef struct
{
  custom_server_t              custom_server;
  custom_remote_clients_list_t custom_remote_clients_list;
}ws_server_t;
//==============================================================================
int ws_server(sock_state_t state, sock_port_t port);
int ws_server_status();
//==============================================================================
int ws_server_send_cmd (int session_id, int argc, ...);
int ws_server_send_pack(int session_id, pack_packet_t *pack);
//==============================================================================
int ws_remote_clients_register(sock_id_t id, sock_name_t name);
//==============================================================================
#endif //WSWORKER_H
