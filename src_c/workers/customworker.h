#ifndef CUSTOMWORKER_H
#define CUSTOMWORKER_H
//==============================================================================
#include <stdio.h>
#include <pthread.h>

#include "defines.h"
#include "globals.h"
#include "protocol_types.h"
#include "socket_types.h"
#include "ncs_error.h"
//==============================================================================
typedef int (*on_state_t)     (void *sender, sock_state_t state);
typedef int (*on_lock_t)      (void *sender, int is_locked);
typedef int (*on_accept_t)    (void *sender, SOCKET socket, sock_host_t host);
typedef int (*on_connect_t)   (void *sender);
typedef int (*on_disconnect_t)(void *sender);
typedef int (*on_send_t)      (void *sender);
typedef int (*on_recv_t)      (void *sender, unsigned char *buffer, int size);
//==============================================================================
typedef struct
{
  sock_id_t       id;
  sock_name_t     name;
  sock_type_t     type;
  sock_mode_t     mode;
  sock_port_t     port;
  sock_host_t     host;
  SOCKET          sock;
  sock_state_t    state;
  BOOL            is_locked;

  on_state_t      on_state;
  on_lock_t       on_lock;
}custom_worker_t;
//==============================================================================
typedef struct
{
  custom_worker_t custom_worker;

  pthread_t       send_thread;
  pthread_t       recv_thread;

  BOOL            active;

  // TODO Временное явление 1
  pack_protocol_t protocol;
  // TODO Временное явление 2
  char           *out_message;
  int             out_message_size;
  // TODO Временное явление 3
  BOOL            hand_shake;
  // TODO Временное явление 4
  FILE           *report;

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

int custom_remote_client_deinit(custom_remote_client_t *custom_remote_client);

int custom_worker_stop  (custom_worker_t *worker);

int custom_server_start (custom_worker_t *worker);
int custom_client_start (custom_worker_t *worker);

int custom_server_work  (custom_server_t *server);
int custom_client_work  (custom_client_t *client);

void *custom_recv_worker(void *arg);

int _custom_server_remote_clients_count(custom_remote_clients_list_t *clients_list);
custom_remote_client_t *_custom_server_remote_clients_next(custom_remote_clients_list_t *clients_list);
//==============================================================================
#endif //CUSTOMWORKER_H
