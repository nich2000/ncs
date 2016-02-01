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
#define SOCK_WORKERS_COUNT 32
//==============================================================================
typedef unsigned short sock_index;
//==============================================================================
typedef struct
{
  SOCKET    _socket;
  pthread_t _thread;
}sock_worker;
//==============================================================================
typedef sock_worker sock_workers[SOCK_WORKERS_COUNT];
//==============================================================================
typedef struct
{
  sock_index   index;
  sock_workers items;
}sock_worker_list;
//==============================================================================
int sock_init();
int sock_deinit();
//==============================================================================
int sock_server_start(int port);
int sock_server_stop();
int sock_server_work();
//==============================================================================
int sock_client_start(int port, const char *host);
int sock_client_work();
int sock_client_stop();
//==============================================================================
static void *sock_work_recv(void *arg);
static void *sock_work_send(void *arg);
//==============================================================================
#endif //SOCKET_H
