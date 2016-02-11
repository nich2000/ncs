//==============================================================================
#include "protocol_utils.h"
//==============================================================================
/*
* b[0] = si & 0xff;
* b[1] = (si >> 8) & 0xff;
* si = (b[0] << 8) | b[1];
*/
//==============================================================================
int bytes_to_int  (unsigned char *bytes, int size, int   *value)
{
  intUnion tmp_value;
  for(unsigned int j = 0; j < sizeof(int); j++)
    tmp_value.buff[j] = bytes[j];
  *value = tmp_value.i;

  return 0;
}
//==============================================================================
int bytes_to_float(unsigned char *bytes, int size, float *value)
{
  floatUnion tmp_value;
  for(unsigned int j = 0; j < sizeof(float); j++)
    tmp_value.buff[j] = bytes[j];
  *value = tmp_value.f;

  return 0;
}
//==============================================================================
char nibbles[] = {"0123456789ABCDEF"};
int bytes_to_hex(unsigned char *bytes, int size, unsigned char *hex)
{
  int j = 0;

  for(int i = 0; i < size; i++)
  {
    hex[j++] = nibbles[bytes[i] >> 0x04];
    hex[j++] = nibbles[bytes[i] &  0x0F];
    hex[j++] = ' ';
  }

  hex[j] = '0';

  return 0;
}
//==============================================================================
//int bytes_from_int  (unsigned char *bytes, size_t size, int   *value);
//int bytes_from_float(unsigned char *bytes, size_t size, float *value);
//==============================================================================
// http://stjarnhimlen.se/snippets/crc-16.c
//==============================================================================
#define POLY 0x8408
//==============================================================================
unsigned short getCRC16(char *data_p, unsigned short length)
{
  unsigned char i;
  unsigned int data;
  unsigned int crc = 0xffff;
  if (length == 0)
    return (~crc);
  do
  {
    for (i=0, data=(unsigned int)0xff & *data_p++;
         i < 8;
         i++, data >>= 1)
    {
      if ((crc & 0x0001) ^ (data & 0x0001))
        crc = (crc >> 1) ^ POLY;
      else
        crc >>= 1;
    }
  }
  while (--length);
  crc = ~crc;
  data = crc;
  crc = (crc << 8) | (data >> 8 & 0xff);
  return (crc);
}
//==============================================================================
