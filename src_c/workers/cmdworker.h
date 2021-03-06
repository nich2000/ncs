//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: cmdworker.h
 */
//==============================================================================
#ifndef CMDWORKER_H
#define CMDWORKER_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "customworker.h"
//==============================================================================
typedef struct
{
  custom_server_t              custom_server;
  custom_remote_clients_list_t custom_remote_clients_list;
}cmd_server_t;
//==============================================================================
typedef struct
{
  custom_client_t              custom_client;
}cmd_client_t;
//==============================================================================
typedef struct
{
  char session_id[PACK_VALUE_SIZE];
  char name[PACK_VALUE_SIZE];
} name_item_t;
//==============================================================================
typedef name_item_t name_items_t[SOCK_WORKERS_COUNT];
//==============================================================================
typedef struct
{
  int         count;
  name_items_t items;
} names_t;
//==============================================================================
typedef cmd_client_t cmd_clients_t[SOCK_WORKERS_COUNT];
//==============================================================================
int cmd_client_count();
cmd_clients_t *cmd_clients();
cmd_client_t *cmd_client_by_index(int index);
//==============================================================================
int cmd_server(sock_state_t state, sock_port_t port);
int cmd_server_status();
//==============================================================================
int cmd_server_send_cmd (int argc, ...);
int cmd_server_send_pack(pack_packet_t *pack);
//==============================================================================
int cmd_remote_client_list(pack_packet_t *pack);
int cmd_map(pack_packet_t *pack);
//==============================================================================
int cmd_remote_client_activate(sock_id_t id, sock_active_t active);
int cmd_remote_client_activate_all(sock_active_t active, sock_active_t except);
int cmd_remote_client_register(sock_id_t id, sock_name_t session_id);
//==============================================================================
int cmd_client(sock_state_t state, sock_port_t port, sock_host_t host, int count);
int cmd_client_status();
//==============================================================================
int cmd_client_send_cmd (sock_id_t client_id, int argc, ...);
int cmd_client_send_pack(sock_id_t client_id, pack_packet_t *pack);
//==============================================================================
#endif //CMDWORKER_H
