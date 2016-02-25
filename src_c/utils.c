//==============================================================================
//==============================================================================
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "socket_utils.h"
#include "log.h"
//==============================================================================
int print_types_info()
{
  char tmp[1024];

  sprintf(
    tmp,
    "\n"                           \
    "pack_word:              %d\n" \
    "pack_words:             %d\n" \
    "pack_packet:            %d\n" \
    "pack_out_packets:       %d\n" \
    "pack_in_packets:        %d\n" \
    "pack_queue_packets:     %d\n" \
    "pack_validation_buffer: %d\n" \
    "pack_out_packets_list:  %d\n" \
    "pack_in_packets_list:   %d\n" \
    "pack_protocol:          %d\n" \
    "pack_queue:             %d\n" \
    "sock_worker:            %d\n" \
    "sock_worker_list:       %d",
    sizeof(pack_word),
    sizeof(pack_words),
    sizeof(pack_packet),
    sizeof(pack_out_packets),
    sizeof(pack_in_packets),
    sizeof(pack_queue_packets),
    sizeof(pack_validation_buffer),
    sizeof(pack_out_packets_list),
    sizeof(pack_in_packets_list),
    sizeof(pack_protocol),
    sizeof(pack_queue),
    sizeof(sock_worker_t),
    sizeof(sock_worker_list_t)
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
    "SOCK_SERVER_STREAMER:   %d\n" \
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
    SOCK_SERVER_STREAMER,
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
int print_worker_info(sock_worker_t *worker, char *prefix)
{
  if(worker == NULL)
    return 1;

  char tmp_info[1024];

  sprintf(tmp_info,
          "%s\n"                                  \
          "addr:                            %d\n" \
          "id:                              %d\n" \
          "type:                            %s\n" \
          "mode:                            %s\n" \
          "port:                            %d\n" \
          "host:                            %s\n" \
          "sock:                            %d\n" \
          "is_active:                       %d\n" \
          "protocol.in_packets_list.index:  %d\n" \
          "protocol.in_packets_list.count:  %d\n" \
          "protocol.out_packets_list.index: %d\n" \
          "protocol.out_packets_list.count: %d",
          prefix,
          &worker,
          worker->id,
          sock_type_to_string(worker->type),
          sock_mode_to_string(worker->mode),
          worker->port,
          worker->host,
          worker->sock,
          worker->is_active,
          worker->protocol.in_packets_list.index,
          worker->protocol.in_packets_list.count,
          worker->protocol.out_packets_list.index,
          worker->protocol.out_packets_list.count);

  log_add(tmp_info, LOG_INFO);

  return 0;
}
//==============================================================================
int print_server_info(sock_server_t *server)
{
  if(server == NULL)
    return 1;

  clr_scr();

  char tmp[256];

  sprintf(tmp,
          "server\n"                              \
          "addr:                            %u\n" \
          "clients.last_id:                 %d\n" \
          "clients.index:                   %d",
          server,
          server->clients.last_id,
          server->clients.index);
  log_add(tmp, LOG_INFO);

  print_worker_info(&server->worker, "server");

  for(int i = 0; i < server->clients.index; i++)
  {
    sock_worker_t tmp_worker = server->clients.items[i];
    print_worker_info(&tmp_worker, "remote client");
  }

  return 0;
}//==============================================================================
