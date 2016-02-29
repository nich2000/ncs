#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include "socket_types.h"
#include "customworker.h"

const char *sock_mode_to_string(sock_mode_t mode);
const char *sock_type_to_string(sock_type_t type);
const char *sock_state_to_string(sock_state_t state);

int sock_print_custom_worker_info(custom_worker_t *worker, char *prefix);
//int sock_print_worker_info(sock_worker_t *worker, char *prefix);
//int sock_print_server_info(sock_server_t *server);

int sock_print_server_header(sock_mode_t mode, sock_port_t port);
int sock_print_client_header(sock_port_t port, sock_host_t host);

#endif //SOCKET_UTILS_H
