//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
/*
* Binary pack by NIch
* Moscow 2016
* ----
* FullPack(size in bytes)
* 4       2      2       SIZE   2   TOTAL = 6 + 2 + 2 + SIZE + 2 = 12 + SIZE
* V01\0   SIZE   INDEX   DATA   CRC
* -
* CRC = CRC(INDEX + DATA)
* -----
* DATA - WORD1WORD2WORD3...
* -----
* OneWord(size in bytes)
* 4       1      2(char/bytes)   SIZE   TOTAL = 5(7) + SIZE
* KEY\0   TYPE   SIZE            DATA
* -
* INT    - SIZE = EMPTY(sizeof(int))
* FLOAT  - SIZE = EMPTY(sizeof(float))
* STRING - SIZE = COUNT
* BYTES  - SIZE = COUNT
* -----
* OneWordIntFloat(example)
* 4       1   4    TOTAL = 4 + 1 + 4 = 9
* POW\0   1   25
* -----
* OneWordStrBytes(example)
* 4       1   2      3      TOTAL = 4 + 1 + 2 + 3 = 10
* NAM\0   3   0x03   IVA
* -----
* FullPack(example)
* 6         2(9+11)   2      9          11               2    TOTAL = 32
* VER01\0   0x16      0x01   POW\0125   NAM\030x04IVAN   CRC
*/
/*
* b[0] = si & 0xff;
* b[1] = (si >> 8) & 0xff;
* si = (b[0] << 8) | b[1];
*/
//==============================================================================
// malloc
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "ncs_pack.h"

#include "ncs_pack_utils.h"
#include "ncs_error.h"

#ifndef DEMS_DEVICE
  #include "ncs_log.h"
