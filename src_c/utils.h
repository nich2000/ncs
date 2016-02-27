#ifndef UTILS_H
#define UTILS_H

#include "socket.h"
#include "protocol.h"

int print_types_info();
int print_defines_info();
int print_custom_worker_info(custom_worker_t *worker, char *prefix);
int print_worker_info(sock_worker_t *worker, char *prefix);
int print_server_info(sock_server_t *server);

#endif //UTILS_H
