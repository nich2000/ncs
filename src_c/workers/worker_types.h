//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: worker_types.h
 */
//==============================================================================
#ifndef WORKER_TYPES_H
#define WORKER_TYPES_H
//==============================================================================
#include <stdio.h>
#include <pthread.h>

#include "defines.h"
#include "globals.h"

#include "socket_types.h"
#include "protocol_types.h"
//==============================================================================
typedef int (*on_state_t)     (void *sender, sock_state_t state);
typedef int (*on_lock_t)      (void *sender, int is_locked);
typedef int (*on_accept_t)    (void *sender, SOCKET socket, sock_host_t host);
typedef int (*on_connect_t)   (void *sender);
typedef int (*on_disconnect_t)(void *sender);
typedef int (*on_send_t)      (void *sender);
typedef int (*on_recv_t)      (void *sender, char *buffer, int size);
//==============================================================================
typedef struct
{
  sock_id_t       id;
  sock_name_t     session_id;
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

  sock_connect_t  connect_state;
  sock_active_t   active_state;
  sock_register_t register_state;

  sock_time_t     connect_time;
  sock_time_t     disconnect_time;
  sock_time_t     active_time;
  sock_time_t     register_time;

  // TODO: Временное явление
  pack_protocol_t protocol;
  char           *out_message;
  int             out_message_size;
  BOOL            hand_shake;
  FILE           *report;
  FILE           *session;
  FILE           *stat;

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
#endif //WORKER_TYPES_H
