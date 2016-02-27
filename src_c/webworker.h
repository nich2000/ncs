#ifndef WEBWORKER_H
#define WEBWORKER_H
//==============================================================================
#include "customworker.h"
//==============================================================================
#define WEB_LINE_SIZE      256
#define WEB_INITIAL_PATH   "../www"
//==============================================================================
typedef struct
{
  custom_server_t custom_server;
}web_worker_t;
//==============================================================================
int web_server(sock_state_t state, sock_port_t port);
int web_server_status();
//==============================================================================
#endif //WEBWORKER_H
