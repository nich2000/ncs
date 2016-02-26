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
typedef unsigned short           pack_crc16;
typedef unsigned char            pack_type;
//==============================================================================
#define PACK_SIZE_SIZE           sizeof(pack_size)
#define PACK_INDEX_SIZE          sizeof(pack_index)
#define PACK_NUMBER_SIZE         sizeof(pack_number)
#define PACK_TYPE_SIZE           sizeof(pack_type)
#define PACK_CRC_SIZE            sizeof(pack_crc16)
//==============================================================================
typedef struct
{
  pack_key   key;              // 4
  pack_value value;            // 12
  pack_type  type;             // 1
  char       _align1[3];       // 3
  pack_size  size;             // 2
  char       _align2[2];       // 2
} pack_word;
//==============================================================================
typedef pack_word pack_words[PACK_WORDS_COUNT];
//==============================================================================
typedef struct
{
  pack_number number;          // 2
  char       _align1[2];       // 2
  pack_size   words_count;     // 2
  char       _align2[2];       // 2
  pack_words  words;           // 24 * 20
} pack_packet;
//==============================================================================
typedef pack_packet  *ppack_packet;
//==============================================================================
typedef pack_packet  pack_out_packets  [PACK_OUT_PACKETS_COUNT];
typedef pack_packet  pack_in_packets   [PACK_IN_PACKETS_COUNT];
typedef ppack_packet pack_queue_packets[PACK_QUEUE_COUNT];
//==============================================================================
typedef struct
{
  pack_size   size;            // 2
  char        _align1[2];      // 2
  pack_buffer buffer;          // 256
} pack_validation_buffer;
//==============================================================================
typedef struct
{
  pack_type        empty;      // 1
  char            _align1[3];  // 3
  pack_index       index;      // 2
  char            _align2[2];  // 2
  pack_count       count;      // 2
  char            _align3[2];  // 2
  pack_count       lock_count; // 2
  char            _align4[2];  // 2
  pack_out_packets items;      // 488 * 8
} pack_out_packets_list;
//==============================================================================
typedef struct
{
  pack_type        empty;      // 1
  char            _align1[3];  // 3
  pack_index       index;      // 2
  char            _align2[2];  // 2
  pack_count       count;      // 2
  char            _align3[2];  // 2
  pack_count       lock_count; // 2
  pack_in_packets  items;      // 488 * 8
} pack_in_packets_list;
//==============================================================================
typedef struct
{
  pack_type          empty;      // 1
  char               _align1[3]; // 3
  pack_index         start;      // 2
  char               _align2[2]; // 2
  pack_index         finish;     // 2
  char               _align3[2]; // 2
  pack_queue_packets packets;    // 4
} pack_queue;
//==============================================================================
typedef struct
{
  pack_validation_buffer validation_buffer;
  pack_out_packets_list  out_packets_list;
  pack_in_packets_list   in_packets_list;
  #ifdef PACK_USE_OWN_QUEUE
  pack_queue             queue;
  #endif
} pack_protocol;
//==============================================================================
#endif // PROTOCOL_TYPES_H
