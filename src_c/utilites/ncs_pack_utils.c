//==============================================================================
//==============================================================================
#include <string.h>

#include "ncs_pack_utils.h"
#include "ncs_error.h"
#include "ncs_log.h"
//==============================================================================
#define POLY 0x8408
unsigned short getCRC16(char *data, unsigned short length)
{
  unsigned char i;
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
int bytes_to_int(unsigned char *bytes, int *value)
{
  intUnion tmp_value;

  for(unsigned int j = 0; j < sizeof(int); j++)
    tmp_value.buff[j] = bytes[j];

  *value = tmp_value.i;

  return 0;
}
//==============================================================================
int bytes_to_float(unsigned char *bytes, float *value)
{
  floatUnion tmp_value;

  for(unsigned int j = 0; j < sizeof(float); j++)
    tmp_value.buff[j] = bytes[j];

  *value = tmp_value.f;

  return 0;
}
//==============================================================================
int bytes_from_int(unsigned char *bytes, int value)
{
  intUnion tmp_value;

  tmp_value.i = value;

  for(unsigned int j = 0; j < sizeof(float); j++)
    bytes[j] = tmp_value.buff[j];

  return 0;
}
//==============================================================================
int bytes_from_float(unsigned char *bytes, float value)
{
  floatUnion tmp_value;

  tmp_value.f = value;

  for(unsigned int j = 0; j < sizeof(float); j++)
    bytes[j] = tmp_value.buff[j];

  return 0;
}
//==============================================================================
unsigned char nibbles[] = {"0123456789ABCDEF"};
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

  return ERROR_NONE;
}
//==============================================================================
char *do_print_pack(pack_packet_t *packet, int indent)
{
  pack_key_t    tmp_key;
  pack_value_t  tmp_value;
  pack_size_t   tmp_words_count = _pack_words_count(packet);
  pack_buffer_t tmp;
  memset(tmp, '\0', PACK_BUFFER_SIZE);

  char *indent_str[128];
  memset(indent_str, '\0', 128);
  for(int i = 0; i < indent; i++)
    sprintf(indent_str, "%s%s", indent_str, " ");

  sprintf(tmp, "\n%swords_count: %d", indent_str, tmp_words_count);

  for(pack_size_t i = 0; i < tmp_words_count; i++)
  {
    pack_word_t *tmp_word = &packet->words[i];

    sprintf(tmp, "%s\n%sword: %d, type: %d, key: %s", tmp, indent_str, i, tmp_word->type, tmp_word->key);

    if(tmp_word->type == PACK_WORD_PACK)
    {
      pack_packet_t tmp_pack;
      if(pack_word_as_pack(tmp_word, &tmp_pack) == ERROR_NONE)
        sprintf(tmp, "%s\n%s    value: %s",                tmp, indent_str, do_print_pack(&tmp_pack, indent+4));
      else
        sprintf(tmp, "%s\n%s    value: error parse value", tmp, indent_str);
    }
    else
    {
      if(pack_val_by_index_as_string(packet, i, tmp_key, tmp_value) == ERROR_NONE)
        sprintf(tmp, "%s\n%s    value: %s",                tmp, indent_str, tmp_value);
      else
        sprintf(tmp, "%s\n%s    value: error parse value", tmp, indent_str);
    }
  }

  return tmp;
}
//==============================================================================
int print_pack(pack_packet_t *packet, char *prefix, BOOL clear, BOOL buffer, BOOL pack, BOOL csv)
{
//  if(clear)
//    clr_scr();

  if(!buffer && !pack && !csv)
    return ERROR_NONE;

  if(buffer)
  {
    pack_buffer_t tmp_buffer;
    pack_size_t   tmp_size;
    pack_buffer_t tmp;

    pack_to_buffer(packet, tmp_buffer, &tmp_size);

    #ifdef SOCK_PACK_MODE
    bytes_to_hex((unsigned char*)tmp_buffer, (pack_size_t)tmp_size, (unsigned char*)tmp);
    log_add(tmp, LOG_DEBUG);
    #else
    log_add(tmp_buffer, LOG_DEBUG);
    #endif
  }

  if(pack)
  {
    char tmp[1024];
    sprintf(tmp, "%s", do_print_pack(packet, 4));
    log_add(tmp, LOG_INFO);
  }

  if(csv)
  {
//    pack_buffer_t csv;
//    pack_values_to_csv(packet, ';', csv);
//    report_add(csv);
  }

  return ERROR_NONE;
}
//==============================================================================
