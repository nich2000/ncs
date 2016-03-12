//==============================================================================
//==============================================================================
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "socket_utils.h"
#include "protocol_types.h"
#include "ncs_log.h"
//==============================================================================
int random_range(int min, int max)
{
  int val = (rand() % (max + 1 - min)) + min;

  return val;
}
//==============================================================================
int print_types_info()
{
  char tmp[1024];

  sprintf(
    tmp,
    "\n"                               \
    "pack_word:                  %d\n" \
    "pack_words:                 %d\n" \
    "pack_packet:                %d\n" \
    "pack_out_packets:           %d\n" \
    "pack_in_packets:            %d\n" \
    "pack_queue_packets:         %d\n" \
    "pack_validation_buffer:     %d\n" \
    "pack_out_packets_list:      %d\n" \
    "pack_in_packets_list:       %d\n" \
    "pack_protocol:              %d\n" \
    "pack_queue:                 %d\n" \
    "custom_worker:              %d\n" \
    "custom_remote_clients_list: %d",
    sizeof(pack_word_t),
    sizeof(pack_words_t),
    sizeof(pack_packet_t),
    sizeof(pack_out_packets_t),
    sizeof(pack_in_packets_t),
    sizeof(pack_queue_packets_t),
    sizeof(pack_validation_buffer_t),
    sizeof(pack_out_packets_list_t),
    sizeof(pack_in_packets_list_t),
    sizeof(pack_protocol_t),
    sizeof(pack_queue_t),
    sizeof(custom_worker_t),
    sizeof(custom_remote_clients_list_t)
  );
  log_add(tmp, LOG_INFO);

  return 0;
}
//==============================================================================
int print_defines_info()
{
  char tmp[1024];

  sprintf(
    tmp,
    "\n"                           \
    "PACK_VERSION:           %s\n" \
    "PACK_BUFFER_SIZE:       %d\n" \
    "PACK_VALUE_SIZE:        %d\n" \
    "PACK_WORDS_COUNT:       %d\n" \
    "PACK_OUT_PACKETS_COUNT: %d\n" \
    "PACK_IN_PACKETS_COUNT:  %d\n" \
    "PACK_QUEUE_COUNT:       %d\n" \
    "SOCK_VERSION:           %s\n" \
    "SOCK_BUFFER_SIZE:       %d\n" \
    "SOCK_WORKERS_COUNT:     %d\n" \
    "SOCK_ERRORS_COUNT:      %d\n" \
    "SOCK_WAIT_SELECT:       %d\n" \
    "SOCK_WAIT_CONNECT:      %d",
    PACK_VERSION,
    PACK_BUFFER_SIZE,
    PACK_VALUE_SIZE,
    PACK_WORDS_COUNT,
    PACK_OUT_PACKETS_COUNT,
    PACK_IN_PACKETS_COUNT,
    PACK_QUEUE_COUNT,
    SOCK_VERSION,
    SOCK_BUFFER_SIZE,
    SOCK_WORKERS_COUNT,
    SOCK_ERRORS_COUNT,
    SOCK_WAIT_SELECT,
    SOCK_WAIT_CONNECT
  );
  log_add(tmp, LOG_INFO);

  return 0;
}
//==============================================================================
unsigned char nibbles[] = {"0123456789ABCDEF"};
int bytes_to_hex(unsigned char *bytes, int size, unsigned char *hex)
{
  int j = 0;

  for(int i = 0; i < size; i++)
  {
    hex[j++] = nibbles[bytes[i] >> 0x04];
    hex[j++] = nibbles[bytes[i] &  0x0F];
    hex[j++] = ' ';
  }

  hex[j] = '0';

  return 0;
}
//==============================================================================
//int bytes_from_int  (unsigned char *bytes, size_t size, int   *value);
//int bytes_from_float(unsigned char *bytes, size_t size, float *value);
//==============================================================================
int pack_print(pack_packet_t *packet, char *prefix, int clear, int buffer, int pack, int csv)
{
  if(clear)
    clr_scr();

  if(!buffer && !pack && !csv)
    return ERROR_NONE;

  char tmp[1024];

  if(buffer)
  {
    pack_buffer_t  tmp_buffer;
    pack_size_t    tmp_size;

    pack_to_buffer(packet, tmp_buffer, &tmp_size);

    #ifdef SOCK_PACK_MODE
    bytes_to_hex((unsigned char*)tmp_buffer, (pack_size_t)tmp_size, (unsigned char*)tmp);
    log_add(tmp, LOG_DEBUG);
    #else
    log_add(tmp_buffer, LOG_DEBUG);
    #endif
  }

  if(pack)
  {
    sprintf(tmp, "%s\n", prefix);
//      sprintf(tmp, "%sNumber: %d\n", tmp, tmp_pack->number);
    pack_key_t     key;
    pack_value_t   valueS;
    pack_size_t tmp_words_count = _pack_words_count(packet);
//      sprintf(tmp, "%sWords: %d\n", tmp, tmp_words_count);
    for(pack_size_t i = 0; i < tmp_words_count; i++)
      if(pack_val_by_index_as_string(packet, i, key, valueS) == ERROR_NONE)
        sprintf(tmp, "%s%s: %s\n", tmp, key, valueS);
    log_add(tmp, LOG_INFO);
  }

  if(csv)
  {
    pack_buffer_t csv;
    pack_values_to_csv(packet, ';', csv);
//    report_add(csv);
  }

  return ERROR_NONE;
}
//==============================================================================
