//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: ncs_pack.h
 */
//==============================================================================
#ifndef PACKET_H
#define PACKET_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "ncs_pack_types.h"
//==============================================================================
#define PACK_GLOBAL_INIT_NUMBER  0
//==============================================================================
#define PACK_CMD_KEY             "CMD\0"
#define PACK_PARAM_KEY           "PAR\0"
//==============================================================================
unsigned short getCRC16(char *data, unsigned short length);
//==============================================================================
int bytes_to_int    (unsigned char *bytes, int   *value);
int bytes_to_float  (unsigned char *bytes, float *value);
int bytes_to_hex    (unsigned char *bytes, int size, unsigned char *hex);
//==============================================================================
int bytes_from_int  (unsigned char *bytes, int   value);
int bytes_from_float(unsigned char *bytes, float value);
//==============================================================================
const char *_pack_version();
//==============================================================================
pack_packet_t *packet();
//==============================================================================
int pack_init                    (pack_packet_t *packet);
//==============================================================================
pack_size_t pack_words_count     (pack_packet_t *pack);
//==============================================================================
pack_flag_t pack_flag            (pack_packet_t *pack);
void        pack_set_flag        (pack_packet_t *pack, pack_flag_t flag);
BOOL        pack_is_set_flag     (pack_packet_t *pack, pack_flag_t flag);
//==============================================================================
int pack_word_as_string          (pack_word_t *word, pack_value_t value);
//==============================================================================
int pack_add_as_int              (pack_packet_t *pack, pack_key_t key, int value);
int pack_add_as_float            (pack_packet_t *pack, pack_key_t key, float value);
int pack_add_as_char             (pack_packet_t *pack, pack_key_t key, char value);
int pack_add_as_string           (pack_packet_t *pack, pack_key_t key, pack_string_t value);
int pack_add_as_bytes            (pack_packet_t *pack, pack_key_t key, pack_bytes_t value, pack_size_t size, pack_type_t type);
int pack_add_as_pack             (pack_packet_t *pack, pack_key_t key, pack_packet_t *inner_pack);
//==============================================================================
int pack_insert_as_int           (pack_packet_t *pack, pack_index_t index, pack_key_t key, int value);
//==============================================================================
int pack_set_as_int              (pack_packet_t *pack, pack_index_t index, pack_key_t key, int value);
//==============================================================================
int pack_assign_pack             (pack_packet_t *dst, pack_packet_t *src);
//==============================================================================
int pack_val_by_index_as_int     (pack_packet_t *pack, pack_index_t index, pack_key_t key, int           *value);
int pack_val_by_index_as_float   (pack_packet_t *pack, pack_index_t index, pack_key_t key, float         *value);
int pack_val_by_index_as_char    (pack_packet_t *pack, pack_index_t index, pack_key_t key, char          *value);
int pack_val_by_index_as_string  (pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_string_t  value);
int pack_val_by_index_as_bytes   (pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_bytes_t   value, pack_size_t *size);
int pack_val_by_index_as_pack    (pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_packet_t *value);
//==============================================================================
int pack_to_buffer_bin           (pack_packet_t *pack, pack_buffer_t buffer, pack_size_t *size);
int pack_to_buffer_txt           (pack_packet_t *pack, pack_buffer_t buffer, pack_size_t *size);
int pack_to_bytes                (pack_packet_t *pack, pack_buffer_t buffer, pack_size_t *size);
//==============================================================================
int pack_keys_to_csv             (pack_packet_t *pack, pack_delim_t delimeter, pack_buffer_t buffer);
int pack_values_to_csv           (pack_packet_t *pack, pack_delim_t delimeter, pack_buffer_t buffer);
//==============================================================================
int pack_add_cmd                 (pack_packet_t *pack, const pack_string_t command);
int pack_add_param               (pack_packet_t *pack, const pack_string_t param);
//==============================================================================
BOOL _pack_is_command            (pack_packet_t *pack);
int pack_command                 (pack_packet_t *pack, pack_value_t command);
//==============================================================================
int            pack_next_param   (pack_packet_t *pack, pack_index_t *index, pack_string_t value);
pack_string_t _pack_next_param   (pack_packet_t *pack, pack_index_t *index);
//==============================================================================
int pack_word_as_pack            (pack_word_t *word, pack_packet_t *pack);
//==============================================================================
int pack_buffer_to_pack          (pack_buffer_t buffer, pack_size_t size, pack_packet_t *pack);
int pack_buffer_to_words         (pack_buffer_t buffer, pack_size_t buffer_size, pack_words_t words, pack_size_t *words_count);
//==============================================================================
#endif //PACKET_H
