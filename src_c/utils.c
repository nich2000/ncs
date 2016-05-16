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
#include "exec.h"
//==============================================================================
extern sock_port_t cmd_server_port;
extern sock_port_t ws_server_port;
extern sock_port_t web_server_port;
extern sock_host_t cmd_server_host;
extern BOOL session_relay_to_web;
extern BOOL log_enable;
extern char log_path[256];
extern BOOL stat_enable;
extern char stat_path[256];
extern BOOL report_enable;
extern char report_path[265];
extern BOOL session_enable;
extern char session_path[256];
extern BOOL session_stream_enable;
extern char session_stream_file[64];
extern BOOL map_enable;
extern char map_path[256];
extern char map_file[64];
extern char log_prefix[8];
extern char web_path[256];
extern int  ws_refresh_rate;
extern BOOL names_enable;
extern char names_file[256];
extern BOOL history_enable;
extern char history_file[256];
extern BOOL binary_protocol;
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
const char *connect_to_string(sock_connect_t value)
{
  switch (value)
  {
    case CONNECTED:
      return "CONNECTED";
      break;

    case DISCONNECTED:
      return "DISCONNECTED";
      break;

    default:
      return "CONNECT_UNKNOWN";
      break;
  }
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
    return "no time\n";
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
void print_help(int run_time)
{
  if(run_time)
  {
    printf(
        "%-15s %-5s # print commnads\n" \
        "%-15s %-5s # print current settings\n" \
        "%-15s %-5s # print version\n" \
        "%-15s %-5s # start all\n" \
        "%-15s %-5s # start command server\n" \
        "%-15s %-5s # start web server\n" \
        "%-15s %-5s # start websocket server\n" \
        "%-15s %-5s # start client\n" \
        "%-15s %-5s # command port\n" \
        "%-15s %-5s # web port\n" \
        "%-15s %-5s # websocket port\n" \
        "%-15s %-5s # cmd server host(for client)\n" \
        "%-15s %-5s # clients start count\n",
        CMD_HELP,       CMD_S_HELP,
        CMD_CONFIG,     CMD_S_CONFIG,
        CMD_VERSION,    CMD_S_VERSION,
        CMD_ALL,        CMD_S_ALL,
        CMD_SERVER,     CMD_S_SERVER,
        CMD_WEB_SERVER, CMD_S_WEB_SERVER,
        CMD_WS_SERVER,  CMD_S_WS_SERVER,
        CMD_CLIENT,     CMD_S_CLIENT,
        PARAM_PORT,     PARAM_S_PORT,
        PARAM_WEB_PORT, PARAM_S_WEB_PORT,
        PARAM_WS_PORT,  PARAM_S_WS_PORT,
        PARAM_HOST,     PARAM_S_HOST,
        PARAM_COUNT,    PARAM_S_COUNT);
  }
  else
  {
    printf(
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n" \
        "%-15s\n",
        CMD_CLEAR,
        CMD_EXIT,
        CMD_TEST,
        CMD_SND_TO_SERVER,
        CMD_SND_TO_WSSERVER,
        CMD_SND_TO_CLIENT,
        CMD_STREAM,
        CMD_TYPES_INFO,
        CMD_DEFINES_INFO,
        CMD_SERVER_INFO,
        CMD_WEB_SERVER_INFO,
        CMD_WS_SERVER_INFO,
        CMD_CLIENT_INFO,
        CMD_CMD_REGISTER,
        CMD_CMD_ACTIVATE,
        CMD_WS_REGISTER,
        CMD_WS_ACTIVATE,
        CMD_RECONFIG);
  }
}
//==============================================================================
void print_version()
{
  char tmp[1024];

  sprintf(tmp,
          "NIch Client Server Project\n"                                \
          "Copyright 2016 NIch(nich2000@mail.ru) All rights reserved\n" \
          "Version: %s\n"                                               \
          "Sock: %s, Pack: %s, Protocol: %s",
          APPLICATION_VERSION,
          SOCK_VERSION, PACK_VERSION, PROTOCOL_VERSION);

  log_add(LOG_NO_IDENT, tmp);
}
//==============================================================================
void print_config()
{
  char tmp[1024];

  sprintf(tmp,
          "%-25s: %d\n" \
          "%-25s: %d\n" \
          "%-25s: %d\n" \
          "%-25s: %s\n" \
          "%-25s: %d\n" \
          "%-25s: %d\n" \
          "%-25s: %d\n" \
          "%-25s: %s\n" \
          "%-25s: %d\n" \
          "%-25s: %s\n" \
          "%-25s: %d\n" \
          "%-25s: %s\n" \
          "%-25s: %d\n" \
          "%-25s: %s\n" \
          "%-25s: %d\n" \
          "%-25s: %s\n" \
          "%-25s: %d\n" \
          "%-25s: %s\n" \
          "%-25s: %d\n" \
          "%-25s: %s\n" \
          "%-25s: %s\n" \
          "%-25s: %s\n" \
          "%-25s: %d\n" \
          "%-25s: %s\n" \
          "%-25s: %d",
          "web_server_port",       web_server_port,        // 1
          "ws_server_port",        ws_server_port,         // 2
          "cmd_server_port",       cmd_server_port,        // 3
          "cmd_server_host",       cmd_server_host,        // 4
          "session_relay_to_web",  session_relay_to_web,   // 5
          "ws_refresh_rate",       ws_refresh_rate,        // 6
          "names_enable",          names_enable,           // 7
          "names_file",            names_file,             // 8
          "log_enable",            log_enable,             // 9
          "log_path",              log_path,               // 10
          "stat_enable",           stat_enable,            // 11
          "stat_path",             stat_path,              // 12
          "report_enable",         report_enable,          // 13
          "report_path",           report_path,            // 14
          "session_enable",        session_enable,         // 15
          "session_path",          session_path,           // 16
          "session_stream_enable", session_stream_enable,  // 17
          "session_stream_file",   session_stream_file,    // 18
          "map_enable",            map_enable,             // 19
          "map_path",              map_path,               // 20
          "map_file",              map_file,               // 21
          "web_path",              web_path,               // 22
          "history_enable",        history_enable,         // 23
          "history_file",          history_file,           // 24
          "binary_protocol",       binary_protocol         // 25
          );

  log_add(LOG_NO_IDENT, tmp);
}
//==============================================================================
void print_types_info()
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
}
//==============================================================================
void print_defines_info()
{
  char tmp[1024];

  SAFE_MODE
  DEBUG_MODE
  PI_DEVICE
  DEMS_DEVICE

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
}
//==============================================================================
