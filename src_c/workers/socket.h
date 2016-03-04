#ifndef SOCKET_H
#define SOCKET_H
//==============================================================================
#include <unistd.h>
#include <pthread.h>

#include "defines.h"
#include "socket_types.h"
//==============================================================================
const char *sock_version();
int sock_error();
//==============================================================================
int sock_init();
int sock_deinit();
//==============================================================================
int sock_send(SOCKET sock, char *buffer, int  size);
int sock_recv(SOCKET sock, char *buffer, int *size);
//==============================================================================
#endif //SOCKET_H
