//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: protocol.c
 */
//==============================================================================
#include <limits.h>
// atoi
#include <stdlib.h>
// memcpy
#include <string.h>

#include "protocol.h"

#include "ncs_log.h"
#include "ncs_pack_utils.h"
#include "ncs_error.h"

#include "exec.h"
//==============================================================================
static int _protocol_id = 0;
//==============================================================================
extern pack_number_t pack_global_number;
extern char *pack_struct_keys[];
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_protocol_t *protocol;
#endif
//==============================================================================
#ifdef PACK_USE_OWN_QUEUE
  int pack_queue_add(pack_number_t number, pack_protocol_t *protocol);
#endif
//------------------------------------------------------------------------------
pack_index_t   _protocol_current_index (pack_type_t out, pack_protocol_t *protocol);
int             protocol_current_buffer(pack_type_t out, pack_buffer_t buffer, pack_size_t *size, pack_protocol_t *protocol);
//------------------------------------------------------------------------------
int             pack_pack_by_number(pack_number_t number, pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol);
pack_packet_t *_pack_pack_by_number(pack_number_t number, pack_type_t out, pack_protocol_t *protocol);
//------------------------------------------------------------------------------
int             pack_pack_by_index(pack_index_t index, pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol);
pack_packet_t *_pack_pack_by_index(pack_index_t index, pack_type_t out, pack_protocol_t *protocol);
//------------------------------------------------------------------------------
int             protocol_next_buffer(pack_buffer_t buffer, pack_size_t *size, pack_protocol_t *protocol);
//------------------------------------------------------------------------------
int is_locked(pack_type_t out, pack_protocol_t *protocol);
int lock     (pack_type_t out, pack_protocol_t *protocol);
int unlock   (pack_type_t out, pack_protocol_t *protocol);
//==============================================================================
int is_locked(pack_type_t out, pack_protocol_t *protocol)
{
  if(out)
    return protocol->out_packets_list.lock_count > 0;
  else
    return protocol->in_packets_list.lock_count > 0;
}
//==============================================================================
int lock(pack_type_t out, pack_protocol_t *protocol)
{
  if(out)
    protocol->out_packets_list.lock_count++;
  else
    protocol->in_packets_list.lock_count++;

  return ERROR_NONE;
}
//==============================================================================
int unlock(pack_type_t out, pack_protocol_t *protocol)
{
  if(out)
  {
    if(protocol->out_packets_list.lock_count > 0)
      protocol->out_packets_list.lock_count--;
  }
  else
  {
    if(protocol->in_packets_list.lock_count > 0)
      protocol->in_packets_list.lock_count--;
  }

  return ERROR_NONE;
}
//==============================================================================
int pack_validation_buffer_init(pack_validation_buffer_t *validation_buffer)
{
  validation_buffer->size = 0;
  memset(validation_buffer->buffer, '\0', PACK_BUFFER_SIZE);

  return ERROR_NONE;
}
//==============================================================================
int pack_in_packets_list_init(pack_in_packets_list_t *in_packets_list)
{
  in_packets_list->empty       = TRUE;
  in_packets_list->index       = PACK_PACKETS_INIT_INDEX;
  in_packets_list->count       = PACK_GLOBAL_INIT_NUMBER;
  in_packets_list->lock_count  = 0;

  for(int i = 0; i < PACK_IN_PACKETS_COUNT; i++)
    pack_init(&in_packets_list->items[i]);

  return ERROR_NONE;
}
//==============================================================================
int pack_out_packets_list_init(pack_out_packets_list_t *out_packets_list)
{
  out_packets_list->empty      = TRUE;
  out_packets_list->index      = PACK_PACKETS_INIT_INDEX;
  out_packets_list->count      = PACK_GLOBAL_INIT_NUMBER;
  out_packets_list->lock_count = 0;

  for(int i = 0; i < PACK_OUT_PACKETS_COUNT; i++)
    pack_init(&out_packets_list->items[i]);

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_QUEUE
int pack_queue_init(pack_queue_t *queue)
{
  queue->empty  = TRUE;
  queue->start  = PACK_QUEUE_INIT_INDEX;
  queue->finish = PACK_QUEUE_INIT_INDEX;

  for(int i = 0; i < PACK_QUEUE_COUNT; i++)
    queue->packets[i] = NULL;

  return ERROR_NONE;
}
#endif
//==============================================================================
int protocol_init(pack_protocol_t *protocol)
{
  protocol->id = _protocol_id++;

  #ifdef PACK_USE_OWN_BUFFER
    protocol = malloc(sizeof(pack_protocol_t));
  #endif

  pack_validation_buffer_init(&protocol->validation_buffer);

  pack_in_packets_list_init(&protocol->in_packets_list);

  pack_out_packets_list_init(&protocol->out_packets_list);

  protocol->on_error            = NULL;
  protocol->on_new_cmd          = NULL;
  protocol->on_new_in_data      = NULL;
  protocol->on_new_out_data     = NULL;

  #ifdef PACK_USE_OWN_QUEUE
  pack_queue_init(&protocol->queue);
  #endif

  return ERROR_NONE;
}
//==============================================================================
pack_index_t _protocol_current_index(pack_type_t out, pack_protocol_t *protocol)
{
  if(out)
    return protocol->out_packets_list.index;
  else
    return protocol->in_packets_list.index;
}
//==============================================================================
int pack_pack_by_number(pack_number_t number, pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol)
{
  if(out)
  {
    if(protocol->out_packets_list.empty)
      pack = 0;
    else
      for(pack_size_t i = 0; i <= PACK_OUT_PACKETS_COUNT; i++)
      {
        if(protocol->out_packets_list.items[i].number == number)
        {
          pack = &protocol->out_packets_list.items[i];
          return ERROR_NONE;
        }
      }
  }
  else
  {
    if(protocol->in_packets_list.empty)
      pack = 0;
    else
      for(pack_size_t i = 0; i <= PACK_IN_PACKETS_COUNT; i++)
      {
        if(protocol->in_packets_list.items[i].number == number)
        {
          pack = &protocol->in_packets_list.items[i];
          return ERROR_NONE;
        }
      }
  }

  return ERROR_NORMAL;
}
//==============================================================================
pack_packet_t *_pack_pack_by_number(pack_number_t number, pack_type_t out, pack_protocol_t *protocol)
{
  if(out)
  {
    if(protocol->out_packets_list.empty)
      return NULL;
    for(pack_size_t i = 0; i <= PACK_OUT_PACKETS_COUNT; i++)
      if(protocol->out_packets_list.items[i].number == number)
        return &protocol->out_packets_list.items[i];
  }
  else
  {
    if(protocol->in_packets_list.empty)
      return NULL;
    for(pack_size_t i = 0; i <= PACK_IN_PACKETS_COUNT; i++)
      if(protocol->in_packets_list.items[i].number == number)
        return &protocol->in_packets_list.items[i];
  }

  return NULL;
}
//==============================================================================
int pack_pack_by_index(pack_index_t index, pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol)
{
  if(out)
  {
    if(protocol->out_packets_list.empty)
      pack = 0;
    else
      pack = &protocol->out_packets_list.items[index];
  }
  else
  {
    if(protocol->in_packets_list.empty)
      pack = 0;
    else
      pack = &protocol->in_packets_list.items[index];
  }

  return ERROR_NONE;
}
//==============================================================================
pack_packet_t *_pack_pack_by_index (pack_index_t index, pack_type_t out, pack_protocol_t *protocol)
{
  if(out)
  {
    if(protocol->out_packets_list.empty)
      return NULL;
    return &protocol->out_packets_list.items[index];
  }
  else
  {
    if(protocol->in_packets_list.empty)
      return NULL;
    return &protocol->in_packets_list.items[index];
  }
}
//==============================================================================
pack_packet_t *_protocol_current_pack(pack_type_t out, pack_protocol_t *protocol)
{
  pack_index_t tmp_index = _protocol_current_index(out, protocol);

  if(out)
  {
    if(protocol->out_packets_list.empty)
      return NULL;
    return &protocol->out_packets_list.items[tmp_index];
  }
  else
  {
    if(protocol->in_packets_list.empty)
      return NULL;
    return &protocol->in_packets_list.items[tmp_index];
  }
}
//==============================================================================
int protocol_begin(pack_protocol_t *protocol, pack_flag_t flag)
{
  if(is_locked(PACK_OUT, protocol))
    return ERROR_NORMAL;
  lock(PACK_OUT, protocol);

  protocol->out_packets_list.count++;
  if(protocol->out_packets_list.count >= USHRT_MAX)
    protocol->out_packets_list.count = PACK_GLOBAL_INIT_NUMBER;

  protocol->out_packets_list.index++;
  if(protocol->out_packets_list.index >= PACK_OUT_PACKETS_COUNT)
    protocol->out_packets_list.index = PACK_PACKETS_INIT_INDEX;

  protocol->out_packets_list.empty = FALSE;

  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  pack_init(tmp_pack);
  pack_set_flag(tmp_pack, flag);

//  if((pack_is_set_flag(tmp_pack, PACK_FLAG_DATA)) && (!pack_is_set_flag(tmp_pack, PACK_FLAG_CMD)))
//    log_add(LOG_INFO, "protocol_begin, data pack");
//  else if((!pack_is_set_flag(tmp_pack, PACK_FLAG_DATA)) && (pack_is_set_flag(tmp_pack, PACK_FLAG_CMD)))
//    log_add(LOG_INFO, "protocol_begin, cmd pack");
//  else
//    log_add(LOG_INFO, "protocol_begin, unknown pack");

//  log_add_fmt(LOG_DEBUG, "protocol_begin, protocol id: %d, packet number: %d, packet index: %d",
//              protocol->id, tmp_pack->number, _protocol_current_index(PACK_OUT, protocol));

  return ERROR_NONE;
}
//==============================================================================
int protocol_end(pack_protocol_t *protocol)
{
  if(!is_locked(PACK_OUT, protocol))
    return ERROR_NORMAL;

  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

//  log_add_fmt(LOG_DEBUG, "protocol_end, protocol id: %d, packet number: %d, packet index: %d",
//              protocol->id, tmp_pack->number, _protocol_current_index(PACK_OUT, protocol));

  #ifdef PACK_USE_OWN_QUEUE
  if(pack_queue_add(tmp_pack->number, protocol) >= ERROR_NORMAL)
    goto error;
  #endif

  if(protocol->on_new_out_data != 0)
    if(protocol->on_new_out_data((void*)protocol, (void*)tmp_pack) >= ERROR_NORMAL)
      goto error;

  unlock(PACK_OUT, protocol);
  return ERROR_NONE;

  error:
  unlock(PACK_OUT, protocol);
  return make_last_error_fmt(ERROR_NORMAL, errno, "protocol_end,\nmessage: %s",
                             last_error()->message);
}
//==============================================================================
#ifdef PACK_USE_OWN_QUEUE
int pack_queue_add(pack_number_t number, pack_protocol_t *protocol)
{
//  log_add_fmt(LOG_DEBUG, "pack_queue_add, protocol id: %d, packet number: %d",
//              protocol->id, number);

  pack_packet_t *tmp_pack = _pack_pack_by_number(number, PACK_OUT, protocol);
  if(tmp_pack == NULL)
    return make_last_error_fmt(ERROR_NORMAL, errno, "pack_queue_add, pack by number(%d) not found",
                               number);

//  log_add_fmt(LOG_DEBUG, "pack_queue_add, pack by number(%d) found",
//              tmp_pack->number);

  pack_queue_t *tmp_queue = &protocol->queue;

  pack_index_t tmp_finish = tmp_queue->finish;

  tmp_queue->packets[tmp_finish] = tmp_pack;

  tmp_queue->empty = FALSE;

  tmp_queue->finish++;
  if(tmp_queue->finish > PACK_QUEUE_COUNT)
    tmp_queue->finish = 0;

  return ERROR_NONE;
}
#endif
//==============================================================================
pack_packet_t *_protocol_next_pack(pack_protocol_t *protocol)
{
  // TODO: Тут нужно вставить ожидание свободности очереди
  if(is_locked(PACK_OUT, protocol))
    return PACK_QUEUE_EMPTY;

  #ifdef PACK_USE_OWN_QUEUE
    pack_queue_t *tmp_queue = &protocol->queue;

    if(tmp_queue->empty)
      return PACK_QUEUE_EMPTY;

    if((tmp_queue->start > PACK_QUEUE_COUNT) || (tmp_queue->finish > PACK_QUEUE_COUNT))
      return PACK_QUEUE_EMPTY;

    pack_index_t tmp_index = tmp_queue->start;

    tmp_queue->start++;
    if(tmp_queue->start > PACK_QUEUE_COUNT)
      tmp_queue->start = 0;

    if(tmp_queue->start == tmp_queue->finish)
      tmp_queue->empty = TRUE;

    pack_packet_t *tmp_pack = tmp_queue->packets[tmp_index];

//    log_add_fmt(LOG_DEBUG, "_protocol_next_pack, protocol id: %d, packet number: %d, queue index: %d",
//                protocol->id, tmp_pack->number, tmp_index);

    return tmp_pack;
  #else
    return _pack_pack_current(PACK_OUT, protocol);
  #endif
}
//==============================================================================
int protocol_next_buffer(pack_buffer_t buffer, pack_size_t *size, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_next_pack(protocol);
  if(tmp_pack != NULL)
  {
    pack_to_buffer_bin(tmp_pack, buffer, size);
    return PACK_QUEUE_FULL;
  }
  else
    return PACK_QUEUE_EMPTY;
}
//==============================================================================
int protocol_add_as_int(pack_key_t key, int value, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  return pack_add_as_int(tmp_pack, key, value);
}
//==============================================================================
int protocol_add_as_float(pack_key_t key, float value, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  return pack_add_as_float(tmp_pack, key, value);
}
//==============================================================================
int protocol_add_as_char(pack_key_t key, char value, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  return pack_add_as_char(tmp_pack, key, value);
}
//==============================================================================
int protocol_add_as_bool(pack_key_t key, BOOL value, pack_protocol_t *protocol)
{
  return ERROR_NONE;
}
//==============================================================================
int protocol_add_as_byte(pack_key_t key, BYTE value, pack_protocol_t *protocol)
{
  return ERROR_NONE;
}
//==============================================================================
int protocol_add_as_string(pack_key_t key, pack_string_t value, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  return pack_add_as_string(tmp_pack, key, value);
}
//==============================================================================
int protocol_add_as_bytes(pack_key_t key, pack_bytes_t value, pack_size_t size, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  return pack_add_as_bytes(tmp_pack, key, value, size, PACK_WORD_BYTES);
}
//==============================================================================
int protocol_add_as_pack(pack_key_t key, pack_packet_t *value, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  return pack_add_as_pack(tmp_pack, key, value);
}
//==============================================================================
int protocol_add_cmd(pack_value_t command, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_CMD_KEY;

  return protocol_add_as_string(tmp_key, (pack_string_t)command, protocol);
}
//==============================================================================
int protocol_add_param_as_int(int param, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;

  return protocol_add_as_int(tmp_key, param, protocol);
}
//==============================================================================
int protocol_add_param_as_float(float param, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;

  return protocol_add_as_float(tmp_key, param, protocol);
}
//==============================================================================
int protocol_add_param_as_char(char param, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;

  return protocol_add_as_char(tmp_key, param, protocol);
}
//==============================================================================
int protocol_add_param_as_bool(BOOL param, pack_protocol_t *protocol)
{
  return ERROR_NONE;
}
//==============================================================================
int protocol_add_param_as_byte(BYTE param, pack_protocol_t *protocol)
{
  return ERROR_NONE;
}
//==============================================================================
int protocol_add_param_as_string(pack_string_t param, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;

  return protocol_add_as_string(tmp_key, param, protocol);
}
//==============================================================================
int protocol_add_param_as_bytes(pack_bytes_t param, pack_size_t size, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;

  return protocol_add_as_bytes(tmp_key, param, size, protocol);
}
//==============================================================================
int protocol_assign_pack(pack_protocol_t *protocol, pack_packet_t *pack)
{
  if(pack == NULL)
    return ERROR_NORMAL;

  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  return pack_assign_pack(tmp_pack, pack);
}
//==============================================================================
int protocol_bin_buffer_validate(pack_buffer_t buffer, pack_size_t size,
  pack_type_t validate, pack_protocol_t *protocol, void *sender)
{
//  log_add(LOG_INFO, "protocol_bin_buffer_validate");

  pack_validation_buffer_t *vbuffer = &protocol->validation_buffer;

  if((vbuffer->size + size) > PACK_BUFFER_SIZE)
  {
    errno = 1;
    return make_last_error_fmt(ERROR_NORMAL, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                    "message: income buffer too big(%d/%d)",
                               errno, (vbuffer->size + size), PACK_BUFFER_SIZE);
  }

  memcpy(&protocol->validation_buffer.buffer[vbuffer->size], buffer, size);
  vbuffer->size += size;

  pack_size_t tmp_remain_size = vbuffer->size;
  int tmp_valid_pack_count = 0;
  while(TRUE)
  {
    if(tmp_remain_size < PACK_VERSION_SIZE)
    {
      errno = 2;
      return make_last_error_fmt(ERROR_WARNING, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                       "message: (on start) empty buffer(%d), valid packs: %d",
                                 errno, tmp_remain_size, tmp_valid_pack_count);
    }

    pack_size_t tmp_pack_pos = 0;

    // Get version
    for(pack_size_t i = 0; i < PACK_VERSION_SIZE; i++)
    {
      if(vbuffer->buffer[tmp_pack_pos++] != PACK_VERSION[i])
      {
        errno = 3;
        return make_last_error_fmt(ERROR_NORMAL, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                        "message: version does not match",
                                   errno);
      }

      if(tmp_remain_size <= 0)
      {
        errno = 4;
        return make_last_error_fmt(ERROR_WARNING, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                         "message: (on version) empty buffer(%d), valid packs: %d",
                                   errno, tmp_remain_size, tmp_valid_pack_count);
      }
      tmp_remain_size--;
    }

    // Get size
    if(tmp_remain_size < PACK_SIZE_SIZE)
    {
      errno = 5;
      return make_last_error_fmt(ERROR_WARNING, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                       "message: (on size) empty buffer(%d), valid packs: %d",
                                 errno, tmp_remain_size, tmp_valid_pack_count);
    }

    tmp_remain_size--;
    pack_size_t tmp_size  = vbuffer->buffer[tmp_pack_pos++] << 8;
    tmp_remain_size--;
    tmp_size             |= vbuffer->buffer[tmp_pack_pos++];

    if(tmp_size == 0)
    {
      errno = 6;
      return make_last_error_fmt(ERROR_NORMAL, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                       "message: value size = %d",
                                 errno, tmp_size);
    }

    // Get value
    pack_buffer_t tmp_value_buffer;
    for(pack_size_t i = 0; i < (tmp_size + PACK_INDEX_SIZE); i++)
    {
      tmp_value_buffer[i] = vbuffer->buffer[tmp_pack_pos++];

      if(tmp_remain_size <= 0)
      {
        errno = 7;
        return make_last_error_fmt(ERROR_WARNING, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                         "message: (on value) empty buffer(%d), valid packs: %d",
                                   errno, tmp_remain_size, tmp_valid_pack_count);
      }
      tmp_remain_size--;
    }

    // Get index
    pack_index_t tmp_index = tmp_value_buffer[0] << 8;
    tmp_index             |= tmp_value_buffer[1];

    // Get crc16 1
    if(tmp_remain_size < PACK_CRC_SIZE)
    {
      errno = 8;
      return make_last_error_fmt(ERROR_WARNING, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                       "message: (on crc) empty buffer(%d), valid packs: %d",
                                 errno, tmp_remain_size, tmp_valid_pack_count);
    }

    tmp_remain_size--;
    pack_crc16_t tmp_crc16_1  = vbuffer->buffer[tmp_pack_pos++] << 8;
    tmp_remain_size--;
    tmp_crc16_1              |= vbuffer->buffer[tmp_pack_pos++];

    // Get crc16 2
    pack_crc16_t tmp_crc16_2 = getCRC16((char *)tmp_value_buffer, (tmp_size + PACK_INDEX_SIZE));

    // Check crc16
    if(tmp_crc16_1 != tmp_crc16_2)
    {
      errno = 9;
      return make_last_error_fmt(ERROR_NORMAL, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                      "message: crc16 does not match",
                                 errno);
    }

    for(pack_size_t j = 0; j < tmp_remain_size; j++)
      vbuffer->buffer[j] = vbuffer->buffer[j + tmp_pack_pos];
    vbuffer->size = tmp_remain_size;

    tmp_valid_pack_count++;

    if(validate == PACK_VALIDATE_ONLY)
    {
      if(tmp_remain_size <= 0)
        break;
      else
      {
        log_add_fmt(LOG_DEBUG, "protocol_bin_buffer_validate, remain: %d", tmp_remain_size);
        continue;
      }
    }

    protocol->in_packets_list.count++;
    if(protocol->in_packets_list.count >= USHRT_MAX)
      protocol->in_packets_list.count = PACK_GLOBAL_INIT_NUMBER;

    protocol->in_packets_list.index++;
    if(protocol->in_packets_list.index >= PACK_IN_PACKETS_COUNT)
      protocol->in_packets_list.index = PACK_PACKETS_INIT_INDEX;

    protocol->in_packets_list.empty = FALSE;

    pack_packet_t *tmp_pack = _protocol_current_pack(PACK_IN, protocol);
    if(tmp_pack == NULL)
    {
      errno = 10;
      return make_last_error_fmt(ERROR_NORMAL, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                      "message: current pack is not available",
                                 errno);
    }

    pack_flag_t f = tmp_pack->flag;
    pack_init(tmp_pack);
    pack_set_flag(tmp_pack, f);

    tmp_pack->number = tmp_index;

    if(pack_buffer_to_words(tmp_value_buffer, tmp_size, tmp_pack->words, &tmp_pack->words_count) == ERROR_NONE)
    {
      if(protocol->on_new_in_data != 0)
      {
//        log_add(LOG_INFO, "protocol_bin_buffer_validate, on_new_in_data");
        if (protocol->on_new_in_data((void*)sender, (void*)tmp_pack) >= ERROR_WARNING)
        {
          errno = 11;
          return make_last_error_fmt(ERROR_NORMAL, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                          "message: %s",
                                     errno, last_error()->message);
        }
      }
    }
    else
    {
      errno = 12;
      return make_last_error_fmt(ERROR_NORMAL, errno, "protocol_bin_buffer_validate, errno: %d,\n" \
                                                      "message: %s",
                                 errno, last_error()->message);
    }

    if(tmp_remain_size <= 0)
      break;
    else
      log_add_fmt(LOG_DEBUG, "protocol_bin_buffer_validate, remain: %d", tmp_remain_size);
  }

  return ERROR_NONE;
}
//==============================================================================
// <"Car_001"|0|0|80974|0.0|0.0|0|0|1|0|-0.46|-3.10|43.2|41.9|4.08|1.000000|2.000000|0.02|1>
// 1. Проверяем наличие < >
// 2. Парсим каждое значение
// 3. Должнно быть PACK_TXT_FORMAT_COUNT
// 4. Должен совпадать xor
//==============================================================================
void get_xor(char *result, char *string, int count)
{
  *result = string[count - 1];
}
//==============================================================================
char *next_token(char *token, char *dst, int *cnt)
{
  if(token != NULL)
  {
    (*cnt)++;
    strcpy(dst, token);
    return strtok(NULL, "&");
  }
  return NULL;
}
//==============================================================================
BOOL buffer_is_cmd(char *buffer)
{
  BOOL result = TRUE;
  for(int i = 0; i < PACK_KEY_SIZE - 1; i++)
    if(buffer[i] != PACK_CMD_KEY[i])
    {
      result = FALSE;
      break;
    }
  return result;
}
//==============================================================================
int protocol_txt_buffer_validate(pack_buffer_t buffer, pack_size_t size,
  pack_type_t validate, pack_protocol_t *protocol, void *sender)
{
  int start = -1;
  int finish = -1;
  for(int i = 0; i < size; i++)
  {
    for(; i < size; i++)
    {
      if(buffer[i] == '<')
        start = i;
      if(buffer[i] == '>')
        finish = i;
      if((start == -1) || (finish == -1))
        continue;
    }

    if(start == -1)
      return make_last_error(ERROR_NORMAL, errno, "protocol_txt_buffer_validate, start not found");

    if(finish == -1)
      return make_last_error(ERROR_NORMAL, errno, "protocol_txt_buffer_validate, finish not found");

    if(start >= finish)
      return make_last_error(ERROR_NORMAL, errno, "protocol_txt_buffer_validate, invalid position of start and finish");

    if(validate == PACK_VALIDATE_STRUCT)
      break;

    // <data|data|data|xor>, xor size = 1 byte
    int tmp_size = finish - start + 1;
    char tmp_buffer[tmp_size];
    memset(tmp_buffer, '\0', tmp_size);

    // data|data|data|xor, ignore < and >
    tmp_size -= 2;
    memcpy(tmp_buffer, (char*)&buffer[start+1], tmp_size);

    char pack_xor = 0xFF;
    get_xor(&pack_xor, (char*)tmp_buffer, tmp_size);

    // data|data|data, for calc xor irnore xor and |
    char tmp_xor = 0xFF;
    calc_xor(&tmp_xor, (char*)tmp_buffer, tmp_size-1);

    if(pack_xor != tmp_xor)
      return make_last_error(ERROR_NORMAL, errno, "protocol_txt_buffer_validate, invalid xor");

    if(validate == PACK_VALIDATE_ONLY)
      break;

    if(buffer_is_cmd(tmp_buffer))
    {
//      if(protocol->on_new_cmd != 0)
//        protocol->on_new_cmd((void*)sender, (void*)tmp_pack);

      char tmp_command[256];
      memset(tmp_command, '\0', 256);
      strncpy(tmp_command, tmp_buffer, tmp_size-2);
      handle_command_ajax(sender, tmp_command);
    }
    else
    {
      int cnt = 0;
      pack_struct_s_t tmp_txt_pack;

      char *token = strtok((char*)&buffer[1], "&");
      token = next_token(token, tmp_txt_pack._ID,             &cnt);
      token = next_token(token, tmp_txt_pack.sGPStime,        &cnt);
      token = next_token(token, tmp_txt_pack.sGPStime_s,      &cnt);
      token = next_token(token, tmp_txt_pack.sTickCount,      &cnt);
      token = next_token(token, tmp_txt_pack.sGPSspeed,       &cnt);
      token = next_token(token, tmp_txt_pack.sGPSheading,     &cnt);
      token = next_token(token, tmp_txt_pack.sGPSlat,         &cnt);
      token = next_token(token, tmp_txt_pack.sGPSlon,         &cnt);
      token = next_token(token, tmp_txt_pack.sint_par1,       &cnt);
      token = next_token(token, tmp_txt_pack.sint_par2,       &cnt);
      token = next_token(token, tmp_txt_pack.sGyro1AngleZ,    &cnt);
      token = next_token(token, tmp_txt_pack.sGyro2AngleZ,    &cnt);
      token = next_token(token, tmp_txt_pack.sMPU1temp,       &cnt);
      token = next_token(token, tmp_txt_pack.sMPU2temp,       &cnt);
      token = next_token(token, tmp_txt_pack.sBatteryVoltage, &cnt);
      token = next_token(token, tmp_txt_pack.sfl_par1,        &cnt);
      token = next_token(token, tmp_txt_pack.sfl_par2,        &cnt);
      token = next_token(token, tmp_txt_pack.sExtVoltage,     &cnt);
      token = next_token(token, tmp_txt_pack.sUSBConnected,   &cnt);  // xor in this token
              next_token(token, tmp_txt_pack.sxor,            &cnt);  // this call just for get last token

      if(cnt != PACK_STRUCT_VAL_COUNT)
        return make_last_error(ERROR_NORMAL, errno, "protocol_txt_buffer_validate, not enough data");

      protocol->in_packets_list.count++;
      if(protocol->in_packets_list.count >= USHRT_MAX)
        protocol->in_packets_list.count = PACK_GLOBAL_INIT_NUMBER;

      protocol->in_packets_list.index++;
      if(protocol->in_packets_list.index >= PACK_IN_PACKETS_COUNT)
        protocol->in_packets_list.index = PACK_PACKETS_INIT_INDEX;

      protocol->in_packets_list.empty = FALSE;

      pack_packet_t *tmp_pack = _protocol_current_pack(PACK_IN, protocol);
      if(tmp_pack == NULL)
        return make_last_error(ERROR_NORMAL, errno, "protocol_txt_buffer_validate, current pack is not available");

      pack_flag_t f = tmp_pack->flag;
      pack_init(tmp_pack);
      pack_set_flag(tmp_pack, f);

      tmp_pack->number = atoi(tmp_txt_pack.sTickCount);

      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[0],  (unsigned char*)tmp_txt_pack._ID);             // 1
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[1],  (unsigned char*)tmp_txt_pack.sGPStime);        // 2
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[2],  (unsigned char*)tmp_txt_pack.sGPStime_s);      // 3
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[3],  (unsigned char*)tmp_txt_pack.sTickCount);      // 4
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[4],  (unsigned char*)tmp_txt_pack.sGPSspeed);       // 5
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[5],  (unsigned char*)tmp_txt_pack.sGPSheading);     // 6
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[6],  (unsigned char*)tmp_txt_pack.sGPSlat);         // 7
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[7],  (unsigned char*)tmp_txt_pack.sGPSlon);         // 8
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[8],  (unsigned char*)tmp_txt_pack.sint_par1);       // 9
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[9],  (unsigned char*)tmp_txt_pack.sint_par2);       // 10
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[10], (unsigned char*)tmp_txt_pack.sGyro1AngleZ);    // 11
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[11], (unsigned char*)tmp_txt_pack.sGyro2AngleZ);    // 12
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[12], (unsigned char*)tmp_txt_pack.sMPU1temp);       // 13
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[13], (unsigned char*)tmp_txt_pack.sMPU2temp);       // 14
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[14], (unsigned char*)tmp_txt_pack.sBatteryVoltage); // 15
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[15], (unsigned char*)tmp_txt_pack.sfl_par1);        // 16
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[16], (unsigned char*)tmp_txt_pack.sfl_par2);        // 17
      pack_add_as_string(tmp_pack, (unsigned char*)pack_struct_keys[17], (unsigned char*)tmp_txt_pack.sExtVoltage);     // 18
      pack_add_as_char  (tmp_pack, (unsigned char*)pack_struct_keys[18],                 tmp_txt_pack.sUSBConnected[0]);// 19
//      pack_add_as_char  (tmp_pack, (unsigned char*)pack_struct_keys[19],                 tmp_txt_pack.sxor[0]);         // 20

      if(protocol->on_new_in_data != 0)
        protocol->on_new_in_data((void*)sender, (void*)tmp_pack);
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int protocol_current_buffer(pack_type_t out, pack_buffer_t buffer, pack_size_t *size, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(out, protocol);

  if(tmp_pack == NULL)
    return PACK_QUEUE_EMPTY;

  if(pack_to_buffer_bin(tmp_pack, buffer, size) == ERROR_NONE)
    return PACK_QUEUE_FULL;
  else
    return PACK_QUEUE_EMPTY;
}
//==============================================================================
