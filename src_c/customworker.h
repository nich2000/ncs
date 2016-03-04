#ifndef CUSTOMWORKER_H
#define CUSTOMWORKER_H
//==============================================================================
#include <pthread.h>

#include "defines.h"
#include "protocol_types.h"
#include "socket_types.h"
#include "ncs_error.h"
//==============================================================================
typedef int (*on_accept_t)    (void *sender, SOCKET socket, sock_host_t host);
typedef int (*on_connect_t)   (void *sender);
typedef int (*on_disconnect_t)(void *sender);
typedef int (*on_send_t)      (void *sender);
typedef int (*on_recv_t)      (void *sender, char *buffer, int size);
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
}custom_worker_t;
//==============================================================================
typedef struct
{
  custom_worker_t custom_worker;

  pthread_t       send_thread;
  pthread_t       recv_thread;

  // Временное явление 1
  pack_protocol_t protocol;
  // Временное явление 2
  char           *out_message;
  int             out_message_size;
  // Временное явление 3
  int             hand_shake;

  on_error_t      on_error;
  on_send_t       on_send;
  on_recv_t       on_recv;
  on_disconnect_t on_disconnect;
}custom_remote_client_t;
//==============================================================================
typedef custom_remote_client_t custom_remote_clients_t[SOCK_WORKERS_COUNT];
//==============================================================================
typedef struct
{
  sock_index_t            index;
  sock_id_t               next_id;
  custom_remote_clients_t items;
} custom_remote_clients_list_t;
//==============================================================================
typedef struct
{
  custom_worker_t custom_worker;

  pthread_t       work_thread;

  on_accept_t     on_accept;
}custom_server_t;
//==============================================================================
typedef struct
{
  custom_remote_client_t custom_remote_client;

  pthread_t              work_thread;

  on_connect_t           on_connect;
}custom_client_t;
//==============================================================================
int custom_worker_init (custom_worker_t *worker);
int custom_remote_client_init(custom_remote_client_t *custom_remote_client);
int custom_remote_clients_list_init(custom_remote_clients_list_t *custom_remote_clients_list);
int custom_server_init(custom_server_t *custom_server);
int custom_client_init(custom_client_t *custom_client);

int custom_worker_stop (custom_worker_t *worker);

int custom_server_start(custom_worker_t *worker);
int custom_client_start(custom_worker_t *worker);

int custom_server_work (custom_server_t *server);
int custom_client_work (custom_client_t *client);

void *custom_recv_worker(void *arg);
//==============================================================================
#endif //CUSTOMWORKER_H
