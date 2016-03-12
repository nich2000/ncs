#ifndef UTILS_H
#define UTILS_H
//==============================================================================
#include "defines.h"
#include "ncs_pack.h"
//==============================================================================
int bytes_to_hex(unsigned char *bytes, int size, unsigned char *hex);
//==============================================================================
int print_types_info();
int print_defines_info();
int pack_print(pack_packet_t *packet, char *prefix, int clear, int buffer, int pack, int csv);
//==============================================================================
#endif //UTILS_H
