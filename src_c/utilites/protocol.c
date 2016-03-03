/*
* Binary pack by NIch
* Moscow 2016
* ----
* FullPack(size in bytes)
* 6         2      2       SIZE   2   TOTAL = 6 + 2 + 2 + SIZE + 2 = 12 + SIZE
* VER01\0   SIZE   INDEX   DATA   CRC
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
//==============================================================================
/*
 * ----
 * todo
 * ----
 * 5. Команда переотправить, получает индекс и добавляет пакет в очередь
 * 6. Критические ошибки выводить в лог
*/
//==============================================================================
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include "defines.h"
#include "protocol.h"
#include "ncs_log.h"
#include "ncs_error.h"
#include "protocol_utils.h"
//==============================================================================
static pack_number_t pack_last_error    = ERROR_NONE;
static pack_number_t pack_global_number = PACK_GLOBAL_INIT_NUMBER;
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_protocol *protocol;
#endif
//==============================================================================
#ifdef PACK_USE_OWN_QUEUE
#ifdef PACK_USE_OWN_BUFFER
int pack_queue_add(pack_number number);
#else
int pack_queue_add(pack_number_t number, pack_protocol_t *protocol);
#endif
#endif
//==============================================================================
int pack_key_by_index (pack_packet_t *pack, pack_index_t index, pack_key_t key);
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int is_locked(pack_type out);
int lock(pack_type out);
int unlock(pack_type out);
pack_index _pack_current_index(pack_type out);
int pack_pack_by_number(pack_number number, pack_type out, pack_packet *pack);
pack_packet *_pack_pack_by_number(pack_number number, pack_type out);
int pack_pack_by_index (pack_index index,   pack_type out, pack_packet *pack);
pack_packet *_pack_pack_by_index (pack_index index, pack_type out);
#else
int is_locked(pack_type_t out, pack_protocol_t *protocol);
int lock(pack_type_t out, pack_protocol_t *protocol);
int unlock(pack_type_t out, pack_protocol_t *protocol);
pack_index_t _pack_current_index(pack_type_t out, pack_protocol_t *protocol);
int pack_pack_by_number(pack_number_t number, pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol);
pack_packet_t *_pack_pack_by_number(pack_number_t number, pack_type_t out, pack_protocol_t *protocol);
int pack_pack_by_index(pack_index_t index, pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol);
pack_packet_t *_pack_pack_by_index (pack_index_t index, pack_type_t out, pack_protocol_t *protocol);
#endif
//==============================================================================
int pack_buffer_to_words(pack_buffer_t buffer, pack_size_t buffer_size, pack_words_t words, pack_size_t *words_count);
//==============================================================================
int pack_word_to_buffer  (pack_word_t *word,     pack_buffer_t buffer, pack_size_t *start_index);
int pack_words_to_buffer (pack_packet_t *pack,   pack_buffer_t buffer, pack_size_t start_index);
int pack_packet_to_buffer(pack_packet_t *packet, pack_buffer_t buffer, pack_size_t *size);
//==============================================================================
int pack_word_as_int    (pack_word_t *word, int   *value);
int pack_word_as_float  (pack_word_t *word, float *value);
int pack_word_as_string (pack_word_t *word, pack_string_t value);
int pack_word_as_bytes  (pack_word_t *word, pack_bytes_t value, pack_size_t *size);
//==============================================================================
pack_size_t _pack_words_size(pack_packet_t *pack);
pack_size_t _pack_word_size(pack_word_t *word);
//==============================================================================
//==============================================================================
int pack_get_last_error()
{
  return pack_last_error;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int is_locked(pack_type out)
#else
int is_locked(pack_type_t out, pack_protocol_t *protocol)
#endif
{
  if(out)
    return protocol->out_packets_list.lock_count > 0;
  else
    return protocol->in_packets_list.lock_count > 0;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int lock(pack_type out)
#else
int lock(pack_type_t out, pack_protocol_t *protocol)
#endif
{
  if(out)
    protocol->out_packets_list.lock_count++;
  else
    protocol->in_packets_list.lock_count++;

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int unlock(pack_type out)
#else
int unlock(pack_type_t out, pack_protocol_t *protocol)
#endif
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
}
//==============================================================================
int pack_word_init(pack_word_t *word)
{
  memset(word->key, 0, PACK_KEY_SIZE);
  memset(word->value, 0, PACK_VALUE_SIZE);
  word->type = PACK_WORD_NONE;
  word->size = 0;
}
//==============================================================================
int pack_packet_init(pack_packet_t *packet)
{
  packet->number      = pack_global_number;
  packet->words_count = 0;

  for(int i = 0; i < PACK_WORDS_COUNT; i++)
    pack_word_init(&packet->words[i]);
}
//==============================================================================
int pack_in_packets_list_init(pack_in_packets_list_t *in_packets_list)
{
  in_packets_list->empty       = PACK_TRUE;
  in_packets_list->index       = PACK_PACKETS_INIT_INDEX;
  in_packets_list->count       = PACK_GLOBAL_INIT_NUMBER;
  in_packets_list->lock_count  = 0;

  for(int i = 0; i < PACK_IN_PACKETS_COUNT; i++)
    pack_packet_init(&in_packets_list->items[i]);
}
//==============================================================================
int pack_out_packets_list_init(pack_out_packets_list_t *out_packets_list)
{
  out_packets_list->empty      = PACK_TRUE;
  out_packets_list->index      = PACK_PACKETS_INIT_INDEX;
  out_packets_list->count      = PACK_GLOBAL_INIT_NUMBER;
  out_packets_list->lock_count = 0;

  for(int i = 0; i < PACK_OUT_PACKETS_COUNT; i++)
    pack_packet_init(&out_packets_list->items[i]);
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
}
#endif
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_protocol_init()
#else
int pack_protocol_init(pack_protocol_t *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
    protocol = malloc(sizeof(pack_protocol));
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
#ifdef PACK_USE_OWN_BUFFER
pack_index _pack_current_index(pack_type out)
#else
pack_index_t _pack_current_index(pack_type_t out, pack_protocol_t *protocol)
#endif
{
  if(out)
    return protocol->out_packets_list.index;
  else
    return protocol->in_packets_list.index;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_pack_by_number(pack_number number, pack_type out, pack_packet *pack)
#else
int pack_pack_by_number(pack_number_t number, pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol)
#endif
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
      };
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
      };
  }

  return ERROR_NORMAL;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_packet *_pack_pack_by_number(pack_number number, pack_type out)
#else
pack_packet_t *_pack_pack_by_number(pack_number_t number, pack_type_t out, pack_protocol_t *protocol)
#endif
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
  };

  return NULL;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_pack_by_index(pack_index index, pack_type out, pack_packet *pack)
