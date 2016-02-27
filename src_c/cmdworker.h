#ifndef CMDWORKER_H
#define CMDWORKER_H
//==============================================================================
#include "customworker.h"
//==============================================================================
typedef struct
{
  custom_server_t custom_server;
}cmd_worker_t;
//==============================================================================
int cmd_server(sock_state_t state, sock_port_t port);
int cmd_server_status();
//==============================================================================
#endif //CMDWORKER_H
