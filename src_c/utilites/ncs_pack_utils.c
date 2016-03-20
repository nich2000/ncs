//==============================================================================
//==============================================================================
#include "ncs_pack_utils.h"
//==============================================================================
#define POLY 0x8408
//==============================================================================
unsigned short getCRC16(char *data, unsigned short length)
{
  char i;
  unsigned int tmp_data;
  unsigned int crc = 0xffff;

  if (length == 0)
    return (~crc);

  do
  {
    for (i=0, tmp_data = (unsigned int)0xff & *data++;
         i < 8;
         i++, tmp_data >>= 1)
    {
      if ((crc & 0x0001) ^ (tmp_data & 0x0001))
        crc = (crc >> 1) ^ POLY;
      else
        crc >>= 1;
    }
  }
  while (--length);

  crc = ~crc;
  tmp_data = crc;
  crc = (crc << 8) | (tmp_data >> 8 & 0xff);

  return (crc);
}
//==============================================================================
int bytes_to_int(char *bytes, int *value)
{
  intUnion tmp_value;

  for(unsigned int j = 0; j < sizeof(int); j++)
    tmp_value.buff[j] = bytes[j];

  *value = tmp_value.i;

  return 0;
}
//==============================================================================
int bytes_to_float(char *bytes, float *value)
{
  floatUnion tmp_value;

  for(unsigned int j = 0; j < sizeof(float); j++)
    tmp_value.buff[j] = bytes[j];

  *value = tmp_value.f;

  return 0;
}
//==============================================================================
int bytes_from_int(char *bytes, int value)
{
  intUnion tmp_value;

  tmp_value.i = value;

  for(unsigned int j = 0; j < sizeof(float); j++)
    bytes[j] = tmp_value.buff[j];

  return 0;
}
//==============================================================================
int bytes_from_float(char *bytes, float value)
{
  floatUnion tmp_value;

  tmp_value.f = value;

  for(unsigned int j = 0; j < sizeof(float); j++)
    bytes[j] = tmp_value.buff[j];

  return 0;
}
//==============================================================================
