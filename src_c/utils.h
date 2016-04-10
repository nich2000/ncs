//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#ifndef UTILS_H
#define UTILS_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "socket_types.h"
#include "worker_types.h"
//==============================================================================
int random_range(int min, int max);
//==============================================================================
const char *state_to_string   (sock_state_t    value);
const char *active_to_string  (sock_active_t   value);
const char *time_to_string    (sock_time_t     value);
const char *register_to_string(sock_register_t value);
//==============================================================================
int print_types_info();
int print_defines_info();
//==============================================================================
int print_custom_worker_info(custom_worker_t *worker, char *prefix);
int print_custom_remote_clients_list_info(custom_remote_clients_list_t *clients_list, char *prefix);
//==============================================================================
#endif //UTILS_H
