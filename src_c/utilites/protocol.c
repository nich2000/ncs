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

#include "protocol.h"
#include "log.h"
#include "protocol_utils.h"
//==============================================================================
// out
//==============================================================================
// Global total output packets counter
pack_number      out_global_number = PACK_GLOBAL_INIT_NUMBER;
// Current index in packets
pack_number      out_packets_index = PACK_PACKETS_INIT_INDEX;
// List of output packets
pack_out_packets out_packets;
// Lock count
int              out_lock = 0;
//==============================================================================
// in
//==============================================================================
// Global total input packets counter
pack_number      in_global_number = PACK_GLOBAL_INIT_NUMBER;
// Current index in packets
pack_number      in_packets_index = PACK_PACKETS_INIT_INDEX;
// Validation buffer
pack_index       in_validation_buffer_size;
pack_buffer      in_validation_buffer;
// List of input packets
pack_in_packets  in_packets;
// Lock count
int              in_lock = 0;
//==============================================================================
// queue
//==============================================================================
pack_queue    queue;
//==============================================================================
int is_locked(pack_type out)
{
  if(out)
    return out_lock > 0;
  else
    return in_lock > 0;
}
//==============================================================================
int lock(pack_type out)
{
  if(out)
    out_lock++;
  else
    in_lock++;

  return PACK_OK;
}
//==============================================================================
int unlock(pack_type out)
{
  if(out)
  {
    out_lock--;
    if(out_lock < 0)
      out_lock = 0;
  }
  else
  {
    in_lock--;
    if(in_lock < 0)
      in_lock = 0;
  }

  return PACK_OK;
}
//==============================================================================
int pack_init()
{
  in_validation_buffer_size = 0;

  in_global_number           = PACK_GLOBAL_INIT_NUMBER;
  in_packets_index           = PACK_PACKETS_INIT_INDEX;

  out_global_number          = PACK_GLOBAL_INIT_NUMBER;
  out_packets_index          = PACK_PACKETS_INIT_INDEX;

  queue.empty                = PACK_TRUE;
  queue.start                = PACK_QUEUE_INIT_INDEX;
  queue.finish               = PACK_QUEUE_INIT_INDEX;

  return PACK_OK;
}
//==============================================================================
int pack_version(pack_ver version)
{
  strncpy((char *)version, PACK_VERSION, PACK_VERSION_SIZE);

  return PACK_OK;
}
//==============================================================================
pack_number _pack_global_number(pack_type out)
{
  if(out)
    return out_global_number;
  else
    return in_global_number;
}
//==============================================================================
pack_index _pack_current_index(pack_type out)
{
  if(out)
    return out_packets_index;
  else
    return in_packets_index;
}
//==============================================================================
int pack_pack_by_number(pack_number number, pack_type out, pack_packet *pack)
{
  if(out)
  {
    for(pack_size i = 0; i <= PACK_OUT_PACKETS_COUNT; i++)
    {
      if(out_packets[i].number == number)
      {
        pack = &out_packets[i];
        return PACK_OK;
      }
    };
  }
  else
  {
    for(pack_size i = 0; i <= PACK_IN_PACKETS_COUNT; i++)
    {
      if(in_packets[i].number == number)
      {
        pack = &in_packets[i];
        return PACK_OK;
      }
    };
  }

  return 1;
}
//==============================================================================
pack_packet *_pack_pack_by_number(pack_number number, pack_type out)
{
  if(out)
  {
    for(pack_size i = 0; i <= PACK_OUT_PACKETS_COUNT; i++)
      if(out_packets[i].number == number)
        return &out_packets[i];
  }
  else
  {
    for(pack_size i = 0; i <= PACK_IN_PACKETS_COUNT; i++)
      if(in_packets[i].number == number)
        return &in_packets[i];
  };

  return NULL;
}
//==============================================================================
int pack_pack_by_index(pack_index index, pack_type out, pack_packet *pack)
{
  if(out)
    pack = &out_packets[index];
  else
    pack = &in_packets[index];

  return PACK_OK;
}
//==============================================================================
pack_packet *_pack_pack_by_index (pack_index index, pack_type out)
{
  if(out)
    return &out_packets[index];
  else
    return &in_packets[index];
}
//==============================================================================
int pack_pack_current(pack_type out, pack_packet *pack)
{
  pack_index tmp_index = _pack_current_index(out);

  pack_pack_by_index(tmp_index, out, pack);

  return PACK_OK;
}
//==============================================================================
pack_packet *_pack_pack_current(pack_type out)
{
  pack_index tmp_index = _pack_current_index(out);

  if(out)
    return &out_packets[tmp_index];
  else
    return &in_packets[tmp_index];
}
//==============================================================================
int pack_word_by_key(pack_packet *pack, pack_key key, pack_size *index, pack_word *word)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return 1;

  for(pack_size i = 0; i < pack->words_count; i++)
    if(strcmp((char *)pack->words[i].key, (char *)key) == 0)
    {
      *word = pack->words[i];
      *index = i;
      return 0;
    };

  return 2;
}
//==============================================================================
int pack_word_by_index(pack_packet *pack, pack_index index, pack_key key, pack_word *word)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return 1;

  if(pack->words_count <= index)
    return 2;

  *word = pack->words[index];
  memcpy(key, word->key, PACK_KEY_SIZE);

  return 0;
}
//==============================================================================
int pack_word_as_int(pack_word *word, int *value)
{
  *value = 0;

  for(pack_size j = 0; j < word->size; j++)
    *value = (*value << 8) + word->value[j];

  return 0;
}
//==============================================================================
int pack_word_as_float(pack_word *word, float *value)
{
  *value = 0.0;

  floatUnion tmp_value;
  for(pack_size j = 0; j < word->size; j++)
    tmp_value.buff[j] = word->value[j];
  *value = tmp_value.f;

  return 0;
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

  return 0;
}
//==============================================================================
int pack_word_as_bytes(pack_word *word, pack_bytes value, pack_size *size)
{
  return 0;
}
//==============================================================================
int pack_val_by_key_as_int(pack_packet *pack, pack_key key, pack_index *index, int   *value)
{
  pack_word tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == PACK_OK)
    return pack_word_as_int(&tmp_word, value);
  else
    return 1;
}
//==============================================================================
int pack_val_by_key_as_float(pack_packet *pack, pack_key key, pack_index *index, float *value)
{
  pack_word tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == PACK_OK)
    return pack_word_as_float(&tmp_word, value);
  else
    return 1;
}
//==============================================================================
int pack_val_by_key_as_string(pack_packet *pack, pack_key key, pack_index *index, pack_string value)
{
  pack_word tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == PACK_OK)
    return pack_word_as_string(&tmp_word, value);
  else
    return 1;
}
//==============================================================================
int pack_val_by_key_as_bytes(pack_packet *pack, pack_key key, pack_index *index, pack_bytes value, pack_size *size)
{
  pack_word tmp_word;
  if(pack_word_by_key(pack, key, index, &tmp_word) == PACK_OK)
    return pack_word_as_bytes(&tmp_word, value, size);
  else
    return 1;
}
//==============================================================================
int pack_val_by_index_as_int(pack_packet *pack, pack_index index, pack_key key, int *value)
{
  pack_word tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == PACK_OK)
    return pack_word_as_int(&tmp_word, value);

  return 0;
}
//==============================================================================
int pack_val_by_index_as_float(pack_packet *pack, pack_index index, pack_key key, float *value)
{
  pack_word tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == PACK_OK)
    return pack_word_as_float(&tmp_word, value);
  else
    return 1;
}
//==============================================================================
int pack_val_by_index_as_string(pack_packet *pack, pack_index index, pack_key key, pack_string value)
{
  pack_word tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == PACK_OK)
    return pack_word_as_string(&tmp_word, value);
  else
    return 1;
}
//==============================================================================
int pack_val_by_index_as_bytes(pack_packet *pack, pack_index index, pack_key key, pack_bytes value, pack_size *size)
{
  pack_word tmp_word;
  if(pack_word_by_index(pack, index, key, &tmp_word) == PACK_OK)
    return pack_word_as_bytes(&tmp_word, value, size);
  else
    return 1;
}
//==============================================================================
int pack_key_by_index(pack_packet *pack, pack_index index, pack_key key)
{
  if(pack == NULL)
    return 1;

  if(pack->words_count > PACK_WORDS_COUNT)
    return 2;

  if(pack->words_count <= index)
    return 3;

  pack_word tmp_word = pack->words[index];

  memcpy(key, tmp_word.key, PACK_KEY_SIZE);

  return PACK_OK;
}
//==============================================================================
int pack_keys_to_csv(pack_packet *pack, unsigned char delimeter, pack_buffer buffer)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return 1;

  pack_size tmp_pos = 0;

  buffer[0] = '\0';

  for(pack_size i = 0; i < pack->words_count; i++)
  {
    for(pack_size j = 0; j < strlen((char *)pack->words[i].key); j++)
      buffer[tmp_pos++] = pack->words[i].key[j];
    buffer[tmp_pos++] = delimeter;
  }

  buffer[tmp_pos] = '\0';

  return 0;
}
//==============================================================================
int pack_values_to_csv(pack_packet *pack, unsigned char delimeter, pack_buffer buffer)
{
  if(pack->words_count > PACK_WORDS_COUNT)
    return 1;

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

  return 0;
}
//==============================================================================
int pack_begin()
{
  if(is_locked(PACK_OUT))
    return 1;

  lock(PACK_OUT);

  out_global_number++;
  if(out_global_number >= USHRT_MAX)
    out_global_number = PACK_GLOBAL_INIT_NUMBER;

  out_packets_index++;
  if(out_packets_index >= PACK_OUT_PACKETS_COUNT)
    out_packets_index = PACK_PACKETS_INIT_INDEX;

//  char tmp[128];

//  sprintf(tmp, "out_global_number: %d", out_global_number);
//  add_to_log(tmp, LOG_DEBUG);

//  sprintf(tmp, "out_packets_index: %d", out_packets_index);
//  add_to_log(tmp, LOG_DEBUG);

  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);

  memcpy(tmp_pack->version, PACK_VERSION, PACK_VERSION_SIZE);
  tmp_pack->size        = 0;
  tmp_pack->number      = out_global_number;
  tmp_pack->words_count = 0;

  return 0;
}
//==============================================================================
int pack_end()
{
  if(!is_locked(PACK_OUT))
    return 1;

  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);

  pack_queue_add(tmp_pack->number);

  unlock(PACK_OUT);

  return 0;
}
//==============================================================================
int pack_queue_add(pack_number number)
{
  pack_packet *tmp_pack = _pack_pack_by_number(number, PACK_OUT);

  if(tmp_pack == 0)
    return 1;

  pack_index tmp_finish = queue.finish;
  queue.packets[tmp_finish] = tmp_pack;
  queue.empty = PACK_FALSE;
  queue.finish++;
  if(queue.finish > PACK_QUEUE_COUNT)
    queue.finish = 0;

  return 0;
}
//==============================================================================
int pack_queue_next(pack_buffer buffer, pack_size *size)
{
  if(queue.empty)
    return 1;

  if((queue.start > PACK_QUEUE_COUNT) || (queue.finish > PACK_QUEUE_COUNT))
    return 2;

  pack_index tmp_start = queue.start;
  pack_packet *tmp_pack = queue.packets[tmp_start];
  pack_packet_to_buffer(tmp_pack, buffer, size);

  queue.start++;
  if(queue.start > PACK_QUEUE_COUNT)
    queue.start = 0;

  if(queue.start == queue.finish)
    queue.empty = PACK_TRUE;

  return 0;
}
//==============================================================================
int pack_add_as_int(pack_key key, int value)
{
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return 2;

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

  // Size counter
  tmp_pack->size += pack_word_size(tmp_word);

  // Words counter
  tmp_pack->words_count++;

  return 0;
}
//==============================================================================
int pack_add_as_float(pack_key key, float value)
{
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return 2;

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

  // Size counter
  tmp_pack->size += pack_word_size(tmp_word);

  // Words counter
  tmp_pack->words_count++;

  return 0;
}
//==============================================================================
int pack_add_as_string(pack_key key, pack_string value)
{
  pack_size tmp_size = strlen((char *)value);

  if(tmp_size >= PACK_VALUE_SIZE)
    return 1;

  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return 2;

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

  // Size counter
  tmp_pack->size += pack_word_size(tmp_word);

  // Words counter
  tmp_pack->words_count++;

  return 0;
}
//==============================================================================
int pack_add_as_bytes (pack_key key, pack_bytes value, pack_size size)
{
  if(size >= PACK_VALUE_SIZE)
    return 1;

  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT);

  if(tmp_pack->words_count >= PACK_WORDS_COUNT)
    return 2;

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

  // Size counter
  tmp_pack->size += pack_word_size(tmp_word);

  // Words counter
  tmp_pack->words_count++;

  return 0;
}
//==============================================================================
int pack_add_cmd(pack_value command)
{
  pack_key tmp_key = PACK_CMD_KEY;
  return pack_add_as_string(tmp_key, (pack_string)command);
}
//==============================================================================
int pack_add_param_as_int(int param)
{
  pack_key tmp_key = PACK_PARAM_KEY;
  return pack_add_as_int(tmp_key, param);
}
//==============================================================================
int pack_add_param_as_float(float param)
{
  pack_key tmp_key = PACK_PARAM_KEY;
  return pack_add_as_float(tmp_key, param);
}
//==============================================================================
int pack_add_param_as_string(pack_string param)
{
  pack_key tmp_key = PACK_PARAM_KEY;
  return pack_add_as_string(tmp_key, param);
}
//==============================================================================
int pack_add_param_as_bytes (pack_bytes param, pack_size size)
{
  pack_key tmp_key = PACK_PARAM_KEY;
  return pack_add_as_bytes(tmp_key, param, size);
}
//==============================================================================
int pack_validate(pack_buffer buffer, pack_size size, pack_type only_validate)
{
//  add_to_log("pack_validate", LOG_DEBUG);

  pack_size i = in_validation_buffer_size;
  for(pack_size j = 0; j < size; j++)
    in_validation_buffer[i++] = buffer[j];
  in_validation_buffer_size += size;

  while(1)
  {
    if(in_validation_buffer_size <= 0)
      return 0;

    pack_size tmp_pack_pos = 0;

    // Get version
    for(pack_size i = 0; i < PACK_VERSION_SIZE; i++)
      if(in_validation_buffer[tmp_pack_pos++] != PACK_VERSION[i])
        return 1;

    // Get size
    pack_size tmp_size = in_validation_buffer[tmp_pack_pos++] << 8;
    tmp_size          |= in_validation_buffer[tmp_pack_pos++];

    // Get value
    pack_buffer tmp_value_buffer;
    for(pack_size i = 0; i < (tmp_size + PACK_INDEX_SIZE); i++)
      tmp_value_buffer[i] = in_validation_buffer[tmp_pack_pos++];

    // Get index
    pack_index tmp_index = (tmp_value_buffer[0] << 8) | tmp_value_buffer[1];

    // Get crc16 1
    pack_crc16 tmp_crc16_1 = in_validation_buffer[tmp_pack_pos++] << 8;
    tmp_crc16_1           |= in_validation_buffer[tmp_pack_pos++];
    // Get crc16 2
    pack_crc16 tmp_crc16_2 = getCRC16((char *)tmp_value_buffer, (tmp_size + PACK_INDEX_SIZE));
    // Check crc16
    if(tmp_crc16_1 != tmp_crc16_2)
      return 2;

    pack_size i = in_validation_buffer_size - tmp_pack_pos;
    for(pack_size j = 0; j < in_validation_buffer_size; j++)
      in_validation_buffer[j] = in_validation_buffer[i++];
    in_validation_buffer_size -= tmp_pack_pos;

    if(only_validate)
      return 0;

    in_packets_index++;
    if(in_packets_index >= PACK_IN_PACKETS_COUNT)
      in_packets_index = PACK_PACKETS_INIT_INDEX;

    pack_packet *tmp_pack = _pack_pack_current(PACK_IN);
    if(tmp_pack == NULL)
      return 3;

    strncpy((char *)tmp_pack->version, PACK_VERSION, PACK_VERSION_SIZE);
    tmp_pack->size = tmp_size;
    tmp_pack->number = tmp_index;
    tmp_pack->crc = tmp_crc16_1;

    pack_buffer_to_words(tmp_value_buffer, tmp_size, tmp_pack->words, &tmp_pack->words_count);

    pack_parse_cmd(tmp_pack);
  };
}
//==============================================================================
int pack_buffer_to_words(pack_buffer buffer, pack_size buffer_size, pack_words words, pack_size *words_count)
{
  pack_size tmp_count = 0;

  // Exclude index
  pack_size i = PACK_INDEX_SIZE;
  while(i < buffer_size)
  {
//    char tmp[128];
//    sprintf(tmp, "%u %u", i, tmp_count);
//    add_to_log(tmp, LOG_DEBUG);

    // Read Key
    for(pack_size j = 0; j < PACK_KEY_SIZE; j++)
      words[tmp_count].key[j] = buffer[i++];

    // Read Type
    words[tmp_count].type = buffer[i++];

    // Read Size
    switch (words[tmp_count].type) {
    case PACK_WORD_NONE:
      break;
    case PACK_WORD_INT:
      words[tmp_count].size = sizeof(int);
      break;
    case PACK_WORD_FLOAT:
      words[tmp_count].size = sizeof(float);
      break;
    case PACK_WORD_STRING:
      {
        words[tmp_count].size = buffer[i++] << 8;
        words[tmp_count].size |= buffer[i++];
      }
      break;
    case PACK_WORD_BYTES:
      {
        words[tmp_count].size = buffer[i++] << 8;
        words[tmp_count].size |= buffer[i++];
      }
      break;
    default:
      break;
    }

    // Read Value
    for(pack_size j = 0; j < words[tmp_count].size; j++)
      words[tmp_count].value[j] = buffer[i++];

    tmp_count++;

//    sprintf(tmp, "%u %u", i, tmp_count);
//    add_to_log(tmp, LOG_DEBUG);
  }

  *words_count = tmp_count;

  return 0;
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

  return 0;
}
//==============================================================================
int pack_words_to_buffer(pack_packet *pack, pack_buffer buffer, pack_size start_index)
{
  for(pack_size i = 0; i < pack->words_count; i++)
    pack_word_to_buffer(&pack->words[i], buffer, &start_index);

  return 0;
}
//==============================================================================
int pack_packet_to_buffer(pack_packet *packet, pack_buffer buffer, pack_size *size)
{
  pack_size tmp_pack_pos = 0;

  // Version
  for(pack_size i = 0; i < PACK_VERSION_SIZE; i++)
    buffer[tmp_pack_pos++] = PACK_VERSION[i];

  // Size(IndexSize + DataSize)
  buffer[tmp_pack_pos++] = (packet->size >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (packet->size     ) & 0xff;

  // Index
  buffer[tmp_pack_pos++] = (packet->number >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (packet->number     ) & 0xff;

  // Buffer for calc crc
  pack_buffer tmp_buffer;
  tmp_buffer[0] = (packet->number >> 8) & 0xff;
  tmp_buffer[1] = (packet->number     ) & 0xff;
  pack_words_to_buffer(packet, tmp_buffer, PACK_INDEX_SIZE);

  pack_size tmp_total_size = (PACK_INDEX_SIZE + packet->size);

  // Words to buffer
  for(pack_size i = PACK_INDEX_SIZE; i < tmp_total_size; i++)
    buffer[tmp_pack_pos++] = tmp_buffer[i];

  // CRC16
  pack_crc16 tmp_crc16 = getCRC16((char *)tmp_buffer, tmp_total_size);
  buffer[tmp_pack_pos++] = (tmp_crc16 >> 8) & 0xff;
  buffer[tmp_pack_pos++] = (tmp_crc16     ) & 0xff;

  // pack_outer_size include PACK_INDEX_SIZE
  *size = PACK_VERSION_SIZE + PACK_SIZE_SIZE + PACK_INDEX_SIZE + packet->size + PACK_CRC_SIZE;

  return 0;
}
//==============================================================================
pack_size pack_words_count(pack_packet *pack)
{
  return pack->words_count;
}
//==============================================================================
pack_size pack_word_size(pack_word *word)
{
  pack_size tmp_size = PACK_KEY_SIZE + PACK_TYPE_SIZE;

  switch (word->type)
  {
  case PACK_WORD_NONE:
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
int pack_parse_cmd(pack_packet *pack)
{
  pack_size tmp_words_count = pack_words_count(pack);

  if(tmp_words_count >= 1)
  {
    pack_key tmp_key;
    int res = pack_key_by_index(pack, 0, tmp_key);
    if(res != 0)
      return res;

    if(strcmp((char *)tmp_key, PACK_CMD_KEY) == 0)
      return 10 + pack_exec_cmd(pack);
  };

  return PACK_OK;
}
//==============================================================================
pack_size pack_params_count(pack_packet *pack)
{
  pack_size tmp_words_count = pack_words_count(pack);
  pack_size tmp_params_count = 0;
  pack_key  tmp_key;

  for(pack_index i = 1; i < tmp_words_count; i++)
  {
    int res = pack_key_by_index(pack, i, tmp_key);

    if(res == 0)
      if(strcmp((char *)tmp_key, PACK_PARAM_KEY) == 0)
        tmp_params_count++;
  }

  return tmp_params_count;
}
//==============================================================================
int pack_exec_cmd(pack_packet *pack)
{
  char tmp[128];

  pack_key tmp_key;
  pack_value tmp_command;
  int res = pack_val_by_index_as_string(pack, 0, tmp_key, tmp_command);

  if(res == PACK_OK)
  {
    pack_size tmp_params_count = pack_params_count(pack);

    sprintf(tmp, "%s: %s(%d)", tmp_key, tmp_command, tmp_params_count);
    log_add(tmp, LOG_DEBUG);
  }

  return 0;
}
//==============================================================================
