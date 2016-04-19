//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: exec.h
 */
//==============================================================================
#ifndef EXEC_H
#define EXEC_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "ncs_pack.h"
//==============================================================================
#define EXEC_UNKNOWN            -1
#define EXEC_NONE                0
#define EXEC_DONE                1
//==============================================================================
#define MAX_COMMAND_SIZE         128
//==============================================================================
typedef int (*exec_func)(int, ...);
//==============================================================================
int handle_command_str (void *sender, char          *command);
int handle_command_pack(void *sender, pack_packet_t *packet);
//==============================================================================
#endif //EXEC_H
