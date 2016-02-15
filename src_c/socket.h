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
#define SOCK_VERSION        "SOCK001"
#define SOCK_VERSION_SIZE   7
//==============================================================================
#define SOCK_OK              0
#define SOCK_ERROR          -1
//==============================================================================
#define SOCK_MODE_UNKNOWN   -1
#define SOCK_MODE_SERVER     0
#define SOCK_MODE_CLIENT     1
#define SOCK_SERVER_STREAMER 0
#define SOCK_BUFFER_SIZE     100
#define SOCK_WORKERS_COUNT   32
#define SOCK_ERRORS_COUNT    10
//==============================================================================
#define SOCK_WAIT_SELECT     5
#define SOCK_WAIT_CONNECT    5
//==============================================================================
typedef unsigned short sock_mode;
typedef unsigned short sock_id;
typedef unsigned short sock_index;
//==============================================================================
typedef struct
{
  sock_id                id;
  sock_mode              mode;
  int                    port;
  char                   host[15];
  SOCKET                 sock;
  int                    worker_kill_flag;
  int                    sender_kill_flag;
  int                    receiver_kill_flag;
  pthread_t              worker;
  pthread_t              sender;
  pthread_t              receiver;
  pack_protocol          protocol;
  exec_func              exec_cmd;
}sock_worker;
//==============================================================================
typedef sock_worker sock_workers[SOCK_WORKERS_COUNT];
//==============================================================================
typedef struct
{
  sock_id      last_id;
  sock_index   index;
  sock_workers items;
}sock_worker_list;
//==============================================================================
typedef struct
{
  int is_stream;
  int last_number;
}sock_streamer;
//==============================================================================
int sock_version(char *version);
//==============================================================================
int sock_server(int port);
int sock_client(int port, char *host);
int sock_exit();
//==============================================================================
int sock_send_cmd(int argc, ...);
//==============================================================================
int soch_exec_cmd(int argc, ...);
//==============================================================================
#endif //SOCKET_H
