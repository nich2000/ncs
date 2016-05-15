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
#define PACK_VERSION             "V01\0"
#define PACK_VERSION_SIZE        4
//==============================================================================
#define PACK_KEY_SIZE            4
//==============================================================================
#ifdef DEMS_DEVICE
#define PACK_BUFFER_SIZE         256
#define PACK_VALUE_SIZE          12
#define PACK_WORDS_COUNT         20
#else
#define PACK_BUFFER_SIZE         1024
#define PACK_VALUE_SIZE          256
#define PACK_WORDS_COUNT         2048 //256 //TODO: разобраться с кол-вом слов в пакете
#endif
//==============================================================================
#define PACK_WORD_NONE           0
#define PACK_WORD_INT            1
#define PACK_WORD_FLOAT          2
#define PACK_WORD_CHAR           3
#define PACK_WORD_BYTE           4
#define PACK_WORD_BOOL           5
#define PACK_WORD_STRING         6
#define PACK_WORD_BYTES          7
#define PACK_WORD_PACK           8
//==============================================================================
#define PACK_FLAG_NONE           0x00
#define PACK_FLAG_BIN            0x01
#define PACK_FLAG_TXT            0x02
#define PACK_FLAG_CMD            0x04
#define PACK_FLAG_DATA           0x08
//==============================================================================
#define PACK_FLAG_DEFAULT        (PACK_FLAG_BIN | PACK_FLAG_CMD)
//==============================================================================
typedef unsigned char           *pack_string_t;
typedef unsigned char           *pack_bytes_t;
typedef unsigned char            pack_buffer_t[PACK_BUFFER_SIZE];
typedef unsigned char            pack_value_t [PACK_VALUE_SIZE];
typedef unsigned char            pack_key_t   [PACK_KEY_SIZE];
typedef unsigned char            pack_delim_t;
typedef unsigned short           pack_count_t;
typedef unsigned short           pack_size_t;
typedef unsigned short           pack_index_t;
typedef unsigned short           pack_number_t;
typedef unsigned short           pack_crc16_t;
typedef unsigned char            pack_type_t;
typedef unsigned char            pack_flag_t;
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
  pack_size_t           size;
} pack_word_t;
//==============================================================================
typedef pack_word_t pack_words_t[PACK_WORDS_COUNT];
//==============================================================================
typedef struct
{
  pack_flag_t           flag;
  pack_number_t         number;
  pack_size_t           words_count;
  pack_words_t          words;
} pack_packet_t;
//==============================================================================
typedef union
{
  int  i;
  char buff[sizeof(int)];
} int_union;
//==============================================================================
typedef union
{
  float f;
  char  buff[sizeof(float)];
} float_union;
//==============================================================================
#endif //NCS_PACK_TYPES_H
