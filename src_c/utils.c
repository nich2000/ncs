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
const char *state_to_string(sock_state_t state)
{
  switch (state) {
  case STATE_NONE:
    return "STATE_NONE";
    break;
  case STATE_START:
    return "STATE_START";
    break;
    case STATE_STARTING:
      return "STATE_STARTING";
      break;
  case STATE_STOP:
    return "STATE_STOP";
    break;
    case STATE_STOPPING:
      return "STATE_STOPPING";
      break;
  case STATE_PAUSE:
    return "STATE_PAUSE";
    break;
    case STATE_PAUSING:
      return "STATE_PAUSING";
      break;
  default:
    return "STATE_UNKNOWN";
    break;
  }
}
//==============================================================================
int print_types_info()
{
  char tmp[1024];

  sprintf(
    tmp,
    #ifdef __linux__
    "\n"                                \
    "pack_word:                  %lu\n" \
    "pack_words:                 %lu\n" \
    "pack_packet:                %lu\n" \
    "pack_out_packets:           %lu\n" \
    "pack_in_packets:            %lu\n" \
    "pack_queue_packets:         %lu\n" \
    "pack_validation_buffer:     %lu\n" \
    "pack_out_packets_list:      %lu\n" \
    "pack_in_packets_list:       %lu\n" \
    "pack_protocol:              %lu\n" \
    "pack_queue:                 %lu\n" \
    "custom_worker:              %lu\n" \
    "custom_remote_clients_list: %lu",
    #elif _WIN32
    "\n"                               \
    "pack_word:                  %u\n" \
    "pack_words:                 %u\n" \
    "pack_packet:                %u\n" \
    "pack_out_packets:           %u\n" \
    "pack_in_packets:            %u\n" \
    "pack_queue_packets:         %u\n" \
    "pack_validation_buffer:     %u\n" \
    "pack_out_packets_list:      %u\n" \
    "pack_in_packets_list:       %u\n" \
    "pack_protocol:              %u\n" \
    "pack_queue:                 %u\n" \
    "custom_worker:              %u\n" \
    "custom_remote_clients_list: %u",
    #endif
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

  return ERROR_NONE;
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

  return ERROR_NONE;
}
//==============================================================================
int print_custom_worker_info(custom_worker_t *worker, char *prefix)
{
  if(worker == NULL)
    return 1;

  char tmp_info[1024];

  #ifdef PRINT_ALL_INFO
  sprintf(tmp_info,
          "%s\n"                                  \
          "addr:                            %d\n" \
          "id:                              %d\n" \
          "type:                            %s\n" \
          "mode:                            %s\n" \
          "port:                            %d\n" \
          "host:                            %s\n" \
          "sock:                            %d\n" \
          "state:                           %s\n" \
          "is_locked:                       %d\n",
          prefix,
          (int)&worker,
          worker->id,
          sock_type_to_string(worker->type),
          sock_mode_to_string(worker->mode),
          worker->port,
          worker->host,
          worker->sock,
          state_to_string(worker->state),
          worker->is_locked);
  #else
  sprintf(tmp_info,
          "%s\n"                                  \
          "id:                              %d\n" \
          "type:                            %s\n" \
          "mode:                            %s\n" \
          "state:                           %s\n",
          prefix,
          worker->id,
          sock_type_to_string(worker->type),
          sock_mode_to_string(worker->mode),
          state_to_string(worker->state));
  #endif

  log_add(tmp_info, LOG_INFO);

  return ERROR_NONE;
}
//==============================================================================
int print_custom_remote_clients_list_info(custom_remote_clients_list_t *clients_list, char *prefix)
{
  if(clients_list == 0)
    return 1;

  log_add_fmt(LOG_INFO, "clients count: %d", _custom_remote_clients_count(clients_list));

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_worker_t *tmp_worker = &clients_list->items[i].custom_worker;

    if(tmp_worker->state == STATE_START)
      print_custom_worker_info(tmp_worker, prefix);
  }

  return ERROR_NONE;
}
//==============================================================================
