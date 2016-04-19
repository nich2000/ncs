//==============================================================================
/**
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
#ifndef __linux__
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif
//==============================================================================
int random_range(int min, int max);
//==============================================================================
const char *sock_mode_to_string(sock_mode_t     value);
const char *sock_type_to_string(sock_type_t     value);
const char *state_to_string    (sock_state_t    value);
const char *active_to_string   (sock_active_t   value);
const char *time_to_string     (sock_time_t     value);
const char *register_to_string (sock_register_t value);
//==============================================================================
int print_types_info();
int print_defines_info();
//==============================================================================
#endif //UTILS_H
