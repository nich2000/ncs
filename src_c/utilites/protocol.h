#ifndef PROTOCOL_H
#define PROTOCOL_H
//==============================================================================
// http://sigidin.blogspot.ru/2011/01/linux.html
// http://ru.stackoverflow.com/questions/376364/%D0%A7%D1%82%D0%BE-%D0%B4%D0%B5%D0%BB%D0%B0%D0%B5%D1%82-select-%D0%B8-fd-isset
// http://www.binarytides.com/code-tcp-socket-server-winsock/
// http://www.binarytides.com/server-client-example-c-sockets-linux/
//==============================================================================
#include "protocol_utils.h"
//==============================================================================
#define PACK_BUFFER_SIZE         512
#define PACK_VALUE_SIZE          10
#define PACK_KEY_SIZE            4
#define PACK_VERSION_SIZE        4
//==============================================================================
#define PACK_WORDS_COUNT         20
#define PACK_OUT_PACKETS_COUNT   (1 * 60 * 10) // 1 minute
#define PACK_IN_PACKETS_COUNT    (    10 * 10) // 10 seconds
#define PACK_QUEUE_COUNT         1
//==============================================================================
#define PACK_GLOBAL_INIT_NUMBER  0
#define PACK_QUEUE_INIT_INDEX    0
#define PACK_PACKETS_INIT_INDEX  0
//==============================================================================
#define PACK_VERSION             "V01\0"
#define PACK_CMD_KEY             "CMD\0"
#define PACK_PARAM_KEY           "PAR\0"
//==============================================================================
#define PACK_OK                  0
#define PACK_ERROR              -1
//==============================================================================
#define PACK_QUEUE_EMPTY         0
#define PACK_QUEUE_FULL          1
//==============================================================================
#define PACK_TRUE                1
#define PACK_FALSE               0
//==============================================================================
#define PACK_OUT                 1
#define PACK_IN                  0
//==============================================================================
#define PACK_WORD_NONE           0
#define PACK_WORD_INT            1
#define PACK_WORD_FLOAT          2
#define PACK_WORD_STRING         3
#define PACK_WORD_BYTES          4
//==============================================================================
typedef unsigned char           *pack_string;
typedef unsigned char           *pack_bytes;
typedef unsigned char            pack_buffer[PACK_BUFFER_SIZE];
typedef unsigned char            pack_value [PACK_VALUE_SIZE];
typedef unsigned char            pack_ver   [PACK_VERSION_SIZE];
typedef unsigned char            pack_key   [PACK_KEY_SIZE];
//==============================================================================
typedef unsigned short           pack_count;
typedef unsigned short           pack_size;
typedef unsigned short           pack_index;
typedef unsigned short           pack_number;
typedef unsigned char            pack_type;
typedef unsigned short           pack_crc16;
//==============================================================================
#define PACK_SIZE_SIZE           sizeof(pack_size)
#define PACK_INDEX_SIZE          sizeof(pack_index)
#define PACK_NUMBER_SIZE         sizeof(pack_number)
#define PACK_TYPE_SIZE           sizeof(pack_type)
#define PACK_CRC_SIZE            sizeof(pack_crc16)
//==============================================================================
typedef struct
{
  pack_key   key;
  pack_type  type;
  pack_size  size;
  pack_value value;
} pack_word;
//==============================================================================
typedef pack_word pack_words[PACK_WORDS_COUNT];
//==============================================================================
typedef struct
{
  pack_number number;
  pack_words  words;
  pack_size   words_count;
} pack_packet;
//==============================================================================
typedef pack_packet  pack_out_packets  [PACK_OUT_PACKETS_COUNT];
typedef pack_packet  pack_in_packets   [PACK_IN_PACKETS_COUNT];
typedef pack_packet *pack_queue_packets[PACK_QUEUE_COUNT];
//==============================================================================
typedef struct
{
  pack_size   size;
  pack_buffer buffer;
} pack_validation_buffer;
//==============================================================================
typedef struct
{
  pack_index       index;
  pack_out_packets items;
  pack_count       lock_count;
} pack_out_packets_list;
//==============================================================================
typedef struct
{
  pack_index      index;
  pack_in_packets items;
  pack_count      lock_count;
} pack_in_packets_list;
//==============================================================================
typedef struct
{
  pack_validation_buffer validation_buffer;
  pack_out_packets_list  out_packets_list;
  pack_in_packets_list   in_packets_list;
} pack_protocol;
//==============================================================================
typedef struct
{
  pack_type          empty;
  pack_index         start;
  pack_index         finish;
  pack_queue_packets packets;
} pack_queue;
//==============================================================================
int         _pack_get_last_error       ();
pack_number _pack_global_number        ();
int          pack_version              (pack_ver version);
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
int pack_init();
int pack_begin();
int pack_end();
pack_packet *_pack_pack_current(pack_type out);
int pack_validate(pack_buffer buffer, pack_size size, pack_type only_validate);
int pack_pack_current(pack_type out, pack_packet *pack);
int pack_add_as_int(pack_key key, int value);
int pack_add_as_float(pack_key key, float value);
int pack_add_as_string(pack_key key, pack_string value);
int pack_add_as_bytes(pack_key key, pack_bytes value, pack_size size);
int pack_add_cmd(pack_value command);
int pack_add_param_as_int(int   param);
int pack_add_param_as_float(float param);
int pack_add_param_as_string(pack_string param);
int pack_add_param_as_bytes(pack_bytes param, pack_size size);
int pack_current_packet_to_buffer(pack_buffer buffer, pack_size *size);
int pack_queue_next(pack_buffer buffer, pack_size *size);
#else
int pack_init(pack_protocol *protocol);
int pack_begin(pack_protocol *protocol);
int pack_end(pack_protocol *protocol);
pack_packet *_pack_pack_current(pack_type out, pack_protocol *protocol);
int pack_validate(pack_buffer buffer, pack_size size, pack_type only_validate, pack_protocol *protocol);
int pack_pack_current(pack_type out, pack_packet *pack, pack_protocol *protocol);
int pack_add_as_int(pack_key key, int value, pack_protocol *protocol);
int pack_add_as_float(pack_key key, float value, pack_protocol *protocol);
int pack_add_as_string(pack_key key, pack_string value, pack_protocol *protocol);
int pack_add_as_bytes(pack_key key, pack_bytes value, pack_size size, pack_protocol *protocol);
int pack_add_cmd(pack_value command, pack_protocol *protocol);
int pack_add_param_as_int(int   param, pack_protocol *protocol);
int pack_add_param_as_float(float param, pack_protocol *protocol);
int pack_add_param_as_string(pack_string param, pack_protocol *protocol);
int pack_add_param_as_bytes(pack_bytes param, pack_size size, pack_protocol *protocol);
int pack_current_packet_to_buffer(pack_buffer buffer, pack_size *size, pack_protocol *protocol);
int pack_queue_next(pack_buffer buffer, pack_size *size, pack_protocol *protocol);
#endif
//==============================================================================
pack_size _pack_words_count    (pack_packet *pack);
pack_size _pack_params_count   (pack_packet *pack);
//==============================================================================
int pack_keys_to_csv           (pack_packet *pack, unsigned char delimeter, pack_buffer buffer);
int pack_values_to_csv         (pack_packet *pack, unsigned char delimeter, pack_buffer buffer);
//==============================================================================
int pack_word_by_key           (pack_packet *pack, pack_key key,     pack_index *index, pack_word *word);
int pack_word_by_index         (pack_packet *pack, pack_index index, pack_key key,      pack_word *word);
int pack_key_by_index          (pack_packet *pack, pack_index index, pack_key key);
//==============================================================================
int pack_val_by_key_as_int     (pack_packet *pack, pack_key key, pack_index *index, int   *value);
int pack_val_by_key_as_float   (pack_packet *pack, pack_key key, pack_index *index, float *value);
int pack_val_by_key_as_string  (pack_packet *pack, pack_key key, pack_index *index, pack_string value);
int pack_val_by_key_as_bytes   (pack_packet *pack, pack_key key, pack_index *index, pack_bytes value, pack_size *size);
//==============================================================================
int pack_val_by_index_as_int   (pack_packet *pack, pack_index index, pack_key key, int   *value);
int pack_val_by_index_as_float (pack_packet *pack, pack_index index, pack_key key, float *value);
int pack_val_by_index_as_string(pack_packet *pack, pack_index index, pack_key key, pack_string value);
int pack_val_by_index_as_bytes (pack_packet *pack, pack_index index, pack_key key, pack_bytes value, pack_size *size);
//==============================================================================
#endif //PROTOCOL_H
