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
typedef union
{
  int  i;
  char buff[sizeof(int)];
} intUnion;
//==============================================================================
typedef union
{
  float f;
  char  buff[sizeof(float)];
} floatUnion;
//==============================================================================
unsigned short getCRC16(char *data, unsigned short length);
//==============================================================================
int bytes_to_int    (unsigned char *bytes, int   *value);
int bytes_to_float  (unsigned char *bytes, float *value);
int bytes_to_hex    (unsigned char *bytes, int size, unsigned char *hex);
//==============================================================================
int bytes_from_int  (unsigned char *bytes, int   value);
int bytes_from_float(unsigned char *bytes, float value);
//==============================================================================
int print_pack(pack_packet_t *packet, char *prefix, BOOL clear, BOOL buffer, BOOL pack, BOOL csv);
//==============================================================================
#endif //PACK_UTILS_H
