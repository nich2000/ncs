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
