//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
 */
//==============================================================================
#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "socket_types.h"
#include "customworker.h"
//==============================================================================
int sock_print_server_header(sock_mode_t mode, sock_port_t port);
int sock_print_client_header(sock_port_t port, sock_host_t host);
//==============================================================================
int print_custom_worker_info(custom_worker_t *worker, char *prefix);
int print_custom_remote_clients_list_info(custom_remote_clients_list_t *clients_list, char *prefix);
//==============================================================================
#endif //SOCKET_UTILS_H
