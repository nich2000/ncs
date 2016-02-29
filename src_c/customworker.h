#ifndef CUSTOMWORKER_H
#define CUSTOMWORKER_H
//==============================================================================
#include <pthread.h>

#include "socket_types.h"
//==============================================================================
typedef int (*on_error_t)     (void *sender);
typedef int (*on_accept_t)    (void *sender, SOCKET socket);
typedef int (*on_connect_t)   (void *sender);
typedef int (*on_disconnect_t)(void *sender);
typedef int (*on_send_t)      (void *sender);
typedef int (*on_recv_t)      (void *sender);
//==============================================================================
typedef struct
{
  sock_id_t       id;
  sock_type_t     type;
  sock_mode_t     mode;
  sock_port_t     port;
  sock_host_t     host;
  SOCKET          sock;
  sock_state_t    state;
  int             is_locked;
  pthread_t       work_thread;

  on_error_t      on_error;
  on_send_t       on_send;
  on_recv_t       on_recv;
}custom_worker_t;
//==============================================================================
typedef custom_worker_t custom_workers_t[SOCK_WORKERS_COUNT];
//==============================================================================
typedef struct
{
  sock_index_t     index;
  custom_workers_t items;
} custom_workers_list_t;
//==============================================================================
typedef struct
{
  custom_worker_t custom_worker;

  on_accept_t     on_accept;
}custom_server_t;
//==============================================================================
typedef struct
{
  custom_worker_t custom_worker;

  on_connect_t    on_connect;
  on_disconnect_t on_disconnect;
}custom_client_t;
//==============================================================================
int custom_worker_init (custom_worker_t *worker);
int custom_worker_stop (custom_worker_t *worker);

int custom_server_start(custom_worker_t *worker);
int custom_client_start(custom_worker_t *worker);

int custom_server_work (custom_server_t *server);
int custom_client_work (custom_client_t *client);
//==============================================================================
#endif //CUSTOMWORKER_H
