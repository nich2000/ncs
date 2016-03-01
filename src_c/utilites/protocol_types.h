#ifndef PROTOCOL_TYPES_H
#define PROTOCOL_TYPES_H
//==============================================================================
#ifdef DEMS_DEVICE
#define PACK_BUFFER_SIZE         256
#define PACK_VALUE_SIZE          12
#define PACK_WORDS_COUNT         20
#define PACK_OUT_PACKETS_COUNT   1  //(1 * 60 * 10) // 1 minute
#define PACK_IN_PACKETS_COUNT    1  //(    10 * 10) // 10 seconds
#define PACK_QUEUE_COUNT         1
#else
#define PACK_BUFFER_SIZE         1024
#define PACK_VALUE_SIZE          128
#define PACK_WORDS_COUNT         32
#define PACK_OUT_PACKETS_COUNT   1
#define PACK_IN_PACKETS_COUNT    1
#define PACK_QUEUE_COUNT         1
#endif
//==============================================================================
#define PACK_KEY_SIZE            4
#define PACK_VERSION_SIZE        4
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
#define PACK_VALIDATE_ONLY       1
#define PACK_VALIDATE_ADD        0
//==============================================================================
#define PACK_WORD_NONE           0
#define PACK_WORD_INT            1
#define PACK_WORD_FLOAT          2
#define PACK_WORD_STRING         3
#define PACK_WORD_BYTES          4
//==============================================================================
typedef unsigned char           *pack_string_t;
typedef unsigned char           *pack_bytes_t;
typedef unsigned char            pack_buffer_t[PACK_BUFFER_SIZE];
typedef unsigned char            pack_value_t [PACK_VALUE_SIZE];
typedef unsigned char            pack_ver_t   [PACK_VERSION_SIZE];
typedef unsigned char            pack_key_t   [PACK_KEY_SIZE];
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
  pack_key_t   key;              // 4
  pack_value_t value;            // 12
  pack_type_t  type;             // 1
  char       _align1[3];       // 3
  pack_size_t  size;             // 2
  char       _align2[2];       // 2
} pack_word_t;
//==============================================================================
typedef pack_word_t pack_words_t[PACK_WORDS_COUNT];
//==============================================================================
typedef struct
{
  pack_number_t number;          // 2
  char       _align1[2];       // 2
  pack_size_t   words_count;     // 2
  char       _align2[2];       // 2
  pack_words_t  words;           // 24 * 20
} pack_packet_t;
//==============================================================================
typedef pack_packet_t  *ppack_packet_t;
//==============================================================================
typedef pack_packet_t  pack_out_packets_t  [PACK_OUT_PACKETS_COUNT];
typedef pack_packet_t  pack_in_packets_t   [PACK_IN_PACKETS_COUNT];
typedef ppack_packet_t pack_queue_packets_t[PACK_QUEUE_COUNT];
//==============================================================================
typedef struct
{
  pack_size_t   size;            // 2
  char        _align1[2];      // 2
  pack_buffer_t buffer;          // 256
} pack_validation_buffer_t;
//==============================================================================
typedef struct
{
  pack_type_t        empty;      // 1
  char            _align1[3];  // 3
  pack_index_t       index;      // 2
  char            _align2[2];  // 2
  pack_count_t       count;      // 2
  char            _align3[2];  // 2
  pack_count_t       lock_count; // 2
  char            _align4[2];  // 2
  pack_out_packets_t items;      // 488 * 8
} pack_out_packets_list_t;
//==============================================================================
typedef struct
{
  pack_type_t        empty;      // 1
  char            _align1[3];  // 3
  pack_index_t       index;      // 2
  char            _align2[2];  // 2
  pack_count_t       count;      // 2
  char            _align3[2];  // 2
  pack_count_t       lock_count; // 2
  pack_in_packets_t  items;      // 488 * 8
} pack_in_packets_list_t;
//==============================================================================
typedef struct
{
  pack_type_t          empty;      // 1
  char               _align1[3]; // 3
  pack_index_t         start;      // 2
  char               _align2[2]; // 2
  pack_index_t         finish;     // 2
  char               _align3[2]; // 2
  pack_queue_packets_t packets;    // 4
} pack_queue_t;
//==============================================================================
typedef struct
{
  pack_validation_buffer_t validation_buffer;
  pack_out_packets_list_t  out_packets_list;
  pack_in_packets_list_t   in_packets_list;
  #ifdef PACK_USE_OWN_QUEUE
  pack_queue_t             queue;
  #endif
} pack_protocol_t;
//==============================================================================
#endif // PROTOCOL_TYPES_H
