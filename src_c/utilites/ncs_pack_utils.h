//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#ifndef PACK_UTILS_H
#define PACK_UTILS_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "ncs_pack.h"
//==============================================================================
int print_pack(pack_packet_t *packet, char *prefix, BOOL clear, BOOL buffer, BOOL pack, BOOL csv);
//==============================================================================
#endif //PACK_UTILS_H
