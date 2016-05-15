//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: protocol_types.h
 */
//==============================================================================
#ifndef PROTOCOL_TYPES_H
#define PROTOCOL_TYPES_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "ncs_error.h"
#include "ncs_pack.h"
//==============================================================================
#ifdef DEMS_DEVICE
#define PACK_OUT_PACKETS_COUNT   1  //(1 * 60 * 10) // 1 minute
#define PACK_IN_PACKETS_COUNT    1  //(    10 * 10) // 10 seconds
#define PACK_QUEUE_COUNT         1
#else
#define PACK_OUT_PACKETS_COUNT   1
#define PACK_IN_PACKETS_COUNT    1
#define PACK_QUEUE_COUNT         1
#endif
//==============================================================================
#define PACK_QUEUE_INIT_INDEX    0
#define PACK_PACKETS_INIT_INDEX  0
//==============================================================================
#define PACK_QUEUE_EMPTY         0
#define PACK_QUEUE_FULL          1
//==============================================================================
#define PACK_IN                  0
#define PACK_OUT                 1
//==============================================================================
#define PACK_VALIDATE_STRUCT     0
#define PACK_VALIDATE_ONLY       1
#define PACK_VALIDATE_ADD        2
//==============================================================================
typedef int (*on_cmd_t)         (void *sender, void *data);
typedef int (*on_data_t)        (void *sender, void *data);
//==============================================================================
typedef pack_packet_t          *ppack_packet_t;
//==============================================================================
typedef pack_packet_t           pack_out_packets_t  [PACK_OUT_PACKETS_COUNT];
typedef pack_packet_t           pack_in_packets_t   [PACK_IN_PACKETS_COUNT];
typedef ppack_packet_t          pack_queue_packets_t[PACK_QUEUE_COUNT];
//==============================================================================
typedef struct
{
  pack_size_t              size;
  pack_buffer_t            buffer;
} pack_validation_buffer_t;
//==============================================================================
typedef struct
{
  BOOL                     empty;
  pack_index_t             index;
  pack_count_t             count;
  pack_count_t             lock_count;
  pack_out_packets_t       items;
} pack_out_packets_list_t;
//==============================================================================
typedef struct
{
  BOOL                     empty;
  pack_index_t             index;
  pack_count_t             count;
  pack_count_t             lock_count;
  pack_in_packets_t        items;
} pack_in_packets_list_t;
//==============================================================================
typedef struct
{
  pack_type_t              empty;
  pack_index_t             start;
  pack_index_t             finish;
  pack_queue_packets_t     packets;
} pack_queue_t;
//==============================================================================
typedef struct
{
  pack_index_t             id;
  pack_validation_buffer_t validation_buffer;
  pack_out_packets_list_t  out_packets_list;
  pack_in_packets_list_t   in_packets_list;
  #ifdef PACK_USE_OWN_QUEUE
  pack_queue_t             queue;
  #endif
  on_error_msg_t           on_error;
  on_cmd_t                 on_new_cmd;
  on_data_t                on_new_in_data;
  on_data_t                on_new_out_data;
} pack_protocol_t;
//==============================================================================
#endif // PROTOCOL_TYPES_H
