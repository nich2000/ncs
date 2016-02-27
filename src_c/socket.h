#ifndef SOCKET_H
#define SOCKET_H
//==============================================================================
#include <unistd.h>
#include <pthread.h>

#include "defines.h"
#include "protocol_types.h"
#include "customworker.h"
#include "socket_types.h"
//==============================================================================
/*
* TODO добавить некий указатель на ExtData или ExtParam,
* чтобы добавить функционал различным видами воркеров,
* например для WS нужен параметр handshake
*/
typedef struct
{
  custom_worker_t        custom_worker;
  pthread_t              send_thread;
  pthread_t              receive_thread;
  pack_protocol          protocol;
//  exec_func              exec_cmd;
//  streamer_worker        streamer;
  int                    handshake;
  char                   *in_massage;
  int                    in_message_size;
  char                   *out_message;
  int                    out_message_size;
}sock_worker_t;
//==============================================================================
typedef sock_worker_t sock_workers_t[SOCK_WORKERS_COUNT];
//==============================================================================
typedef struct
{
  sock_id_t      last_id;
  sock_index_t   index;
  sock_workers_t items;
}sock_worker_list_t;
//==============================================================================
typedef struct
{
  sock_worker_t      worker;
  sock_worker_list_t clients;
}sock_server_t;
//==============================================================================
int sock_version(char *version);
int sock_get_error();
//==============================================================================
int sock_init();
int sock_deinit();
//==============================================================================
int sock_server(sock_server_t *server, sock_port_t port, sock_mode_t mode);
int sock_server_send_cmd  (sock_server_t *server, int argc, ...);
int soch_server_exec_cmd  (sock_server_t *server, int argc, ...);
int sock_server_send_to_ws(sock_server_t *server, int argc, ...);
//==============================================================================
int sock_client(int port, char *host, sock_worker_t *worker);
int sock_client_send_cmd(sock_worker_t *worker, int argc, ...);
int soch_client_exec_cmd(sock_worker_t *worker, int argc, ...);
//==============================================================================
#endif //SOCKET_H
