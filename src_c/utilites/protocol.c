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
* 4       1   2      4      TOTAL = 4 + 1 + 2 + 4 = 11
* NAM\0   3   0x04   IVAN
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
 * 1. Побайтный парсинг входного буфера и добавление в список полученных пакетов
 * 2. !Очередь пакетов на отправку
 * 3. !Список созданных пакетов(на случай переотправки)
 * 4. !Список полученных пакетов
 * 5. Команда переотправить, получает индекс и добавляет пакет в очередь
 * 6. !Отправка идет только из очереди
 * 7. Списки в клиенте по одному, в сервере на каждого клиента
*/
//==============================================================================
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include "defines.h"
#include "protocol.h"
#include "log.h"
#include "protocol_utils.h"
//==============================================================================
static pack_number pack_last_error    = PACK_OK;
static pack_number pack_global_number = PACK_GLOBAL_INIT_NUMBER;
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_protocol *protocol;
#endif
//==============================================================================
#ifdef PACK_USE_OWN_QUEUE
pack_queue queue;
#ifdef PACK_USE_OWN_BUFFER
int pack_queue_add(pack_number number);
#else
int pack_queue_add(pack_number number, pack_protocol *protocol);
#endif
#endif
//==============================================================================
int pack_key_by_index (pack_packet *pack, pack_index index, pack_key key);
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
int is_locked(pack_type out, pack_protocol *protocol);
int lock(pack_type out, pack_protocol *protocol);
int unlock(pack_type out, pack_protocol *protocol);
pack_index _pack_current_index(pack_type out, pack_protocol *protocol);
int pack_pack_by_number(pack_number number, pack_type out, pack_packet *pack, pack_protocol *protocol);
pack_packet *_pack_pack_by_number(pack_number number, pack_type out, pack_protocol *protocol);
int pack_pack_by_index(pack_index index, pack_type out, pack_packet *pack, pack_protocol *protocol);
pack_packet *_pack_pack_by_index (pack_index index, pack_type out, pack_protocol *protocol);
#endif
//==============================================================================
int pack_buffer_to_words(pack_buffer buffer, pack_size buffer_size, pack_words words, pack_size *words_count);
//==============================================================================
int pack_word_to_buffer  (pack_word *word,     pack_buffer buffer, pack_size *start_index);
int pack_words_to_buffer (pack_packet *pack,   pack_buffer buffer, pack_size start_index);
int pack_packet_to_buffer(pack_packet *packet, pack_buffer buffer, pack_size *size);
//==============================================================================
int pack_word_as_int    (pack_word *word, int   *value);
int pack_word_as_float  (pack_word *word, float *value);
int pack_word_as_string (pack_word *word, pack_string value);
int pack_word_as_bytes  (pack_word *word, pack_bytes value, pack_size *size);
//==============================================================================
pack_size _pack_words_size(pack_packet *pack);
pack_size _pack_word_size(pack_word *word);
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
int is_locked(pack_type out, pack_protocol *protocol)
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
int lock(pack_type out, pack_protocol *protocol)
#endif
{
  if(out)
    protocol->out_packets_list.lock_count++;
  else
    protocol->in_packets_list.lock_count++;

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int unlock(pack_type out)
#else
int unlock(pack_type out, pack_protocol *protocol)
#endif
{
  if(out)
  {
    protocol->out_packets_list.lock_count--;
    if(protocol->out_packets_list.lock_count < 0)
      protocol->out_packets_list.lock_count = 0;
  }
  else
  {
    protocol->in_packets_list.lock_count--;
    if(protocol->in_packets_list.lock_count < 0)
      protocol->in_packets_list.lock_count = 0;
  }

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_protocol_init()
#else
int pack_protocol_init(pack_protocol *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
  protocol = malloc(sizeof(pack_protocol));
  #endif

  protocol->validation_buffer.size      = 0;

  protocol->in_packets_list.empty       = PACK_TRUE;
  protocol->in_packets_list.index       = PACK_PACKETS_INIT_INDEX;
  protocol->in_packets_list.lock_count  = 0;

  protocol->out_packets_list.empty      = PACK_TRUE;
  protocol->out_packets_list.index      = PACK_PACKETS_INIT_INDEX;
  protocol->out_packets_list.lock_count = 0;

  #ifdef PACK_USE_OWN_QUEUE
  queue.empty  = PACK_TRUE;
  queue.start  = PACK_QUEUE_INIT_INDEX;
  queue.finish = PACK_QUEUE_INIT_INDEX;
  #endif

  return PACK_OK;
}
//==============================================================================
int pack_version(pack_ver version)
{
  strncpy((char *)version, PACK_VERSION, PACK_VERSION_SIZE);

  return PACK_OK;
}
//==============================================================================
pack_number _pack_global_number()
{
  return pack_global_number;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_index _pack_current_index(pack_type out)
#else
pack_index _pack_current_index(pack_type out, pack_protocol *protocol)
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
int pack_pack_by_number(pack_number number, pack_type out, pack_packet *pack, pack_protocol *protocol)
#endif
{
  if(out)
  {
    if(protocol->out_packets_list.empty)
      pack = 0;
    else
      for(pack_size i = 0; i <= PACK_OUT_PACKETS_COUNT; i++)
      {
        if(protocol->out_packets_list.items[i].number == number)
        {
          pack = &protocol->out_packets_list.items[i];
          return PACK_OK;
        }
      };
  }
  else
  {
    if(protocol->in_packets_list.empty)
      pack = 0;
    else
      for(pack_size i = 0; i <= PACK_IN_PACKETS_COUNT; i++)
      {
        if(protocol->in_packets_list.items[i].number == number)
        {
          pack = &protocol->in_packets_list.items[i];
          return PACK_OK;
        }
      };
  }

  return PACK_ERROR;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_packet *_pack_pack_by_number(pack_number number, pack_type out)
#else
pack_packet *_pack_pack_by_number(pack_number number, pack_type out, pack_protocol *protocol)
#endif
{
  if(out)
  {
    if(protocol->out_packets_list.empty)
      return NULL;
    for(pack_size i = 0; i <= PACK_OUT_PACKETS_COUNT; i++)
      if(protocol->out_packets_list.items[i].number == number)
        return &protocol->out_packets_list.items[i];
  }
  else
  {
    if(protocol->in_packets_list.empty)
      return NULL;
    for(pack_size i = 0; i <= PACK_IN_PACKETS_COUNT; i++)
      if(protocol->in_packets_list.items[i].number == number)
        return &protocol->in_packets_list.items[i];
  };

  return NULL;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_pack_by_index(pack_index index, pack_type out, pack_packet *pack)
#else
int pack_pack_by_index(pack_index index, pack_type out, pack_packet *pack, pack_protocol *protocol)
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

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_packet *_pack_pack_by_index (pack_index index, pack_type out)
#else
pack_packet *_pack_pack_by_index (pack_index index, pack_type out, pack_protocol *protocol)
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

  return PACK_OK;
}
#else
int pack_pack_current(pack_type out, pack_packet *pack, pack_protocol *protocol)
{
  pack_index tmp_index = _pack_current_index(out, protocol);

  pack_pack_by_index(tmp_index, out, pack, protocol);

  return PACK_OK;
}
#endif
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
pack_packet *_pack_pack_current(pack_type out)
{
  pack_index tmp_index = _pack_current_index(out);

  if(out)
    return &protocol->out_packets_list.items[tmp_index];
  else
    return &protocol->in_packets_list.items[tmp_index];
}
#else
pack_packet *_pack_pack_current(pack_type out, pack_protocol *protocol)
{
  pack_index tmp_index = _pack_current_index(out, protocol);

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
int pack_word_by_key(pack_packet *pack, pack_key key, pack_size *index, pack_word *word)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return PACK_ERROR;

  for(pack_size i = 0; i < pack->words_count; i++)
    if(strcmp((char *)pack->words[i].key, (char *)key) == 0)
    {
      *word = pack->words[i];
      *index = i;
      return PACK_OK;
    };

  return PACK_ERROR;
}
//==============================================================================
int pack_word_by_index(pack_packet *pack, pack_index index, pack_key key, pack_word *word)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return PACK_ERROR;

  if(pack->words_count <= index)
    return PACK_ERROR;

  *word = pack->words[index];
  memcpy(key, word->key, PACK_KEY_SIZE);

  return PACK_OK;
}
//==============================================================================
int pack_word_as_int(pack_word *word, int *value)
{
  *value = 0;

  for(pack_size j = 0; j < word->size; j++)
    *value = (*value << 8) + word->value[j];

  return PACK_OK;
}
//==============================================================================
int pack_word_as_float(pack_word *word, float *value)
{
  *value = 0.0;

  floatUnion tmp_value;
  for(pack_size j = 0; j < word->size; j++)
    tmp_value.buff[j] = word->value[j];
  *value = tmp_value.f;

  return PACK_OK;
}
//==============================================================================
int pack_word_as_string(pack_word *word, pack_string value)
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
        for(pack_size j = 0; j < word->size; j++)
          value[j] = word->value[j];
        value[word->size] = '\0';
      }
      break;
    case PACK_WORD_BYTES:
      break;
    default:
      break;
  }

  return PACK_OK;
}
//==============================================================================
int pack_word_as_bytes(pack_word *word, pack_bytes value, pack_size *size)
{
  return PACK_ERROR;
}
//==============================================================================
int pack_val_by_key_as_int(pack_packet *pack, pack_key key, pack_index *index, int   *value)
{
  pack_word tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == PACK_OK)
    return pack_word_as_int(&tmp_word, value);
  else
    return PACK_ERROR;
}
//==============================================================================
int pack_val_by_key_as_float(pack_packet *pack, pack_key key, pack_index *index, float *value)
{
  pack_word tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == PACK_OK)
    return pack_word_as_float(&tmp_word, value);
  else
    return PACK_ERROR;
}
//==============================================================================
int pack_val_by_key_as_string(pack_packet *pack, pack_key key, pack_index *index, pack_string value)
{
  pack_word tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == PACK_OK)
    return pack_word_as_string(&tmp_word, value);
  else
    return PACK_ERROR;
}
//==============================================================================
int pack_val_by_key_as_bytes(pack_packet *pack, pack_key key, pack_index *index, pack_bytes value, pack_size *size)
{
  pack_word tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == PACK_OK)
    return pack_word_as_bytes(&tmp_word, value, size);
  else
    return PACK_ERROR;
}
//==============================================================================
int pack_val_by_index_as_int(pack_packet *pack, pack_index index, pack_key key, int *value)
{
  pack_word tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == PACK_OK)
    return pack_word_as_int(&tmp_word, value);
  else
    return PACK_ERROR;
}
//==============================================================================
int pack_val_by_index_as_float(pack_packet *pack, pack_index index, pack_key key, float *value)
{
  pack_word tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == PACK_OK)
    return pack_word_as_float(&tmp_word, value);
  else
    return PACK_ERROR;
}
//==============================================================================
int pack_val_by_index_as_string(pack_packet *pack, pack_index index, pack_key key, pack_string value)
{
  pack_word tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == PACK_OK)
    return pack_word_as_string(&tmp_word, value);
  else
    return PACK_ERROR;
}
//==============================================================================
int pack_val_by_index_as_bytes(pack_packet *pack, pack_index index, pack_key key, pack_bytes value, pack_size *size)
{
  pack_word tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == PACK_OK)
    return pack_word_as_bytes(&tmp_word, value, size);
  else
    return PACK_ERROR;
}
//==============================================================================
int pack_key_by_index(pack_packet *pack, pack_index index, pack_key key)
{
  if(pack == NULL)
    return PACK_ERROR;

  if(pack->words_count > PACK_WORDS_COUNT)
    return PACK_ERROR;

  if(pack->words_count <= index)
    return PACK_ERROR;

  pack_word tmp_word = pack->words[index];

  memcpy(key, tmp_word.key, PACK_KEY_SIZE);

  return PACK_OK;
}
//==============================================================================
int pack_keys_to_csv(pack_packet *pack, unsigned char delimeter, pack_buffer buffer)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return PACK_ERROR;

  pack_size tmp_pos = 0;

  buffer[0] = '\0';

  for(pack_size i = 0; i < pack->words_count; i++)
  {
    for(pack_size j = 0; j < strlen((char *)pack->words[i].key); j++)
      buffer[tmp_pos++] = pack->words[i].key[j];
    buffer[tmp_pos++] = delimeter;
  }

  buffer[tmp_pos] = '\0';

  return PACK_OK;
}
//==============================================================================
int pack_values_to_csv(pack_packet *pack, unsigned char delimeter, pack_buffer buffer)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return PACK_ERROR;

  pack_size tmp_pos = 0;

  buffer[0] = '\0';

  pack_value valueS;

  for(pack_size i = 0; i < pack->words_count; i++)
  {
    pack_word_as_string(&pack->words[i], valueS);

    for(pack_size j = 0; j < strlen((char *)valueS); j++)
      buffer[tmp_pos++] = valueS[j];

    buffer[tmp_pos++] = delimeter;
  }

  buffer[tmp_pos] = '\0';

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_begin()
#else
int pack_begin(pack_protocol *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
  if(is_locked(PACK_OUT))
    return PACK_ERROR;
  lock(PACK_OUT);
  #else
  if(is_locked(PACK_OUT, protocol))
    return PACK_ERROR;
  lock(PACK_OUT, protocol);
  #endif

  pack_global_number++;
  if(pack_global_number >= USHRT_MAX)
    pack_global_number = PACK_GLOBAL_INIT_NUMBER;

  protocol->out_packets_list.index++;
  if(protocol->out_packets_list.index >= PACK_OUT_PACKETS_COUNT)
    protocol->out_packets_list.index = PACK_PACKETS_INIT_INDEX;

  protocol->out_packets_list.empty = PACK_FALSE;

  #ifdef PACK_USE_OWN_BUFFER
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  tmp_pack->number      = pack_global_number;
  tmp_pack->words_count = 0;

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_end()
#else
int pack_end(pack_protocol *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
  if(!is_locked(PACK_OUT))
    return PACK_ERROR;
  #else
  if(!is_locked(PACK_OUT, protocol))
    return PACK_ERROR;
  #endif

  #ifdef PACK_USE_OWN_QUEUE
  #ifdef PACK_USE_OWN_BUFFER
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  pack_queue_add(tmp_pack->number);
  #else
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  pack_queue_add(tmp_pack->number, protocol);
  #endif
  #endif

  #ifdef PACK_USE_OWN_BUFFER
  unlock(PACK_OUT);
  #else
  unlock(PACK_OUT, protocol);
  #endif

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_QUEUE
#ifdef PACK_USE_OWN_BUFFER
int pack_queue_add(pack_number number)
#else
int pack_queue_add(pack_number number, pack_protocol *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
  pack_packet *tmp_pack = _pack_pack_by_number(number, PACK_OUT);
  #else
  pack_packet *tmp_pack = _pack_pack_by_number(number, PACK_OUT, protocol);
  #endif

  if(tmp_pack == NULL)
    return PACK_ERROR;

  pack_index tmp_finish = queue.finish;
  queue.packets[tmp_finish] = tmp_pack;
  queue.empty = PACK_FALSE;
  queue.finish++;
  if(queue.finish > PACK_QUEUE_COUNT)
    queue.finish = 0;

  return PACK_OK;
}
#endif
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_queue_next(pack_buffer buffer, pack_size *size)
#else
int pack_queue_next(pack_buffer buffer, pack_size *size, pack_protocol *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
  if(is_locked(PACK_OUT))
    return PACK_QUEUE_EMPTY;
  #else
  if(is_locked(PACK_OUT, protocol))
    return PACK_QUEUE_EMPTY;
  #endif

  #ifdef PACK_USE_OWN_QUEUE
  if(queue.empty)
    return PACK_QUEUE_EMPTY;

  if((queue.start > PACK_QUEUE_COUNT) || (queue.finish > PACK_QUEUE_COUNT))
    return PACK_QUEUE_EMPTY;

  pack_index tmp_start = queue.start;
  pack_packet *tmp_pack = queue.packets[tmp_start];
  pack_packet_to_buffer(tmp_pack, buffer, size);

  queue.start++;
  if(queue.start > PACK_QUEUE_COUNT)
    queue.start = 0;

  if(queue.start == queue.finish)
    queue.empty = PACK_TRUE;

  return PACK_QUEUE_FULL;
  #else
  #ifdef PACK_USE_OWN_BUFFER
  return pack_current_packet_to_buffer(PACK_OUT, buffer, size);
  #else
  return pack_current_packet_to_buffer(PACK_OUT, buffer, size, protocol);
  #endif
  #endif
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_as_int(pack_key key, int value)
#else
int pack_add_as_int(pack_key key, int value, pack_protocol *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return PACK_ERROR;

  pack_word *tmp_word = &tmp_pack->words[tmp_pack->words_count];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = PACK_WORD_INT;

  // Size
  tmp_word->size = sizeof(int);

  // Value
  pack_size i = 0;
  tmp_word->value[i++] = (value >> 24) & 0xff;
  tmp_word->value[i++] = (value >> 16) & 0xff;
  tmp_word->value[i++] = (value >> 8 ) & 0xff;
  tmp_word->value[i++] = (value      ) & 0xff;

  // Words counter
  tmp_pack->words_count++;

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_as_float(pack_key key, float value)
#else
int pack_add_as_float(pack_key key, float value, pack_protocol *protocol)
#endif
{
  #ifdef PACK_USE_OWN_BUFFER
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return PACK_ERROR;

  pack_word *tmp_word = &tmp_pack->words[tmp_pack->words_count];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = PACK_WORD_FLOAT;

  // Size
  tmp_word->size = sizeof(float);

  // Value
  floatUnion tmp_value;
  tmp_value.f = value;
  for(pack_size i = 0; i < sizeof(float); i++)
    tmp_word->value[i] = tmp_value.buff[i];

  // Words counter
  tmp_pack->words_count++;

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_as_string(pack_key key, pack_string value)
#else
int pack_add_as_string(pack_key key, pack_string value, pack_protocol *protocol)
#endif
{
  pack_size tmp_size = strlen((char *)value);

  if(tmp_size >= PACK_VALUE_SIZE)
    return PACK_ERROR;

  #ifdef PACK_USE_OWN_BUFFER
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return PACK_ERROR;

  pack_word *tmp_word = &tmp_pack->words[tmp_pack->words_count];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = PACK_WORD_STRING;

  // Size
  tmp_word->size = tmp_size;

  // Value
  for(pack_size i = 0; i < tmp_size; i++)
    tmp_word->value[i] = value[i];

  // Words counter
  tmp_pack->words_count++;

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_as_bytes (pack_key key, pack_bytes value, pack_size size)
#else
int pack_add_as_bytes (pack_key key, pack_bytes value, pack_size size, pack_protocol *protocol)
#endif
{
  if(size >= PACK_VALUE_SIZE)
    return PACK_ERROR;

  #ifdef PACK_USE_OWN_BUFFER
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);
  #else
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT, protocol);
  #endif

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return PACK_ERROR;

  pack_word *tmp_word = &tmp_pack->words[tmp_pack->words_count];

  // Key
  memcpy(tmp_word->key, key, PACK_KEY_SIZE);

  // Type
  tmp_word->type = PACK_WORD_BYTES;

  // Size
  tmp_word->size = size;

  // Value
  for(pack_size i = 0; i < size; i++)
    tmp_word->value[i] = value[i];

  // Words counter
  tmp_pack->words_count++;

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_add_cmd(pack_value command)
{
  pack_key tmp_key = PACK_CMD_KEY;
  return pack_add_as_string(tmp_key, (pack_string)command);
}
#else
int pack_add_cmd(pack_value command, pack_protocol *protocol)
{
  pack_key tmp_key = PACK_CMD_KEY;
  return pack_add_as_string(tmp_key, (pack_string)command, protocol);
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
int pack_add_param_as_int(int param, pack_protocol *protocol)
{
  pack_key tmp_key = PACK_PARAM_KEY;
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
int pack_add_param_as_float(float param, pack_protocol *protocol)
{
  pack_key tmp_key = PACK_PARAM_KEY;
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
int pack_add_param_as_string(pack_string param, pack_protocol *protocol)
{
  pack_key tmp_key = PACK_PARAM_KEY;
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
int pack_add_param_as_bytes (pack_bytes param, pack_size size, pack_protocol *protocol)
{
  pack_key tmp_key = PACK_PARAM_KEY;
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
int pack_validate(pack_buffer buffer, pack_size size, pack_type only_validate)
#else
int pack_validate(pack_buffer buffer, pack_size size, pack_type only_validate, pack_protocol *protocol)
#endif
{
//  log_add("pack_validate", LOG_INFO);

  if((protocol->validation_buffer.size + size) > PACK_BUFFER_SIZE)
  {
    char tmp[256];
    sprintf(tmp, "pack_validate, buffer to big(%d/%d)", (protocol->validation_buffer.size + size), PACK_BUFFER_SIZE);
    log_add(tmp, LOG_CRITICAL_ERROR);
    return PACK_ERROR;
  }

  pack_size i = protocol->validation_buffer.size;
  for(pack_size j = 0; j < size; j++)
    protocol->validation_buffer.buffer[i++] = buffer[j];
  protocol->validation_buffer.size = i;

  pack_size tmp_validation_size = protocol->validation_buffer.size;

  int tmp_valid_count = 0;

  while(1)
  {
    if(protocol->validation_buffer.size <= 0)
      return tmp_valid_count;

    pack_size tmp_pack_pos = 0;

    // Get version
    for(pack_size i = 0; i < PACK_VERSION_SIZE; i++)
    {
      if(protocol->validation_buffer.buffer[tmp_pack_pos++] != PACK_VERSION[i])
        return tmp_valid_count;

      tmp_validation_size--;
      if(tmp_validation_size <= 0)
        return tmp_valid_count;
    };

    // Get size
    if(tmp_validation_size < 2)
      return tmp_valid_count;

    pack_size tmp_size = protocol->validation_buffer.buffer[tmp_pack_pos++] << 8;
    tmp_size          |= protocol->validation_buffer.buffer[tmp_pack_pos++];

    // Get value
    pack_buffer tmp_value_buffer;
    for(pack_size i = 0; i < (tmp_size + PACK_INDEX_SIZE); i++)
    {
      tmp_value_buffer[i] = protocol->validation_buffer.buffer[tmp_pack_pos++];

      tmp_validation_size--;
      if(tmp_validation_size <= 0)
        return tmp_valid_count;
    };

    // Get index
    pack_index tmp_index = tmp_value_buffer[0] << 8;
    tmp_index           |= tmp_value_buffer[1];

    // Get crc16 1
    if(tmp_validation_size < 2)
      return tmp_valid_count;

    pack_crc16 tmp_crc16_1 = protocol->validation_buffer.buffer[tmp_pack_pos++] << 8;
    tmp_crc16_1           |= protocol->validation_buffer.buffer[tmp_pack_pos++];

    // Get crc16 2
    pack_crc16 tmp_crc16_2 = getCRC16((char *)tmp_value_buffer, (tmp_size + PACK_INDEX_SIZE));

    // Check crc16
    if(tmp_crc16_1 != tmp_crc16_2)
      return tmp_valid_count;

    pack_size tmp_remain_count = protocol->validation_buffer.size - tmp_pack_pos;
    for(pack_size j = 0; j < tmp_remain_count; j++)
      protocol->validation_buffer.buffer[j] = protocol->validation_buffer.buffer[j + tmp_pack_pos];
    protocol->validation_buffer.size = tmp_remain_count;

    tmp_valid_count++;

    // TODO 3
    if(only_validate)
      continue;

    protocol->in_packets_list.index++;
    if(protocol->in_packets_list.index >= PACK_IN_PACKETS_COUNT)
      protocol->in_packets_list.index = PACK_PACKETS_INIT_INDEX;

    protocol->in_packets_list.empty = PACK_FALSE;

    #ifdef PACK_USE_OWN_BUFFER
    pack_packet *tmp_pack = _pack_pack_current(PACK_IN);
    #else
    pack_packet *tmp_pack = _pack_pack_current(PACK_IN, protocol);
    #endif
    if(tmp_pack == NULL)
      return PACK_ERROR;

    tmp_pack->number = tmp_index;

    pack_buffer_to_words(tmp_value_buffer, tmp_size, tmp_pack->words, &tmp_pack->words_count);

//    pack_parse_private_cmd(tmp_pack);
  };

  return tmp_valid_count;
}
//==============================================================================
int pack_buffer_to_words(pack_buffer buffer, pack_size buffer_size, pack_words words, pack_size *words_count)
{
  pack_size tmp_count = 0;

  // Exclude index
  pack_size i = PACK_INDEX_SIZE;
  while(i < buffer_size)
  {
    pack_word *tmp_word = &words[tmp_count];

    // Read Key
    for(pack_size j = 0; j < PACK_KEY_SIZE; j++)
      tmp_word->key[j] = buffer[i++];

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
    for(pack_size j = 0; j < tmp_word->size; j++)
      tmp_word->value[j] = buffer[i++];

    // Words count
    tmp_count++;
    *words_count = tmp_count;
    if(tmp_count > PACK_WORDS_COUNT)
      return PACK_ERROR;
  }

  return PACK_OK;
}
//==============================================================================
int pack_word_to_buffer(pack_word *word, pack_buffer buffer, pack_size *start_index)
{
  pack_size tmp_index = *start_index;

  for(pack_size i = 0; i < PACK_KEY_SIZE; i++)
    buffer[tmp_index++] = word->key[i];

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

  for(pack_size i = 0; i < word->size; i++)
    buffer[tmp_index++] = word->value[i];

  *start_index = tmp_index;

  return PACK_OK;
}
//==============================================================================
int pack_words_to_buffer(pack_packet *pack, pack_buffer buffer, pack_size start_index)
{
  for(pack_size i = 0; i < pack->words_count; i++)
    pack_word_to_buffer(&pack->words[i], buffer, &start_index);

  return PACK_OK;
}
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_current_packet_to_buffer(pack_type out, pack_buffer buffer, pack_size *size)
{
  pack_packet *tmp_pack = _pack_pack_current(out);
  return pack_packet_to_buffer(tmp_pack, buffer, size);
}
#else
int pack_current_packet_to_buffer(pack_type out, pack_buffer buffer, pack_size *size, pack_protocol *protocol)
{
  pack_packet *tmp_pack = _pack_pack_current(out, protocol);

  if(tmp_pack == NULL)
    return PACK_QUEUE_EMPTY;

  if(pack_packet_to_buffer(tmp_pack, buffer, size) == PACK_OK)
    return PACK_QUEUE_FULL;
  else
    return PACK_QUEUE_EMPTY;
}
#endif
//==============================================================================
int pack_packet_to_buffer(pack_packet *packet, pack_buffer buffer, pack_size *size)
{
  pack_size tmp_pack_pos = 0;

  // Version
  for(pack_size i = 0; i < PACK_VERSION_SIZE; i++)
    buffer[tmp_pack_pos++] = PACK_VERSION[i];

  pack_size tmp_packet_size = _pack_words_size(packet);

  // Size(IndexSize + DataSize)
//  buffer[tmp_pack_pos++] = (packet->size >> 8) & 0xff;
//  buffer[tmp_pack_pos++] = (packet->size     ) & 0xff;
  buffer[tmp_pack_pos++] = (tmp_packet_size >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (tmp_packet_size     ) & 0xff;

  // Index
  buffer[tmp_pack_pos++] = (packet->number >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (packet->number     ) & 0xff;

  // Buffer for calc crc
  pack_buffer tmp_buffer;
  tmp_buffer[0] = (packet->number >> 8) & 0xff;
  tmp_buffer[1] = (packet->number     ) & 0xff;
  pack_words_to_buffer(packet, tmp_buffer, PACK_INDEX_SIZE);

//  pack_size tmp_total_size = (PACK_INDEX_SIZE + packet->size);
  pack_size tmp_total_size = (PACK_INDEX_SIZE + tmp_packet_size);

  // Words to buffer
  for(pack_size i = PACK_INDEX_SIZE; i < tmp_total_size; i++)
    buffer[tmp_pack_pos++] = tmp_buffer[i];

  // CRC16
  pack_crc16 tmp_crc16 = getCRC16((char *)tmp_buffer, tmp_total_size);
  buffer[tmp_pack_pos++] = (tmp_crc16 >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (tmp_crc16     ) & 0xff;

  // pack_outer_size include PACK_INDEX_SIZE
//  *size = PACK_VERSION_SIZE + PACK_SIZE_SIZE + PACK_INDEX_SIZE + packet->size + PACK_CRC_SIZE;
  *size = PACK_VERSION_SIZE + PACK_SIZE_SIZE + PACK_INDEX_SIZE + tmp_packet_size + PACK_CRC_SIZE;

  return PACK_OK;
}
//==============================================================================
pack_size _pack_words_count(pack_packet *pack)
{
  return pack->words_count;
}
//==============================================================================
pack_size _pack_words_size(pack_packet *pack)
{
  pack_size tmp_size = 0;

  for(pack_size i = 0; i < pack->words_count; i++)
    tmp_size += _pack_word_size(&pack->words[i]);

  return tmp_size;
}
//==============================================================================
pack_size _pack_word_size(pack_word *word)
{
  pack_size tmp_size = PACK_KEY_SIZE + PACK_TYPE_SIZE;

  pack_type tmp_word_type = word->type;

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
int pack_command(pack_packet *pack, pack_value command)
{
  pack_count tmp_words_count = _pack_words_count(pack);
  if(tmp_words_count >= 1)
  {
    pack_key  tmp_key;
    pack_value valueS;

    int res = pack_val_by_index_as_string(pack, 0, tmp_key, valueS);
    if(res == PACK_OK)
      if(strcmp((char *)tmp_key, PACK_CMD_KEY) == 0)
      {
        strcpy(command, valueS);
        return PACK_OK;
      };
  };

  return PACK_ERROR;
}
//==============================================================================
pack_size _pack_params_count(pack_packet *pack)
{
  pack_size tmp_words_count = _pack_words_count(pack);
  pack_size tmp_params_count = 0;
  pack_key  tmp_key;

  for(pack_index i = 1; i < tmp_words_count; i++)
  {
    int res = pack_key_by_index(pack, i, tmp_key);
    if(res == PACK_OK)
      if(strcmp((char *)tmp_key, PACK_PARAM_KEY) == 0)
        tmp_params_count++;
  }

  return tmp_params_count;
}
//==============================================================================
int pack_param_by_index_as_string(pack_packet *pack, pack_index index, pack_key key, pack_string value)
{
  return pack_val_by_index_as_string(pack, index, key, value);
}
//==============================================================================
