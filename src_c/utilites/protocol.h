#ifndef PROTOCOL_H
#define PROTOCOL_H
//==============================================================================
// http://sigidin.blogspot.ru/2011/01/linux.html
// http://ru.stackoverflow.com/questions/376364/%D0%A7%D1%82%D0%BE-%D0%B4%D0%B5%D0%BB%D0%B0%D0%B5%D1%82-select-%D0%B8-fd-isset
// http://www.binarytides.com/code-tcp-socket-server-winsock/
// http://www.binarytides.com/server-client-example-c-sockets-linux/
//==============================================================================
#include "protocol_utils.h"
#include "protocol_types.h"
//==============================================================================
int            _pack_get_last_error       ();
pack_number_t  _pack_global_number        ();
const char  *  _pack_version              ();
//==============================================================================
#ifdef PACK_USE_OWN_BUFFER
  int pack_protocol_init();
  int pack_begin();
  int pack_end();
  pack_packet *_pack_pack_current(pack_type out);
  int pack_buffer_validate(pack_buffer buffer, pack_size size, pack_type only_validate);
  int pack_pack_current(pack_type out, pack_packet *pack);
  int pack_current_packet_to_buffer(pack_type out, pack_buffer buffer, pack_size *size);
  //------------------------------------------------------------------------------
  pack_packet *_pack_next();
  int pack_next_buffer(pack_buffer buffer, pack_size *size);
  //------------------------------------------------------------------------------
  int pack_add_as_int(pack_key key, int value);
  int pack_add_as_float(pack_key key, float value);
  int pack_add_as_string(pack_key key, pack_string value);
  int pack_add_as_bytes(pack_key key, pack_bytes value, pack_size size);
  //------------------------------------------------------------------------------
  int pack_add_cmd(pack_value command);
  int pack_add_param_as_int(int   param);
  int pack_add_param_as_float(float param);
  int pack_add_param_as_string(pack_string param);
  int pack_add_param_as_bytes(pack_bytes param, pack_size size);
#else
  int pack_protocol_init (pack_protocol_t *protocol);
  int pack_begin(pack_protocol_t *protocol);
  int pack_end  (pack_protocol_t *protocol);
  pack_packet_t *_pack_pack_current(pack_type_t out, pack_protocol_t *protocol);
  int pack_buffer_validate(pack_buffer_t buffer, pack_size_t size, pack_type_t only_validate, pack_protocol_t *protocol);
  int pack_pack_current(pack_type_t out, pack_packet_t *pack, pack_protocol_t *protocol);
  int pack_current_packet_to_buffer(pack_type_t out, pack_buffer_t buffer, pack_size_t *size, pack_protocol_t *protocol);
  //------------------------------------------------------------------------------
  int pack_packet_to_buffer(pack_packet_t *packet, pack_buffer_t buffer, pack_size_t *size);
  int pack_packet_to_json  (pack_packet_t *packet, pack_buffer_t buffer, pack_size_t *size);
  //------------------------------------------------------------------------------
  pack_packet_t *_pack_next(pack_protocol_t *protocol);
  int pack_next_buffer(pack_buffer_t buffer, pack_size_t *size, pack_protocol_t *protocol);
  //------------------------------------------------------------------------------
  int pack_add_as_int   (pack_key_t key, int         value, pack_protocol_t *protocol);
  int pack_add_as_float (pack_key_t key, float       value, pack_protocol_t *protocol);
  int pack_add_as_string(pack_key_t key, pack_string_t value, pack_protocol_t *protocol);
  int pack_add_as_bytes (pack_key_t key, pack_bytes_t  value, pack_size_t size, pack_protocol_t *protocol);
  //------------------------------------------------------------------------------
  int pack_add_cmd            (pack_value_t command, pack_protocol_t *protocol);
  int pack_add_param_as_int   (int   param,        pack_protocol_t *protocol);
  int pack_add_param_as_float (float param,        pack_protocol_t *protocol);
  int pack_add_param_as_string(pack_string_t param,  pack_protocol_t *protocol);
  int pack_add_param_as_bytes (pack_bytes_t param, pack_size_t size, pack_protocol_t *protocol);
#endif
//==============================================================================
pack_size_t _pack_words_count    (pack_packet_t *pack);
//==============================================================================
int       pack_command           (pack_packet_t *pack, pack_value_t command);
pack_size_t _pack_params_count   (pack_packet_t *pack);
//==============================================================================
int pack_param_by_index_as_string(pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_string_t value);
//==============================================================================
int pack_keys_to_csv             (pack_packet_t *pack, unsigned char delimeter, pack_buffer_t buffer);
int pack_values_to_csv           (pack_packet_t *pack, unsigned char delimeter, pack_buffer_t buffer);
//==============================================================================
int pack_word_by_key             (pack_packet_t *pack, pack_key_t key,     pack_index_t *index, pack_word_t *word);
int pack_word_by_index           (pack_packet_t *pack, pack_index_t index, pack_key_t key,      pack_word_t *word);
int pack_key_by_index            (pack_packet_t *pack, pack_index_t index, pack_key_t key);
//==============================================================================
int pack_val_by_key_as_int       (pack_packet_t *pack, pack_key_t key, pack_index_t *index, int   *value);
int pack_val_by_key_as_float     (pack_packet_t *pack, pack_key_t key, pack_index_t *index, float *value);
int pack_val_by_key_as_string    (pack_packet_t *pack, pack_key_t key, pack_index_t *index, pack_string_t value);
int pack_val_by_key_as_bytes     (pack_packet_t *pack, pack_key_t key, pack_index_t *index, pack_bytes_t value, pack_size_t *size);
//==============================================================================
int pack_val_by_index_as_int     (pack_packet_t *pack, pack_index_t index, pack_key_t key, int   *value);
int pack_val_by_index_as_float   (pack_packet_t *pack, pack_index_t index, pack_key_t key, float *value);
int pack_val_by_index_as_string  (pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_string_t value);
int pack_val_by_index_as_bytes   (pack_packet_t *pack, pack_index_t index, pack_key_t key, pack_bytes_t value, pack_size_t *size);
//==============================================================================
#endif //PROTOCOL_H
