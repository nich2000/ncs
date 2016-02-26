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
int         _pack_get_last_error       ();
pack_number _pack_global_number        ();
int          pack_version              (pack_ver version);
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
  int pack_protocol_init (pack_protocol *protocol);
  int pack_begin(pack_protocol *protocol);
  int pack_end  (pack_protocol *protocol);
  pack_packet *_pack_pack_current(pack_type out, pack_protocol *protocol);
  int pack_buffer_validate(pack_buffer buffer, pack_size size, pack_type only_validate, pack_protocol *protocol);
  int pack_pack_current(pack_type out, pack_packet *pack, pack_protocol *protocol);
  int pack_current_packet_to_buffer(pack_type out, pack_buffer buffer, pack_size *size, pack_protocol *protocol);
  //------------------------------------------------------------------------------
  int pack_packet_to_buffer(pack_packet *packet, pack_buffer buffer, pack_size *size);
  int pack_packet_to_json  (pack_packet *packet, pack_buffer buffer, pack_size *size);
  //------------------------------------------------------------------------------
  pack_packet *_pack_next(pack_protocol *protocol);
  int pack_next_buffer(pack_buffer buffer, pack_size *size, pack_protocol *protocol);
  //------------------------------------------------------------------------------
  int pack_add_as_int   (pack_key key, int         value, pack_protocol *protocol);
  int pack_add_as_float (pack_key key, float       value, pack_protocol *protocol);
  int pack_add_as_string(pack_key key, pack_string value, pack_protocol *protocol);
  int pack_add_as_bytes (pack_key key, pack_bytes  value, pack_size size, pack_protocol *protocol);
  //------------------------------------------------------------------------------
  int pack_add_cmd            (pack_value command, pack_protocol *protocol);
  int pack_add_param_as_int   (int   param,        pack_protocol *protocol);
  int pack_add_param_as_float (float param,        pack_protocol *protocol);
  int pack_add_param_as_string(pack_string param,  pack_protocol *protocol);
  int pack_add_param_as_bytes (pack_bytes param, pack_size size, pack_protocol *protocol);
#endif
//==============================================================================
pack_size _pack_words_count    (pack_packet *pack);
//==============================================================================
int       pack_command         (pack_packet *pack, pack_value command);
pack_size _pack_params_count   (pack_packet *pack);
//==============================================================================
int pack_param_by_index_as_string(pack_packet *pack, pack_index index, pack_key key, pack_string value);
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
