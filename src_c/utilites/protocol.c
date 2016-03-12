//==============================================================================
/*
 * ----
 * todo
 * ----
 * 5. Команда переотправить, получает индекс и добавляет пакет в очередь
 * 6. Критические ошибки выводить в лог
*/
//==============================================================================
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"
#include "ncs_log.h"
#include "ncs_pack_utils.h"
//==============================================================================
extern pack_number_t pack_global_number;
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

  memset(validation_buffer->buffer, 0, PACK_BUFFER_SIZE);

  return ERROR_NONE;
}
//==============================================================================
int pack_in_packets_list_init(pack_in_packets_list_t *in_packets_list)
{
  in_packets_list->empty       = PACK_TRUE;
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
  out_packets_list->empty      = PACK_TRUE;
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
  queue->empty  = PACK_TRUE;
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
  #ifdef PACK_USE_OWN_BUFFER
    protocol = malloc(sizeof(pack_protocol_t));
  #endif

  pack_validation_buffer_init(&protocol->validation_buffer);

  pack_in_packets_list_init(&protocol->in_packets_list);

  pack_out_packets_list_init(&protocol->out_packets_list);

  protocol->on_error            = 0;
  protocol->on_new_in_data      = 0;
  protocol->on_new_out_data     = 0;

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
int protocol_begin(pack_protocol_t *protocol)
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

  protocol->out_packets_list.empty = PACK_FALSE;

  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  pack_init(tmp_pack);

  return ERROR_NONE;
}
//==============================================================================
int protocol_end(pack_protocol_t *protocol)
{
  if(!is_locked(PACK_OUT, protocol))
    return ERROR_NORMAL;

    pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  #ifdef PACK_USE_OWN_QUEUE
    pack_queue_add(tmp_pack->number, protocol);
  #endif

  if(protocol->on_new_out_data != 0)
    protocol->on_new_out_data((void*)protocol, (void*)tmp_pack);

  unlock(PACK_OUT, protocol);

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_QUEUE
int pack_queue_add(pack_number_t number, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _pack_pack_by_number(number, PACK_OUT, protocol);

  if(tmp_pack == NULL)
    return ERROR_NORMAL;

  pack_queue_t *tmp_queue = &protocol->queue;

  pack_index_t tmp_finish = tmp_queue->finish;

  tmp_queue->packets[tmp_finish] = tmp_pack;

  tmp_queue->empty = PACK_FALSE;

  tmp_queue->finish++;
  if(tmp_queue->finish > PACK_QUEUE_COUNT)
    tmp_queue->finish = 0;

  return ERROR_NONE;
}
#endif
//==============================================================================
pack_packet_t *_protocol_next_pack(pack_protocol_t *protocol)
{
  // TODO Тут нужно вставить ожидание свободности очереди
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
      tmp_queue->empty = PACK_TRUE;

    return tmp_queue->packets[tmp_index];
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
    pack_to_buffer(tmp_pack, buffer, size);
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
int protocol_add_as_string(pack_key_t key, pack_string_t value, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  return pack_add_as_string(tmp_pack, key, value);
}
//==============================================================================
int protocol_add_as_bytes (pack_key_t key, pack_bytes_t value, pack_size_t size, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(PACK_OUT, protocol);

  return pack_add_as_bytes(tmp_pack, key, value, size);
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
int protocol_buffer_validate(pack_buffer_t buffer, pack_size_t size,
                         pack_type_t only_validate, pack_protocol_t *protocol,
                         void *sender)
{
//  log_add("pack_validate", LOG_DEBUG);

  pack_validation_buffer_t *vbuffer = &protocol->validation_buffer;

  if((vbuffer->size + size) > PACK_BUFFER_SIZE)
  {
    #ifndef DEMS_DEVICE
    log_add_fmt(LOG_ERROR_CRITICAL, "pack_validate, buffer too big(%d/%d)", (vbuffer->size + size), PACK_BUFFER_SIZE);
    #endif
    return ERROR_NORMAL;
  }

  memcpy(&protocol->validation_buffer.buffer[vbuffer->size], buffer, size);
  vbuffer->size += size;

  pack_size_t tmp_validation_size = vbuffer->size;

  int tmp_valid_count = 0;

  while(1)
  {
    if(vbuffer->size <= 0)
      return tmp_valid_count;

    pack_size_t tmp_pack_pos = 0;

    // Get version
    for(pack_size_t i = 0; i < PACK_VERSION_SIZE; i++)
    {
      if(vbuffer->buffer[tmp_pack_pos++] != PACK_VERSION[i])
        return tmp_valid_count;

      tmp_validation_size--;
      if(tmp_validation_size <= 0)
        return tmp_valid_count;
    }

    // Get size
    if(tmp_validation_size < 2)
      return tmp_valid_count;

    pack_size_t tmp_size = vbuffer->buffer[tmp_pack_pos++] << 8;
    tmp_size          |= vbuffer->buffer[tmp_pack_pos++];

    // Get value
    pack_buffer_t tmp_value_buffer;
    for(pack_size_t i = 0; i < (tmp_size + PACK_INDEX_SIZE); i++)
    {
      tmp_value_buffer[i] = vbuffer->buffer[tmp_pack_pos++];

      tmp_validation_size--;
      if(tmp_validation_size <= 0)
        return tmp_valid_count;
    }

    // Get index
    pack_index_t tmp_index = tmp_value_buffer[0] << 8;
    tmp_index           |= tmp_value_buffer[1];

    // Get crc16 1
    if(tmp_validation_size < 2)
      return tmp_valid_count;

    pack_crc16_t tmp_crc16_1 = vbuffer->buffer[tmp_pack_pos++] << 8;
    tmp_crc16_1           |= vbuffer->buffer[tmp_pack_pos++];

    // Get crc16 2
    pack_crc16_t tmp_crc16_2 = getCRC16((char *)tmp_value_buffer, (tmp_size + PACK_INDEX_SIZE));

    // Check crc16
    if(tmp_crc16_1 != tmp_crc16_2)
      return tmp_valid_count;

    pack_size_t tmp_remain_count = vbuffer->size - tmp_pack_pos;
    for(pack_size_t j = 0; j < tmp_remain_count; j++)
      vbuffer->buffer[j] = vbuffer->buffer[j + tmp_pack_pos];
    vbuffer->size = tmp_remain_count;

    tmp_valid_count++;

    if(only_validate)
      continue;

    protocol->in_packets_list.count++;
    if(protocol->in_packets_list.count >= USHRT_MAX)
      protocol->in_packets_list.count = PACK_GLOBAL_INIT_NUMBER;

    protocol->in_packets_list.index++;
    if(protocol->in_packets_list.index >= PACK_IN_PACKETS_COUNT)
      protocol->in_packets_list.index = PACK_PACKETS_INIT_INDEX;

    protocol->in_packets_list.empty = PACK_FALSE;

    pack_packet_t *tmp_pack = _protocol_current_pack(PACK_IN, protocol);

    if(tmp_pack == NULL)
      return ERROR_NORMAL;

    tmp_pack->number = tmp_index;

    if(pack_buffer_to_words(tmp_value_buffer, tmp_size, tmp_pack->words, &tmp_pack->words_count) == ERROR_NONE)
    {
      if(protocol->on_new_in_data != 0)
        protocol->on_new_in_data((void*)sender, (void*)tmp_pack);
    }
  }

  return tmp_valid_count;
}
//==============================================================================
int protocol_current_buffer(pack_type_t out, pack_buffer_t buffer, pack_size_t *size, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _protocol_current_pack(out, protocol);

  if(tmp_pack == NULL)
    return PACK_QUEUE_EMPTY;

  if(pack_to_buffer(tmp_pack, buffer, size) == ERROR_NONE)
    return PACK_QUEUE_FULL;
  else
    return PACK_QUEUE_EMPTY;
}
//==============================================================================
