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
int cmd_server_list(pack_packet_t *pack);
int cmd_server_activate(sock_id_t id);
//==============================================================================
int cmd_client(sock_state_t state, sock_port_t port, sock_host_t host, int count);
int cmd_client_status();
//==============================================================================
int cmd_client_send_cmd (int argc, ...);
int cmd_client_send_pack(pack_packet_t *pack);
//==============================================================================
#endif //CMDWORKER_H
