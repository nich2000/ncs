#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include "defines.h"
#include "globals.h"
#include "socket_types.h"
#include "customworker.h"

const char *sock_mode_to_string(sock_mode_t mode);
const char *sock_type_to_string(sock_type_t type);

int sock_print_server_header(sock_mode_t mode, sock_port_t port);
int sock_print_client_header(sock_port_t port, sock_host_t host);

#endif //SOCKET_UTILS_H
