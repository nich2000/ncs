//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
 */
//==============================================================================
#ifndef PROTOCOL_H
#define PROTOCOL_H
//==============================================================================
// http://sigidin.blogspot.ru/2011/01/linux.html
// http://ru.stackoverflow.com/questions/376364/%D0%A7%D1%82%D0%BE-%D0%B4%D0%B5%D0%BB%D0%B0%D0%B5%D1%82-select-%D0%B8-fd-isset
// http://www.binarytides.com/code-tcp-socket-server-winsock/
// http://www.binarytides.com/server-client-example-c-sockets-linux/
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "protocol_types.h"
#include "protocol_utils.h"
//==============================================================================
int protocol_init                        (pack_protocol_t *protocol);
int protocol_begin                       (pack_protocol_t *protocol);
int protocol_end                         (pack_protocol_t *protocol);
//==============================================================================
int protocol_bin_buffer_validate(pack_buffer_t buffer,
                                 pack_size_t size,
                                 pack_type_t only_validate,
                                 pack_protocol_t *protocol,
                                 void *sender);
//==============================================================================
int protocol_txt_buffer_validate(pack_buffer_t buffer,
                                 pack_size_t size,
                                 pack_type_t only_validate,
                                 pack_protocol_t *protocol,
                                 void *sender);
//==============================================================================
pack_packet_t *_protocol_next_pack(pack_protocol_t *protocol);
//==============================================================================
int protocol_add_as_int                  (pack_key_t key, int            value,                   pack_protocol_t *protocol);
int protocol_add_as_float                (pack_key_t key, float          value,                   pack_protocol_t *protocol);
int protocol_add_as_char                 (pack_key_t key, char           value,                   pack_protocol_t *protocol);
int protocol_add_as_string               (pack_key_t key, pack_string_t  value,                   pack_protocol_t *protocol);
int protocol_add_as_bytes                (pack_key_t key, pack_bytes_t   value, pack_size_t size, pack_protocol_t *protocol);
int protocol_add_as_pack                 (pack_key_t key, pack_packet_t *value,                   pack_protocol_t *protocol);
//==============================================================================
int protocol_add_cmd                     (pack_value_t command, pack_protocol_t *protocol);
//==============================================================================
int protocol_add_param_as_int            (int   param, pack_protocol_t *protocol);
int protocol_add_param_as_float          (float param, pack_protocol_t *protocol);
int protocol_add_param_as_char           (char  param, pack_protocol_t *protocol);
int protocol_add_param_as_string         (pack_string_t param, pack_protocol_t *protocol);
int protocol_add_param_as_bytes          (pack_bytes_t param, pack_size_t size, pack_protocol_t *protocol);
//==============================================================================
int protocol_assign_pack                 (pack_protocol_t *protocol, pack_packet_t *pack);
//==============================================================================
pack_packet_t *_protocol_current_pack    (pack_type_t out, pack_protocol_t *protocol);
//==============================================================================
#endif //PROTOCOL_H