#endif
//==============================================================================
pack_size_t _pack_params_count   (pack_packet_t *pack);
//==============================================================================
int pack_param_by_index_as_string(pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_string_t value);
//==============================================================================
int pack_word_by_key             (pack_packet_t *pack, pack_key_t key,     pack_index_t *index, pack_word_t *word);
int pack_word_by_index           (pack_packet_t *pack, pack_index_t index, pack_key_t key,      pack_word_t *word);
//==============================================================================
int pack_key_by_index            (pack_packet_t *pack, pack_index_t index, pack_key_t key);
//==============================================================================
int pack_val_by_key_as_int       (pack_packet_t *pack, pack_key_t key, pack_index_t *index, int   *value);
int pack_val_by_key_as_float     (pack_packet_t *pack, pack_key_t key, pack_index_t *index, float *value);
int pack_val_by_key_as_string    (pack_packet_t *pack, pack_key_t key, pack_index_t *index, pack_string_t value);
int pack_val_by_key_as_bytes     (pack_packet_t *pack, pack_key_t key, pack_index_t *index, pack_bytes_t value, pack_size_t *size);
//==============================================================================
pack_size_t _pack_words_size     (pack_packet_t *pack);
//==============================================================================
int pack_words_to_buffer         (pack_packet_t *pack, pack_buffer_t buffer, pack_size_t start_index);
//==============================================================================
int pack_word_init               (pack_word_t *word);
//==============================================================================
const char *_pack_word_as_string (pack_word_t *word);
//==============================================================================
int pack_word_to_buffer          (pack_word_t *word, pack_buffer_t buffer, pack_size_t *start_index);
int pack_assign_word             (pack_word_t *dst, pack_word_t *src);
//==============================================================================
int pack_word_as_int             (pack_word_t *word, int   *value);
int pack_word_as_float           (pack_word_t *word, float *value);
int pack_word_as_bytes           (pack_word_t *word, pack_bytes_t value, pack_size_t *size);
//==============================================================================
pack_size_t _pack_word_size      (pack_word_t *word);
//==============================================================================
pack_number_t pack_global_number = PACK_GLOBAL_INIT_NUMBER;
int pack_last_error = ERROR_NONE;
//==============================================================================
const char *_pack_version()
{
  return PACK_VERSION;
}
//==============================================================================
pack_number_t _pack_global_number()
{
  return pack_global_number;
}
//==============================================================================
int _pack_get_last_error()
{
  return pack_last_error;
}
//==============================================================================
pack_packet_t *packet()
{
  return NULL;
}
//==============================================================================
int pack_word_init(pack_word_t *word)
{
  memset(word->key, 0, PACK_KEY_SIZE);
  memset(word->value, 0, PACK_VALUE_SIZE);
  word->type = PACK_WORD_NONE;
  word->size = 0;

  return ERROR_NONE;
}
//==============================================================================
int pack_init(pack_packet_t *packet)
{
  packet->number      = pack_global_number++;
  packet->words_count = 0;

  for(int i = 0; i < PACK_WORDS_COUNT; i++)
    pack_word_init(&packet->words[i]);

  if(pack_global_number >= USHRT_MAX)
    pack_global_number = PACK_GLOBAL_INIT_NUMBER;

  return ERROR_NONE;
}
//==============================================================================
int pack_add_as_int(pack_packet_t *pack, pack_key_t key, int value)
{
  if(pack->words_count >= PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_set_as_int(pack, pack->words_count, key, value);

  pack->words_count++;

  return ERROR_NONE;
}
//==============================================================================
int pack_set_as_int(pack_packet_t *pack, pack_index_t index, pack_key_t key, int value)
{
  if(index >= PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_word_t *tmp_word = &pack->words[index];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = PACK_WORD_INT;

  // Size
  tmp_word->size = sizeof(int);

  // Value
  pack_size_t i = 0;
  tmp_word->value[i++] = (value >> 24) & 0xff;
  tmp_word->value[i++] = (value >> 16) & 0xff;
  tmp_word->value[i++] = (value >> 8 ) & 0xff;
  tmp_word->value[i++] = (value      ) & 0xff;

  return ERROR_NONE;
}
//==============================================================================
int pack_insert_as_int(pack_packet_t *pack, pack_index_t index, pack_key_t key, int value)
{
  return ERROR_NONE;
}
//==============================================================================
int pack_add_as_float(pack_packet_t *pack, pack_key_t key, float value)
{
  if(pack->words_count >= PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_word_t *tmp_word = &pack->words[pack->words_count];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = PACK_WORD_FLOAT;

  // Size
  tmp_word->size = sizeof(float);

  // Value
  floatUnion tmp_value;
  tmp_value.f = value;
  memcpy(tmp_word->value, tmp_value.buff, sizeof(float));

  // Words counter
  pack->words_count++;

  return ERROR_NONE;
}
//==============================================================================
int pack_add_as_string(pack_packet_t *pack, pack_key_t key, pack_string_t value)
{
  pack_size_t tmp_size = strlen((char *)value);

  if(tmp_size >= PACK_VALUE_SIZE)
    return make_last_error_fmt(ERROR_NORMAL, errno, "pack_add_as_bytes, value too big, key: %s, size: %d of %d",
                               key, tmp_size, PACK_VALUE_SIZE);

  if(pack->words_count >= PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_word_t *tmp_word = &pack->words[pack->words_count];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = PACK_WORD_STRING;

  // Size
  tmp_word->size = tmp_size;

  // Value
  memcpy(tmp_word->value, value, tmp_size);

  // Words counter
  pack->words_count++;

  return ERROR_NONE;
}
//==============================================================================
int pack_add_as_bytes(pack_packet_t *pack, pack_key_t key, pack_bytes_t value, pack_size_t size, pack_type_t type)
{
  if(size >= PACK_VALUE_SIZE)
    return make_last_error_fmt(ERROR_NORMAL, errno, "pack_add_as_bytes, value too big, key: %s, size: %d of %d",
                               key, size, PACK_VALUE_SIZE);

  if(pack->words_count >= PACK_WORDS_COUNT)
  {
    return ERROR_NORMAL;
  }

  pack_word_t *tmp_word = &pack->words[pack->words_count];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = type; //PACK_WORD_BYTES PACK_WORD_PACK

  // Size
  tmp_word->size = size;

  // Value
  memcpy(tmp_word->value, value, tmp_word->size);

  // Words counter
  pack->words_count++;

  return ERROR_NONE;
}
//==============================================================================
int pack_add_as_pack(pack_packet_t *pack, pack_key_t key, pack_packet_t *inner_pack)
{
  pack_size_t   tmp_size;
  pack_buffer_t tmp_buffer;
  pack_to_bytes(inner_pack, tmp_buffer, &tmp_size);

  pack_add_as_bytes(pack, key, tmp_buffer, tmp_size, PACK_WORD_PACK);

  return ERROR_NONE;
}
//==============================================================================
int pack_add_cmd(pack_packet_t *pack, const pack_string_t command)
{
  pack_key_t tmp_key = PACK_CMD_KEY;

  pack_add_as_string(pack, tmp_key, command);

  return ERROR_NONE;
}
//==============================================================================
int pack_add_param(pack_packet_t *pack, const pack_string_t param)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;

  pack_add_as_string(pack, tmp_key, param);

  return ERROR_NONE;
}
//==============================================================================
int pack_assign_word(pack_word_t *dst, pack_word_t *src)
{
  if(dst == NULL)
    return ERROR_NORMAL;

  if(src == NULL)
    return ERROR_NORMAL;

  memcpy(dst->key, src->key, PACK_KEY_SIZE);
  dst->size = src->size;
  dst->type = src->type;
  memcpy(dst->value, src->value, PACK_VALUE_SIZE);

  return ERROR_NONE;
}
//==============================================================================
int pack_assign_pack(pack_packet_t *dst, pack_packet_t *src)
{
  if(dst == NULL)
    return ERROR_NORMAL;

  if(src == NULL)
    return ERROR_NORMAL;

  dst->number = src->number;
  dst->words_count = src->words_count;

  for(int i = 0; i < src->words_count; i++)
    pack_assign_word(&dst->words[i], &src->words[i]);

  return ERROR_NONE;
}
//==============================================================================
int pack_word_by_key(pack_packet_t *pack, pack_key_t key, pack_size_t *index, pack_word_t *word)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  for(pack_size_t i = 0; i < pack->words_count; i++)
    if(strcmp((char *)pack->words[i].key, (char *)key) == 0)
    {
      *word = pack->words[i];
      *index = i;
      return ERROR_NONE;
    }

  return ERROR_NORMAL;
}
//==============================================================================
int pack_word_by_index(pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_word_t *word)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  if(pack->words_count <= index)
    return ERROR_NORMAL;

  *word = pack->words[index];
  memcpy(key, word->key, PACK_KEY_SIZE);

  return ERROR_NONE;
}
//==============================================================================
int pack_val_by_key_as_int(pack_packet_t *pack, pack_key_t key, pack_index_t *index, int   *value)
{
  pack_word_t tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == ERROR_NONE)
    return pack_word_as_int(&tmp_word, value);
  else
    return ERROR_NORMAL;
}
//==============================================================================
int pack_val_by_key_as_float(pack_packet_t *pack, pack_key_t key, pack_index_t *index, float *value)
{
  pack_word_t tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == ERROR_NONE)
    return pack_word_as_float(&tmp_word, value);
  else
    return ERROR_NORMAL;
}
//==============================================================================
int pack_val_by_key_as_string(pack_packet_t *pack, pack_key_t key, pack_index_t *index, pack_string_t value)
{
  pack_word_t tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == ERROR_NONE)
    return pack_word_as_string(&tmp_word, value);
  else
    return ERROR_NORMAL;
}
//==============================================================================
int pack_val_by_key_as_bytes(pack_packet_t *pack, pack_key_t key, pack_index_t *index, pack_bytes_t value, pack_size_t *size)
{
  pack_word_t tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == ERROR_NONE)
    return pack_word_as_bytes(&tmp_word, value, size);
  else
    return ERROR_NORMAL;
}
//==============================================================================
int pack_val_by_index_as_int(pack_packet_t *pack, pack_index_t index, pack_key_t key, int *value)
{
  pack_word_t tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == ERROR_NONE)
    return pack_word_as_int(&tmp_word, value);
  else
    return ERROR_NORMAL;
}
//==============================================================================
int pack_val_by_index_as_float(pack_packet_t *pack, pack_index_t index, pack_key_t key, float *value)
{
  pack_word_t tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == ERROR_NONE)
    return pack_word_as_float(&tmp_word, value);
  else
    return ERROR_NORMAL;
}
//==============================================================================
int pack_val_by_index_as_string(pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_string_t value)
{
  pack_word_t tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == ERROR_NONE)
    return pack_word_as_string(&tmp_word, value);
  else
    return ERROR_NORMAL;
}
//==============================================================================
int pack_val_by_index_as_bytes(pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_bytes_t value, pack_size_t *size)
{
  pack_word_t tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == ERROR_NONE)
    return pack_word_as_bytes(&tmp_word, value, size);
  else
    return ERROR_NORMAL;
}
//==============================================================================
int pack_val_by_index_as_pack(pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_packet_t *value)
{
  pack_word_t tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == ERROR_NONE)
    return pack_word_as_pack(&tmp_word, value);
  else
    return ERROR_NORMAL;
}
//==============================================================================
int pack_key_by_index(pack_packet_t *pack, pack_index_t index, pack_key_t key)
{
  if(pack == NULL)
    return ERROR_NORMAL;

  if(pack->words_count > PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  if(pack->words_count <= index)
    return ERROR_NORMAL;

  pack_word_t tmp_word = pack->words[index];

  memcpy(key, tmp_word.key, PACK_KEY_SIZE);

  return ERROR_NONE;
}
//==============================================================================
int pack_keys_to_csv(pack_packet_t *pack, pack_delim_t delimeter, pack_buffer_t buffer)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_size_t tmp_pos = 0;

  buffer[0] = '\0';

  for(pack_size_t i = 0; i < pack->words_count; i++)
  {
    for(pack_size_t j = 0; j < PACK_KEY_SIZE; j++)
      buffer[tmp_pos++] = pack->words[i].key[j];

    buffer[tmp_pos++] = delimeter;
  }

  buffer[tmp_pos] = '\0';

  return ERROR_NONE;
}
//==============================================================================
int pack_values_to_csv(pack_packet_t *pack, pack_delim_t delimeter, pack_buffer_t buffer)
{
  if(pack == NULL)
    return ERROR_NORMAL;

  if(pack->words_count > PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_size_t tmp_pos = 0;

  buffer[0] = '\0';

  pack_value_t valueS;

  for(pack_size_t i = 0; i < pack->words_count; i++)
  {
    pack_word_as_string(&pack->words[i], valueS);

    for(size_t j = 0; j < strlen((char*)valueS); j++)
      buffer[tmp_pos++] = valueS[j];

    buffer[tmp_pos++] = delimeter;
  }

  buffer[tmp_pos] = '\0';

  return ERROR_NONE;
}
//==============================================================================
int pack_words_to_buffer(pack_packet_t *pack, pack_buffer_t buffer, pack_size_t start_index)
{
  for(pack_size_t i = 0; i < pack->words_count; i++)
    pack_word_to_buffer(&pack->words[i], buffer, &start_index);

  return ERROR_NONE;
}
//==============================================================================
int pack_to_buffer(pack_packet_t *pack, pack_buffer_t buffer, pack_size_t *size)
{
  // Version
  memcpy(buffer, (const void*)PACK_VERSION, PACK_VERSION_SIZE);
  pack_size_t tmp_pack_pos = PACK_VERSION_SIZE;

  pack_size_t tmp_packet_size = _pack_words_size(pack);

  // Size
  buffer[tmp_pack_pos++] = (tmp_packet_size >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (tmp_packet_size     ) & 0xff;

  // Index
  buffer[tmp_pack_pos++] = (pack->number >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (pack->number     ) & 0xff;

  // Buffer for calc crc
  pack_buffer_t tmp_buffer;
  tmp_buffer[0] = (pack->number >> 8) & 0xff;
  tmp_buffer[1] = (pack->number     ) & 0xff;
  pack_words_to_buffer(pack, tmp_buffer, PACK_INDEX_SIZE);

  pack_size_t tmp_total_size = (PACK_INDEX_SIZE + tmp_packet_size);

  // Words to buffer
  memcpy(&buffer[tmp_pack_pos], &tmp_buffer[2], tmp_packet_size);
  tmp_pack_pos += tmp_packet_size;

  // CRC16
  pack_crc16_t tmp_crc16 = getCRC16((char *)tmp_buffer, tmp_total_size);
  buffer[tmp_pack_pos++] = (tmp_crc16 >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (tmp_crc16     ) & 0xff;

  // pack_outer_size include PACK_INDEX_SIZE
  *size = PACK_VERSION_SIZE + PACK_SIZE_SIZE + PACK_INDEX_SIZE + tmp_packet_size + PACK_CRC_SIZE;

  return ERROR_NONE;
}
//==============================================================================
int pack_to_bytes(pack_packet_t *pack, pack_buffer_t buffer, pack_size_t *size)
{
  return pack_to_buffer(pack, buffer, size);
}
//==============================================================================
pack_size_t _pack_words_count(pack_packet_t *pack)
{
  return pack->words_count;
}
//==============================================================================
pack_size_t _pack_words_size(pack_packet_t *pack)
{
  pack_size_t tmp_size = 0;

  for(pack_size_t i = 0; i < pack->words_count; i++)
    tmp_size += _pack_word_size(&pack->words[i]);

  return tmp_size;
}
//==============================================================================
int pack_word_as_int(pack_word_t *word, int *value)
{
  // TODO: этот метод не работает
//  bytes_to_int(word->value, value);

  *value = 0;
  for(pack_size_t j = 0; j < word->size; j++)
    *value = (*value << 8) + word->value[j];

  return ERROR_NONE;
}
//==============================================================================
int pack_word_as_float(pack_word_t *word, float *value)
{
  bytes_to_float((unsigned char*)word->value, value);

  return ERROR_NONE;
}
//==============================================================================
const char *_pack_word_as_string(pack_word_t *word)
{
  pack_value_t tmp_value;

  pack_word_as_string(word, tmp_value);

  // TODO: возможна утечка
  char *tmp_string = (char*)malloc(strlen((char*)tmp_value)+1);
  tmp_string[strlen((char*)tmp_value)] = '\0';

  strcpy(tmp_string, (char *)tmp_value);

  return tmp_string;
}
//==============================================================================
int pack_word_as_string(pack_word_t *word, pack_string_t value)
{
  value[0] = '\0';

  switch (word->type)
  {
    case PACK_WORD_NONE:
      break;
    case PACK_WORD_INT:
      {
        int valueI;
        pack_word_as_int(word, &valueI);
        sprintf((char *)value, "%d", valueI);
      }
      break;
    case PACK_WORD_FLOAT:
      {
        float valueF;
        pack_word_as_float(word, &valueF);
        sprintf((char *)value, "%f", valueF);
      }
      break;
    case PACK_WORD_STRING:
      {
        for(pack_size_t j = 0; j < word->size; j++)
          value[j] = word->value[j];
        value[word->size] = '\0';
      }
      break;
    case PACK_WORD_BYTES:
      break;
    case PACK_WORD_PACK:
      break;
    default:
      break;
  }

  return ERROR_NONE;
}
//==============================================================================
int pack_word_as_bytes(pack_word_t *word, pack_bytes_t value, pack_size_t *size)
{
  return ERROR_NORMAL;
}
//==============================================================================
int pack_word_as_pack(pack_word_t *word, pack_packet_t *pack)
{
  if(word->type == PACK_WORD_PACK)
    return pack_buffer_to_pack(word->value, word->size, pack);
  else
    return make_last_error(ERROR_NORMAL, errno, "pack_word_as_pack, word is not inner packet");
}
//==============================================================================
pack_size_t _pack_word_size(pack_word_t *word)
{
  pack_size_t tmp_size = PACK_KEY_SIZE + PACK_TYPE_SIZE;

  pack_type_t tmp_word_type = word->type;

  switch (tmp_word_type)
  {
  case PACK_WORD_NONE:
    tmp_size += 0;
    break;
  case PACK_WORD_INT:
    tmp_size += sizeof(int);
    break;
  case PACK_WORD_FLOAT:
    tmp_size += sizeof(float);
    break;
  case PACK_WORD_STRING:
    tmp_size += PACK_SIZE_SIZE;
    tmp_size += word->size;
    break;
  case PACK_WORD_BYTES:
    tmp_size += PACK_SIZE_SIZE;
    tmp_size += word->size;
    break;
  case PACK_WORD_PACK:
    tmp_size += PACK_SIZE_SIZE;
    tmp_size += word->size;
    break;
  default:
    break;
  }

  return tmp_size;
}
//==============================================================================
int pack_word_to_buffer(pack_word_t *word, pack_buffer_t buffer, pack_size_t *start_index)
{
  pack_size_t tmp_index = *start_index;

  memcpy(&buffer[tmp_index], word->key, PACK_KEY_SIZE);
  tmp_index += PACK_KEY_SIZE;

  buffer[tmp_index++] = word->type;

  switch (word->type)
  {
  case PACK_WORD_NONE:
    break;
  case PACK_WORD_INT:
    break;
  case PACK_WORD_FLOAT:
    break;
  case PACK_WORD_STRING:
    {
      buffer[tmp_index++] = (word->size >> 8) & 0xff;
      buffer[tmp_index++] = (word->size     ) & 0xff;
    }
    break;
  case PACK_WORD_BYTES:
    {
      buffer[tmp_index++] = (word->size >> 8) & 0xff;
      buffer[tmp_index++] = (word->size     ) & 0xff;
    }
    break;
  case PACK_WORD_PACK:
    {
      buffer[tmp_index++] = (word->size >> 8) & 0xff;
      buffer[tmp_index++] = (word->size     ) & 0xff;
    }
    break;
  default:
    break;
  }

  memcpy(&buffer[tmp_index], word->value, word->size);
  tmp_index += word->size;

  *start_index = tmp_index;

  return ERROR_NONE;
}
//==============================================================================
int pack_buffer_to_pack(pack_buffer_t buffer, pack_size_t size, pack_packet_t *pack)
{
  pack_init(pack);

  // TODO: maybe govnocode
  int init_index = PACK_VERSION_SIZE + PACK_SIZE_SIZE;
  int init_size  = size - init_index - PACK_CRC_SIZE;

  return pack_buffer_to_words(&buffer[init_index], init_size, pack->words, &pack->words_count);
}
//==============================================================================
int pack_buffer_to_words(pack_buffer_t buffer, pack_size_t buffer_size, pack_words_t words, pack_size_t *words_count)
{
  pack_size_t tmp_count = 0;

  if(buffer_size < (PACK_KEY_SIZE + PACK_TYPE_SIZE))
    return make_last_error(ERROR_NORMAL, errno, "pack_buffer_to_words, buffer too small");

  // Exclude index
  pack_size_t i = PACK_INDEX_SIZE;
  while(i < buffer_size)
  {
    pack_word_t *tmp_word = &words[tmp_count];

    // Read Key
    memcpy(tmp_word->key, &buffer[i], PACK_KEY_SIZE);
    i += PACK_KEY_SIZE;

    // Read Type
    tmp_word->type = buffer[i++];

    // Read Size
    switch (tmp_word->type)
    {
      case PACK_WORD_NONE:
        tmp_word->size = 0;
        break;
      case PACK_WORD_INT:
        tmp_word->size = sizeof(int);
        break;
      case PACK_WORD_FLOAT:
        tmp_word->size = sizeof(float);
        break;
      case PACK_WORD_STRING:
        {
          tmp_word->size  = buffer[i++] << 8;
          tmp_word->size |= buffer[i++];
        }
        break;
      case PACK_WORD_BYTES:
        {
          tmp_word->size  = buffer[i++] << 8;
          tmp_word->size |= buffer[i++];
        }
        break;
      case PACK_WORD_PACK:
        {
          tmp_word->size  = buffer[i++] << 8;
          tmp_word->size |= buffer[i++];
        }
        break;
      default:
        return make_last_error(ERROR_NORMAL, errno, "pack_buffer_to_words, unknown word type");
    }

    // Read Value
    memcpy(tmp_word->value, &buffer[i], tmp_word->size);
    i += tmp_word->size;

    // Words count
    tmp_count++;
    *words_count = tmp_count;
    if(tmp_count > PACK_WORDS_COUNT)
      return make_last_error(ERROR_NORMAL, errno, "pack_buffer_to_words, words count overflow");
  }

  return ERROR_NONE;
}
//==============================================================================
/*
 * Command pack
 *
 * 0 - Key: "CMD", Value: UserCommand
 * 1 - KEY: "PAR", Value: UserParam1
 * 2 - KEY: "PAR", Value: UserParam2
 * etc
*/
//==============================================================================
BOOL _pack_is_command(pack_packet_t *pack)
{
  pack_count_t tmp_words_count = _pack_words_count(pack);
  if(tmp_words_count >= 1)
  {
    pack_key_t tmp_key;
    if(pack_key_by_index(pack, 0, tmp_key) == ERROR_NONE)
      if(strcmp((char *)tmp_key, PACK_CMD_KEY) == 0)
        return TRUE;
  }

  return FALSE;
}
//==============================================================================
int pack_command(pack_packet_t *pack, pack_value_t command)
{
  if(_pack_is_command(pack))
  {
    pack_key_t  tmp_key;
    pack_value_t valueS;

    if(pack_val_by_index_as_string(pack, 0, tmp_key, valueS) == ERROR_NONE)
    {
      strcpy((char*)command, (const char*)valueS);

      return ERROR_NONE;
    }
    else
      return make_last_error(ERROR_NORMAL, errno, "pack_command, pack_val_by_index_as_string");
  }

  return make_last_error(ERROR_NORMAL, errno, "pack_command, pack is not command");
}
//==============================================================================
pack_size_t _pack_params_count(pack_packet_t *pack)
{
  pack_size_t tmp_words_count = _pack_words_count(pack);
  pack_size_t tmp_params_count = 0;
  pack_key_t  tmp_key;

  for(pack_index_t i = 1; i < tmp_words_count; i++)
  {
    int res = pack_key_by_index(pack, i, tmp_key);
    if(res == ERROR_NONE)
      if(strcmp((char *)tmp_key, PACK_PARAM_KEY) == 0)
        tmp_params_count++;
  }

  return tmp_params_count;
}
//==============================================================================
int pack_next_param(pack_packet_t *pack, pack_index_t *index, pack_string_t value)
{
  pack_size_t tmp_words_count = _pack_words_count(pack);
  pack_key_t  tmp_key;

  for(int i = (int)(*index); i < tmp_words_count; i++)
  {
    if(pack_key_by_index(pack, i, tmp_key) == ERROR_NONE)
    {
      pack_param_by_index_as_string(pack, (pack_index_t)i, tmp_key, value);
      *index = i+1;
      return ERROR_NONE;
    }
  }

  return ERROR_NORMAL;
}
//==============================================================================
pack_string_t _pack_next_param(pack_packet_t *pack, pack_index_t *index)
{
  return NULL;
}
//==============================================================================
int pack_param_by_index_as_string(pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_string_t value)
{
  if(strcmp((char *)key, PACK_PARAM_KEY) == 0)
  {
    return pack_val_by_index_as_string(pack, index, key, value);
  }
  else
    return ERROR_NORMAL;
}
//==============================================================================
