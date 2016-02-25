#ifndef PROTOCOL_UTILS_H
#define PROTOCOL_UTILS_H
//==============================================================================
#include "protocol.h"
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
int bytes_to_int  (unsigned char *bytes, int size, int   *value);
int bytes_to_float(unsigned char *bytes, int size, float *value);
int bytes_to_hex  (unsigned char *bytes, int size, unsigned char *hex);
//==============================================================================
//int bytes_from_int  (unsigned char *bytes, size_t size, int   *value);
//int bytes_from_float(unsigned char *bytes, size_t size, float *value);
//==============================================================================
unsigned short getCRC16(char *data_p, unsigned short length);
//==============================================================================
//int pack_print(pack_packet *packet, char *prefix, int clear, int buffer, int pack, int csv);
//==============================================================================
#endif //PROTOCOL_UTILS_H
