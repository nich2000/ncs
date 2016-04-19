//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: ncs_pack_types.h
 */
//==============================================================================
#ifndef NCS_PACK_TYPES_H
#define NCS_PACK_TYPES_H
//==============================================================================
#include "defines.h"
#include "globals.h"
//==============================================================================
typedef union
{
  int  i;
  char buff[sizeof(int)];
} intUnion;
//==============================================================================
typedef union
{
  float f;
  char  buff[sizeof(float)];
} floatUnion;
//==============================================================================
#define PACK_KEY_SIZE            4
#define PACK_VERSION_SIZE        4
//==============================================================================
#ifdef DEMS_DEVICE
#define PACK_BUFFER_SIZE         256
#define PACK_VALUE_SIZE          12
#define PACK_WORDS_COUNT         20
#else
#define PACK_BUFFER_SIZE         1024
#define PACK_VALUE_SIZE          256
#define PACK_WORDS_COUNT         256 // 2048
#endif
//==============================================================================
#define PACK_WORD_NONE           0
#define PACK_WORD_INT            1
#define PACK_WORD_FLOAT          2
#define PACK_WORD_CHAR           3
#define PACK_WORD_STRING         4
#define PACK_WORD_BYTES          5
#define PACK_WORD_PACK           6
//==============================================================================
typedef unsigned char           *pack_string_t;
typedef unsigned char           *pack_bytes_t;
typedef unsigned char            pack_buffer_t[PACK_BUFFER_SIZE];
typedef unsigned char            pack_value_t [PACK_VALUE_SIZE];
typedef unsigned char            pack_ver_t   [PACK_VERSION_SIZE];
typedef unsigned char            pack_key_t   [PACK_KEY_SIZE];
typedef unsigned char            pack_delim_t;
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
  pack_number_t         number;
//  char               _align1[2];
  pack_size_t           words_count;
//  char               _align2[2];
  pack_words_t          words;
} pack_packet_t;
//==============================================================================
#endif //NCS_PACK_TYPES_H
