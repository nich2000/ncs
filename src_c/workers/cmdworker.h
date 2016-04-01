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
int cmd_server(sock_state_t state, sock_port_t port);
int cmd_server_status();
//==============================================================================
int cmd_server_send_cmd (int argc, ...);
int cmd_server_send_pack(pack_packet_t *pack);
//==============================================================================
int cmd_remote_clients_list(pack_packet_t *pack);
int cmd_remote_clients_activate(sock_id_t id, sock_active_t active);
int cmd_remote_clients_activate_all(sock_active_t active, sock_active_t except);
int cmd_remote_clients_register(sock_id_t id, sock_name_t name);
//==============================================================================
int cmd_client(sock_state_t state, sock_port_t port, sock_host_t host, int count);
int cmd_client_status();
//==============================================================================
int cmd_client_send_cmd (int argc, ...);
int cmd_client_send_pack(pack_packet_t *pack);
//==============================================================================
#endif //CMDWORKER_H
