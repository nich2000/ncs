//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
// memset
#include <string.h>
// malloc
#include <stdlib.h>

#include "ncs_pack_utils.h"

#include "ncs_error.h"
#include "ncs_log.h"
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
  pack_string_t tmp = (unsigned char*)malloc(PACK_BUFFER_SIZE);
  memset(tmp, '\0', PACK_BUFFER_SIZE);

  char indent_str[128];
  memset(indent_str, '\0', 128);
  for(int i = 0; i < indent; i++)
    sprintf(indent_str, "%s%s", indent_str, " ");

  sprintf((char*)tmp, "\n%swords_count: %d", (char*)indent_str, tmp_words_count);

  for(pack_size_t i = 0; i < tmp_words_count; i++)
  {
    pack_word_t *tmp_word = &packet->words[i];

    sprintf((char*)tmp, "%s\n%sword: %d, type: %d, key: %s", tmp, (char*)indent_str, i, tmp_word->type, tmp_word->key);

    if(tmp_word->type == PACK_WORD_PACK)
    {
      pack_packet_t tmp_pack;
      if(pack_word_as_pack(tmp_word, &tmp_pack) == ERROR_NONE)
      {
        char *tmp_output = do_print_pack(&tmp_pack, indent+4);
        sprintf((char*)tmp, "%s\n%s    value: %s", tmp, (char*)indent_str, tmp_output);
        free(tmp_output);
      }
      else
        sprintf((char*)tmp, "%s\n%s    value: error parse value", (char*)tmp, (char*)indent_str);
    }
    else
    {
      if(pack_val_by_index_as_string(packet, i, tmp_key, tmp_value) == ERROR_NONE)
        sprintf((char*)tmp, "%s\n%s    value: %s", (char*)tmp, (char*)indent_str, tmp_value);
      else
        sprintf((char*)tmp, "%s\n%s    value: error parse value", (char*)tmp, (char*)indent_str);
    }
  }

  return (char*)tmp;
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

    pack_to_buffer(packet, tmp_buffer, &tmp_size);

    #ifdef SOCK_PACK_MODE
    pack_buffer_t tmp;
    bytes_to_hex((unsigned char*)tmp_buffer, (pack_size_t)tmp_size, (unsigned char*)tmp);
    log_add_fmt(LOG_DEBUG, "print_pack, %s",
                (char*)tmp);
    #else
    log_add_fmt(LOG_DEBUG, "print_pack, %s",
                tmp_buffer);
    #endif
  }

  if(pack)
  {
    // TODO: тут выскакивает ошибка, к примеру если подключть 3-х клиентов
    // Я так думаю, что из за большого пакета
//    char *tmp = do_print_pack(packet, 4);
//    log_add(tmp, LOG_INFO);
//    free(tmp);
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
