//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#ifndef SOCKET_H
#define SOCKET_H
//==============================================================================
#include <unistd.h>
#include <pthread.h>

#include "defines.h"
#include "socket_types.h"
//==============================================================================
const char *_sock_version();
int sock_error();
//==============================================================================
int sock_init();
int sock_deinit();
//==============================================================================
int sock_accept (SOCKET sock, SOCKET *remote_sock, sock_host_t host, sock_port_t *port);
int sock_connect(SOCKET sock, sock_port_t port, sock_host_t host);
int sock_send   (SOCKET sock, char *buffer, int  size);
int sock_recv   (SOCKET sock, char *buffer, int *size);
//==============================================================================
#endif //SOCKET_H
