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
#define MODE_SERVER          0
#define MODE_CLIENT          1
#define SOCK_SERVER_STREAMER 0
#define SOCK_BUFFER_SIZE     100
#define SOCK_WORKERS_COUNT   32
//==============================================================================
typedef unsigned short sock_mode;
typedef unsigned short sock_id;
typedef unsigned short sock_index;
//==============================================================================
typedef struct
{
  sock_id                id;
  sock_mode              mode;
  SOCKET                 sock;
  pthread_t              sender;
  pthread_t              receiver;
  pack_validation_buffer validation_buffer;
  pack_out_packets_list  out_packets_list;
  pack_in_packets_list   in_packets_list;
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
int sock_init();
int sock_deinit();
//==============================================================================
int sock_server_init();
int sock_server_start(int port);
int sock_server_work();
int sock_server_stop();
//==============================================================================
int sock_client_init();
int sock_client_start(int port, const char *host);
int sock_client_work();
int sock_client_stop();
//==============================================================================
#endif //SOCKET_H