#else
int pack_pack_by_index(pack_index_t index, pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol)
#endif
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
  };

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_packet *_pack_pack_by_index (pack_index index, pack_type out)
#else
pack_packet_t *_pack_pack_by_index (pack_index_t index, pack_type_t out, pack_protocol_t *protocol)
#endif
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
#ifdef PACK_USE_OWN_BUFFER
int pack_pack_current(pack_type out, pack_packet *pack)
{
  pack_index tmp_index = _pack_current_index(out);

  pack_pack_by_index(tmp_index, out, pack);

  return ERROR_NONE;
}
#else
int pack_pack_current(pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol)
{
  pack_index_t tmp_index = _pack_current_index(out, protocol);

  pack_pack_by_index(tmp_index, out, pack, protocol);

  return ERROR_NONE;
}
#endif
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_packet *_pack_pack_current(pack_type out)
{
  pack_index tmp_index = _pack_current_index(out);

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
#else
pack_packet_t *_pack_pack_current(pack_type_t out, pack_protocol_t *protocol)
{
  pack_index_t tmp_index = _pack_current_index(out, protocol);

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
#endif
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
    };

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
int pack_word_as_int(pack_word_t *word, int *value)
{
  *value = 0;

  for(pack_size_t j = 0; j < word->size; j++)
    *value = (*value << 8) + word->value[j];

  return ERROR_NONE;
}
//==============================================================================
int pack_word_as_float(pack_word_t *word, float *value)
{
  *value = 0.0;

  floatUnion tmp_value;
  for(pack_size_t j = 0; j < word->size; j++)
    tmp_value.buff[j] = word->value[j];
  *value = tmp_value.f;

  return ERROR_NONE;
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
int pack_keys_to_csv(pack_packet_t *pack, unsigned char delimeter, pack_buffer_t buffer)
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
int pack_values_to_csv(pack_packet_t *pack, unsigned char delimeter, pack_buffer_t buffer)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_size_t tmp_pos = 0;

  buffer[0] = '\0';

  pack_value_t valueS;

  for(pack_size_t i = 0; i < pack->words_count; i++)
  {
    pack_word_as_string(&pack->words[i], valueS);

    for(pack_size_t j = 0; j < pack->words[i].size; j++)
      buffer[tmp_pos++] = valueS[j];

    buffer[tmp_pos++] = delimeter;
  }

  buffer[tmp_pos] = '\0';

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_begin()
#else
int pack_begin(pack_protocol_t *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
    if(is_locked(PACK_OUT))
      return ERROR_NORMAL;
    lock(PACK_OUT);
  #else
    if(is_locked(PACK_OUT, protocol))
      return ERROR_NORMAL;
    lock(PACK_OUT, protocol);
  #endif

  pack_global_number++;
  if(pack_global_number >= USHRT_MAX)
    pack_global_number = PACK_GLOBAL_INIT_NUMBER;

  protocol->out_packets_list.count++;
  if(protocol->out_packets_list.count >= USHRT_MAX)
    protocol->out_packets_list.count = PACK_GLOBAL_INIT_NUMBER;

  protocol->out_packets_list.index++;
  if(protocol->out_packets_list.index >= PACK_OUT_PACKETS_COUNT)
    protocol->out_packets_list.index = PACK_PACKETS_INIT_INDEX;

  protocol->out_packets_list.empty = PACK_FALSE;

  #ifdef PACK_USE_OWN_BUFFER
    pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
    pack_packet_t *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  pack_packet_init(tmp_pack);

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_end()
#else
int pack_end(pack_protocol_t *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
    if(!is_locked(PACK_OUT))
      return ERROR_NORMAL;

    pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
    if(!is_locked(PACK_OUT, protocol))
      return ERROR_NORMAL;

    pack_packet_t *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  #ifdef PACK_USE_OWN_QUEUE
    #ifdef PACK_USE_OWN_BUFFER
      pack_queue_add(tmp_pack->number);
    #else
      pack_queue_add(tmp_pack->number, protocol);
    #endif
  #endif

  #ifdef PACK_USE_OWN_BUFFER
    unlock(PACK_OUT);
  #else
    if(protocol->on_new_out_data != 0)
      protocol->on_new_out_data((void*)protocol, (void*)tmp_pack);

    unlock(PACK_OUT, protocol);
  #endif

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_QUEUE
#ifdef PACK_USE_OWN_BUFFER
int pack_queue_add(pack_number number)
#else
int pack_queue_add(pack_number_t number, pack_protocol_t *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
    pack_packet *tmp_pack = _pack_pack_by_number(number, PACK_OUT);
  #else
    pack_packet_t *tmp_pack = _pack_pack_by_number(number, PACK_OUT, protocol);
  #endif

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
#ifdef PACK_USE_OWN_BUFFER
pack_packet *_pack_next()
#else
pack_packet_t *_pack_next(pack_protocol_t *protocol)
#endif
{
  // TODO Тут нужно вставить ожидание свободности очереди
  #ifdef PACK_USE_OWN_BUFFER
    if(is_locked(PACK_OUT))
      return PACK_QUEUE_EMPTY;
  #else
    if(is_locked(PACK_OUT, protocol))
      return PACK_QUEUE_EMPTY;
  #endif

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
    #ifdef PACK_USE_OWN_BUFFER
      return _pack_pack_current(PACK_OUT);
    #else
      return _pack_pack_current(PACK_OUT, protocol);
    #endif
  #endif
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_next_buffer(pack_buffer buffer, pack_size *size)
{
  pack_packet *tmp_pack = _pack_next(protocol);
#else
int pack_next_buffer(pack_buffer_t buffer, pack_size_t *size, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _pack_next(protocol);
#endif
  if(tmp_pack != NULL)
  {
    pack_packet_to_buffer(tmp_pack, buffer, size);
    return PACK_QUEUE_FULL;
  }
  else
    return PACK_QUEUE_EMPTY;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_as_int(pack_key key, int value)
#else
int pack_add_as_int(pack_key_t key, int value, pack_protocol_t *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
    pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
    pack_packet_t *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_word_t *tmp_word = &tmp_pack->words[tmp_pack->words_count];

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

  // Words counter
  tmp_pack->words_count++;

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_as_float(pack_key key, float value)
#else
int pack_add_as_float(pack_key_t key, float value, pack_protocol_t *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
    pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
    pack_packet_t *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_word_t *tmp_word = &tmp_pack->words[tmp_pack->words_count];

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
  tmp_pack->words_count++;

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_as_string(pack_key key, pack_string value)
#else
int pack_add_as_string(pack_key_t key, pack_string_t value, pack_protocol_t *protocol)
#endif
{
  pack_size_t tmp_size = strlen((char *)value);

  if(tmp_size >= PACK_VALUE_SIZE)
  {
    char tmp[256];
    sprintf(tmp, "pack_add_as_string, value too big, value: %s", value);
    log_add(tmp, LOG_CRITICAL_ERROR);
    return ERROR_NORMAL;
  };

  #ifdef PACK_USE_OWN_BUFFER
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
  pack_packet_t *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_word_t *tmp_word = &tmp_pack->words[tmp_pack->words_count];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = PACK_WORD_STRING;

  // Size
  tmp_word->size = tmp_size;

  // Value
  memcpy(tmp_word->value, value, tmp_size);

  // Words counter
  tmp_pack->words_count++;

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_as_bytes (pack_key key, pack_bytes value, pack_size size)
#else
int pack_add_as_bytes (pack_key_t key, pack_bytes_t value, pack_size_t size, pack_protocol_t *protocol)
#endif
{
  if(size >= PACK_VALUE_SIZE)
  {
    char tmp[256];
    sprintf(tmp, "pack_add_as_bytes, value too big, value: %s", value);
    log_add(tmp, LOG_CRITICAL_ERROR);
    return ERROR_NORMAL;
  };

  #ifdef PACK_USE_OWN_BUFFER
    pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
    pack_packet_t *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return ERROR_NORMAL;

  pack_word_t *tmp_word = &tmp_pack->words[tmp_pack->words_count];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = PACK_WORD_BYTES;

  // Size
  tmp_word->size = size;

  // Value
  memcpy(tmp_word->value, value, tmp_word->size);

  // Words counter
  tmp_pack->words_count++;

  return ERROR_NONE;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_cmd(pack_value command)
{
  pack_key tmp_key = PACK_CMD_KEY;
  return pack_add_as_string(tmp_key, (pack_string)command);
}
#else
int pack_add_cmd(pack_value_t command, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_CMD_KEY;
  return pack_add_as_string(tmp_key, (pack_string_t)command, protocol);
}
#endif
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_param_as_int(int param)
{
  pack_key tmp_key = PACK_PARAM_KEY;
  return pack_add_as_int(tmp_key, param);
}
#else
int pack_add_param_as_int(int param, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;
  return pack_add_as_int(tmp_key, param, protocol);
}
#endif
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_param_as_float(float param)
{
  pack_key tmp_key = PACK_PARAM_KEY;
  return pack_add_as_float(tmp_key, param);
}
#else
int pack_add_param_as_float(float param, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;
  return pack_add_as_float(tmp_key, param, protocol);
}
#endif
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_param_as_string(pack_string param)
{
  pack_key tmp_key = PACK_PARAM_KEY;
  return pack_add_as_string(tmp_key, param);
}
#else
int pack_add_param_as_string(pack_string_t param, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;
  return pack_add_as_string(tmp_key, param, protocol);
}
#endif
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_param_as_bytes (pack_bytes param, pack_size size)
{
  pack_key tmp_key = PACK_PARAM_KEY;
  return pack_add_as_bytes(tmp_key, param, size);
}
#else
int pack_add_param_as_bytes (pack_bytes_t param, pack_size_t size, pack_protocol_t *protocol)
{
  pack_key_t tmp_key = PACK_PARAM_KEY;
  return pack_add_as_bytes(tmp_key, param, size, protocol);
}
#endif
//==============================================================================
/*
 * TODO
 * 1. Проверить флаг only_validate, если буфер валидации содержит несколько пакетов, что будет с буфером
 * 2. Если прошел хоть один цикл валидации, а лучше, если в буфере осталось меньше пакета(как проверить хз),
 *    то не возвращать ошибку валидации, все же отработало
 * 3. Если в буфере валидации, несколько пакетов, то флаг only_validate не даст проверить остальные
 * 4.
*/
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_buffer_validate(pack_buffer buffer, pack_size size, pack_type only_validate)
#else
int pack_buffer_validate(pack_buffer_t buffer, pack_size_t size, pack_type_t only_validate, pack_protocol_t *protocol)
#endif
{
//  log_add("pack_validate", LOG_INFO);

  pack_validation_buffer_t *vbuffer = &protocol->validation_buffer;

  if((vbuffer->size + size) > PACK_BUFFER_SIZE)
  {
    char tmp[256];
    sprintf(tmp, "pack_validate, buffer too big(%d/%d)", (vbuffer->size + size), PACK_BUFFER_SIZE);
    log_add(tmp, LOG_CRITICAL_ERROR);
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
    };

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
    };

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

    #ifdef PACK_USE_OWN_BUFFER
      pack_packet *tmp_pack = _pack_pack_current(PACK_IN);
    #else
      pack_packet_t *tmp_pack = _pack_pack_current(PACK_IN, protocol);
    #endif

    if(tmp_pack == NULL)
      return ERROR_NORMAL;

    tmp_pack->number = tmp_index;

    if(pack_buffer_to_words(tmp_value_buffer, tmp_size, tmp_pack->words, &tmp_pack->words_count) == ERROR_NONE)
    {
      if(protocol->on_new_in_data != 0)
        protocol->on_new_in_data((void*)protocol, (void*)tmp_pack);
    };
  };

  return tmp_valid_count;
}
//==============================================================================
int pack_buffer_to_words(pack_buffer_t buffer, pack_size_t buffer_size, pack_words_t words, pack_size_t *words_count)
{
  pack_size_t tmp_count = 0;

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
      default:
        break;
    }

    // Read Value
    memcpy(tmp_word->value, &buffer[i], tmp_word->size);
    i += tmp_word->size;

    // Words count
    tmp_count++;
    *words_count = tmp_count;
    if(tmp_count > PACK_WORDS_COUNT)
      return ERROR_NORMAL;
  }

  return ERROR_NONE;
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
  default:
    break;
  }

  memcpy(&buffer[tmp_index], word->value, word->size);
  tmp_index += word->size;

  *start_index = tmp_index;

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
#ifdef PACK_USE_OWN_BUFFER
int pack_current_packet_to_buffer(pack_type out, pack_buffer buffer, pack_size *size)
{
  pack_packet *tmp_pack = _pack_pack_current(out);

  if(tmp_pack == NULL)
    return PACK_QUEUE_EMPTY;

  if(pack_packet_to_buffer(tmp_pack, buffer, size) == ERROR_NONE)
    return PACK_QUEUE_FULL;
  else
    return PACK_QUEUE_EMPTY;
}
#else
int pack_current_packet_to_buffer(pack_type_t out, pack_buffer_t buffer, pack_size_t *size, pack_protocol_t *protocol)
{
  pack_packet_t *tmp_pack = _pack_pack_current(out, protocol);

  if(tmp_pack == NULL)
    return PACK_QUEUE_EMPTY;

  if(pack_packet_to_buffer(tmp_pack, buffer, size) == ERROR_NONE)
    return PACK_QUEUE_FULL;
  else
    return PACK_QUEUE_EMPTY;
}
#endif
//==============================================================================
int pack_packet_to_buffer(pack_packet_t *packet, pack_buffer_t buffer, pack_size_t *size)
{
//  char *buffer = (char*)malloc(PACK_VERSION_SIZE);

  // Version
  memcpy(buffer, (const void*)PACK_VERSION, PACK_VERSION_SIZE);
  pack_size_t tmp_pack_pos = PACK_VERSION_SIZE;

  pack_size_t tmp_packet_size = _pack_words_size(packet);

  // Size
  buffer[tmp_pack_pos++] = (tmp_packet_size >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (tmp_packet_size     ) & 0xff;

  // Index
  buffer[tmp_pack_pos++] = (packet->number >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (packet->number     ) & 0xff;

  // Buffer for calc crc
  pack_buffer_t tmp_buffer;
  tmp_buffer[0] = (packet->number >> 8) & 0xff;
  tmp_buffer[1] = (packet->number     ) & 0xff;
  pack_words_to_buffer(packet, tmp_buffer, PACK_INDEX_SIZE);

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
int pack_packet_to_json(pack_packet_t *packet, pack_buffer_t buffer, pack_size_t *size)
{
  return ERROR_NONE;
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
  default:
    break;
  }

  return tmp_size;
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
int pack_command(pack_packet_t *pack, pack_value_t command)
{
  pack_count_t tmp_words_count = _pack_words_count(pack);
  if(tmp_words_count >= 1)
  {
    pack_key_t  tmp_key;
    pack_value_t valueS;

    int res = pack_val_by_index_as_string(pack, 0, tmp_key, valueS);
    if(res == ERROR_NONE)
      if(strcmp((char *)tmp_key, PACK_CMD_KEY) == 0)
      {
        strcpy((char*)command, (const char*)valueS);
        return ERROR_NONE;
      };
  };

  return ERROR_NORMAL;
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
int pack_param_by_index_as_string(pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_string_t value)
{
  return pack_val_by_index_as_string(pack, index, key, value);
}
//==============================================================================
