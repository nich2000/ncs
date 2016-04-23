//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: utils.c
 */
//==============================================================================
// for random_range
#include <stdlib.h>

#include "utils.h"

#include "ncs_log.h"
//==============================================================================
#ifndef __linux__
/**
 * getline.c
 * Copyright (C) 1991 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 * Read up to (and including) a newline from STREAM into *LINEPTR
 * (and null-terminate it). *LINEPTR is a pointer returned from malloc (or
 * NULL), pointing to *N characters of space.  It is realloc'd as
 * necessary.  Returns the number of characters read (not including the
 * null terminator), or -1 on error or EOF.
 */
ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
  static char line[256];
  char *ptr;
  unsigned int len;

  if (lineptr == NULL || n == NULL)
  {
    errno = EINVAL;
    return -1;
  }

  if (ferror (stream))
    return -1;

  if (feof(stream))
    return -1;

  fgets(line,256,stream);

  ptr = strchr(line,'\n');
  if (ptr)
    *ptr = '\0';

  len = strlen(line);

  if ((len+1) < 256)
  {
    ptr = realloc(*lineptr, 256);
    if (ptr == NULL)
      return(-1);
    *lineptr = ptr;
    *n = 256;
  }

  strcpy(*lineptr,line);
  return(len);
}
#endif
//==============================================================================
int random_range(int min, int max)
{
  return (rand() % (max + 1 - min)) + min;
}
//==============================================================================
const char *state_to_string(sock_state_t value)
{
  switch (value)
  {
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

    case STATE_RESUME:
      return "STATE_RESUME";
      break;

    case STATE_RESUMING:
      return "STATE_RESUMING";
      break;

    case STATE_STEP:
      return "STATE_STEP";
      break;

    default:
      return "STATE_UNKNOWN";
      break;
  }
}
//==============================================================================
const char *active_to_string(sock_active_t value)
{
  switch (value)
  {
    case ACTIVE_NONE:
      return "ACTIVE_NONE";
      break;

    case ACTIVE_FIRST:
      return "ACTIVE_FIRST";
      break;

    case ACTIVE_SECOND:
      return "ACTIVE_SECOND";
       break;

    default:
      return "ACTIVE_UNKNOWN";
      break;
  }
}
//==============================================================================
const char *time_to_string(sock_time_t value)
{
  if(!value)
    return "no time";
  else
    return asctime(localtime(&value));
}
//==============================================================================
const char *register_to_string(sock_register_t value)
{
  switch (value)
  {
    case REGISTER_NONE:
      return "REGISTER_NONE";
      break;

    case REGISTER_OK:
      return "REGISTER_OK";
      break;

    default:
      return "REGISTER_UNKNOWN";
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
  log_add(LOG_INFO, tmp);

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
  log_add(LOG_INFO, tmp);

  return ERROR_NONE;
}
//==============================================================================
const char *sock_mode_to_string(sock_mode_t mode)
{
  switch (mode)
  {
    case SOCK_MODE_UNKNOWN:
      return "SOCK_MODE_UNKNOWN";
      break;
    case SOCK_MODE_CMD_CLIENT:
      return "SOCK_MODE_CMD_CLIENT";
      break;
    case SOCK_MODE_CMD_SERVER:
      return "SOCK_MODE_CMD_SERVER";
      break;
    case SOCK_MODE_WS_SERVER:
      return "SOCK_MODE_WS_SERVER";
      break;
    case SOCK_MODE_WEB_SERVER:
      return "SOCK_MODE_WEB_SERVER";
      break;
    default:
      return "SOCK_MODE_UNKNOWN";
      break;
  }
}
//==============================================================================
const char *sock_type_to_string(sock_type_t type)
{
  switch (type)
  {
    case SOCK_TYPE_UNKNOWN:
      return "SOCK_TYPE_UNKNOWN";
      break;
    case SOCK_TYPE_CLIENT:
      return "SOCK_TYPE_CLIENT";
      break;
    case SOCK_TYPE_SERVER:
      return "SOCK_TYPE_SERVER";
      break;
    case SOCK_TYPE_REMOTE_CLIENT:
      return "SOCK_TYPE_REMOTE_CLIENT";
      break;
    default:
      return "SOCK_TYPE_UNKNOWN";
      break;
  }
}
//==============================================================================
