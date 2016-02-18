#ifndef SOCKET_H
#define SOCKET_H
//==============================================================================
#ifdef __linux__
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/ip.h>
  #include <arpa/inet.h>
  #include <errno.h>
#elif _WIN32
  #include <winsock.h>
#else
#endif
//==============================================================================
#include <pthread.h>

#include "defines.h"
#include "protocol.h"
#include "streamer.h"
#include "exec.h"
//==============================================================================
#ifdef __linux__
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR   -1

  typedef int SOCKET;

  #define closesocket close
#elif _WIN32
  #define WINSOCK_VERSION 0x0101
#else
#endif
//==============================================================================
//#define SOCK_EXTRA_LOGS
//==============================================================================
#define SOCK_TYPE_UNKNOWN       -1
#define SOCK_TYPE_CLIENT         0
#define SOCK_TYPE_SERVER         1
#define SOCK_TYPE_REMOTE_CLIENT  2
//==============================================================================
#define SOCK_MODE_UNKNOWN       -1
#define SOCK_MODE_CLIENT         0
#define SOCK_MODE_SERVER         1
#define SOCK_MODE_WS_SERVER      2
#define SOCK_MODE_WEB_SERVER     3
//==============================================================================
#define SOCK_VERSION             "SOCK001"
#define SOCK_VERSION_SIZE        7
//==============================================================================
#define SOCK_OK                  0
#define SOCK_ERROR              -1
//==============================================================================
#define SOCK_SERVER_STREAMER     0
#define SOCK_BUFFER_SIZE         100
#define SOCK_WORKERS_COUNT       64
#define SOCK_ERRORS_COUNT        10
//==============================================================================
#define SOCK_WAIT_SELECT         5
#define SOCK_WAIT_CONNECT        5
//==============================================================================
typedef unsigned short sock_mode_t;
typedef unsigned short sock_type_t;
typedef unsigned short sock_id_t;
typedef unsigned short sock_index_t;
//==============================================================================
typedef struct
{
  sock_id_t              id;
  sock_type_t            type;
  sock_mode_t            mode;
  int                    port;
  char                   host[15];
  SOCKET                 sock;
  int                    worker_kill_flag;
  int                    sender_kill_flag;
  int                    receiver_kill_flag;
  pthread_t              work_thread;
  pthread_t              send_thread;
  pthread_t              receive_thread;
  pack_protocol          protocol;
  exec_func              exec_cmd;
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
typedef struct
{
  sock_worker_t      worker;
  streamer_worker  streamer;
}sock_client_t;
//==============================================================================
int sock_version(char *version);
//==============================================================================
int sock_server(int port, sock_server_t *server, sock_mode_t mode);
int sock_client(int port, char *host, sock_client_t *client);
//==============================================================================
int sock_exit(sock_worker_t *worker);
//==============================================================================
int sock_send_cmd(int argc, ...);
int soch_exec_cmd(int argc, ...);
//==============================================================================
#endif //SOCKET_H
