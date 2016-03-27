#ifndef UTILS_H
#define UTILS_H
//==============================================================================
#include "defines.h"
#include "globals.h"
#include "customworker.h"
//==============================================================================
const char *state_to_string(sock_state_t state);
//==============================================================================
int bytes_to_hex(unsigned char *bytes, int size, unsigned char *hex);
//==============================================================================
int print_types_info();
int print_defines_info();
//==============================================================================
int print_custom_worker_info(custom_worker_t *worker, char *prefix);
int print_custom_remote_clients_list_info(custom_remote_clients_list_t *clients_list, char *prefix);
//==============================================================================
#endif //UTILS_H
