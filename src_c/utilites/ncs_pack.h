#ifndef PACKET_H
#define PACKET_H
//==============================================================================
#include "defines.h"
#include "globals.h"
//==============================================================================
#define PACK_GLOBAL_INIT_NUMBER  0
//==============================================================================
#define PACK_VERSION             "V01\0"
#define PACK_CMD_KEY             "CMD\0"
#define PACK_PARAM_KEY           "PAR\0"
//==============================================================================
#ifdef DEMS_DEVICE
#define PACK_BUFFER_SIZE         256
#define PACK_VALUE_SIZE          12
#define PACK_WORDS_COUNT         20
#else
#define PACK_BUFFER_SIZE         102400
#define PACK_VALUE_SIZE          128
#define PACK_WORDS_COUNT         32
#endif
//==============================================================================
#define PACK_KEY_SIZE            4
#define PACK_VERSION_SIZE        4
//==============================================================================
#define PACK_WORD_NONE           0
#define PACK_WORD_INT            1
#define PACK_WORD_FLOAT          2
#define PACK_WORD_STRING         3
#define PACK_WORD_BYTES          4
//==============================================================================
typedef unsigned char                    *pack_string_t;
typedef unsigned char                    *pack_bytes_t;
typedef unsigned char                     pack_buffer_t[PACK_BUFFER_SIZE];
typedef unsigned char                     pack_value_t [PACK_VALUE_SIZE];
typedef unsigned char                     pack_ver_t   [PACK_VERSION_SIZE];
typedef unsigned char                     pack_key_t   [PACK_KEY_SIZE];
typedef unsigned char                     pack_delim_t;
//==============================================================================
typedef unsigned short           pack_count_t;
typedef unsigned short           pack_size_t;
typedef unsigned short           pack_index_t;
typedef unsigned short           pack_number_t;
typedef unsigned short           pack_crc16_t;
typedef unsigned char            pack_type_t;
//==============================================================================
#define PACK_SIZE_SIZE           sizeof(pack_size_t)
#define PACK_INDEX_SIZE          sizeof(pack_index_t)
#define PACK_NUMBER_SIZE         sizeof(pack_number_t)
#define PACK_TYPE_SIZE           sizeof(pack_type_t)
#define PACK_CRC_SIZE            sizeof(pack_crc16_t)
//==============================================================================
typedef struct
{
  pack_key_t            key;
  pack_value_t          value;
  pack_type_t           type;
//  char               _align1[3];
  pack_size_t           size;
//  char               _align2[2];
} pack_word_t;
//==============================================================================
typedef pack_word_t pack_words_t[PACK_WORDS_COUNT];
//==============================================================================
typedef struct
{
  pack_number_t number;
//  char               _align1[2];
  pack_size_t           words_count;
//  char               _align2[2];
  pack_words_t          words;
} pack_packet_t;
//==============================================================================
const char *_pack_version();
//==============================================================================
pack_packet_t *packet();
//==============================================================================
int pack_init                    (pack_packet_t *packet);
//==============================================================================
pack_size_t _pack_words_count    (pack_packet_t *pack);
//==============================================================================
int pack_word_as_string          (pack_word_t *word, pack_string_t value);
//==============================================================================
int pack_add_as_int              (pack_packet_t *pack, pack_key_t key, int value);
int pack_add_as_float            (pack_packet_t *pack, pack_key_t key, float value);
int pack_add_as_string           (pack_packet_t *pack, pack_key_t key, pack_string_t value);
int pack_add_as_bytes            (pack_packet_t *pack, pack_key_t key, pack_bytes_t value, pack_size_t size);
//==============================================================================
int pack_assign_pack             (pack_packet_t *dst, pack_packet_t *src);
//==============================================================================
int pack_val_by_index_as_int     (pack_packet_t *pack, pack_index_t index, pack_key_t key, int   *value);
int pack_val_by_index_as_float   (pack_packet_t *pack, pack_index_t index, pack_key_t key, float *value);
int pack_val_by_index_as_string  (pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_string_t value);
int pack_val_by_index_as_bytes   (pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_bytes_t value, pack_size_t *size);
//==============================================================================
int pack_to_buffer               (pack_packet_t *pack, pack_buffer_t buffer, pack_size_t *size);
int pack_to_json                 (pack_packet_t *pack, pack_buffer_t buffer);
//==============================================================================
int pack_keys_to_csv             (pack_packet_t *pack, pack_delim_t delimeter, pack_buffer_t buffer);
int pack_values_to_csv           (pack_packet_t *pack, pack_delim_t delimeter, pack_buffer_t buffer);
//==============================================================================
int pack_add_cmd                 (pack_packet_t *pack, pack_string_t command);
int pack_add_param               (pack_packet_t *pack, pack_string_t param);
//==============================================================================
BOOL _pack_is_command            (pack_packet_t *pack);
int pack_command                 (pack_packet_t *pack, pack_value_t command);
//==============================================================================
int            pack_next_param   (pack_packet_t *pack, pack_index_t *index, pack_string_t value);
pack_string_t _pack_next_param   (pack_packet_t *pack, pack_index_t *index);
//==============================================================================
int pack_buffer_to_words(pack_buffer_t buffer, pack_size_t buffer_size, pack_words_t words, pack_size_t *words_count);
//==============================================================================
#endif //PACKET_H
