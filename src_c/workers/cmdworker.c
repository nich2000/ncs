//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "cmdworker.h"

#include "streamer.h"
#include "exec.h"
#include "customworker.h"
#include "wsworker.h"
#include "socket_utils.h"
#include "socket.h"
#include "utils.h"
#include "ncs_log.h"
#include "protocol.h"
#include "protocol_types.h"
#include "ncs_pack_utils.h"
//==============================================================================
typedef struct
{
  char session_id[PACK_VALUE_SIZE];
  char name[PACK_VALUE_SIZE];
} name_item_t;
//==============================================================================
typedef name_item_t name_items_t[SOCK_WORKERS_COUNT];
//==============================================================================
typedef struct
{
  int         count;
  name_items_t items;
} names_t;
//==============================================================================
// map item example
// bs1;-1;1;52,130010000000000;23,771370000000000;5796596,462;1622429,742
//==============================================================================
#define MAP_SIZE 102400
#define MAP_ITEM_SIZE 32
typedef struct
{
  char kind[MAP_ITEM_SIZE];
  char number[MAP_ITEM_SIZE];
  char index[MAP_ITEM_SIZE];
  char lat_f[MAP_ITEM_SIZE];
  char lon_f[MAP_ITEM_SIZE];
  char lat[MAP_ITEM_SIZE];
  char lon[MAP_ITEM_SIZE];
} map_item_t;
//==============================================================================
typedef map_item_t map_items_t[MAP_SIZE];
//==============================================================================
typedef struct
{
  int         count;
  map_items_t items;
} map_t;
//==============================================================================
int cmd_server_init     (cmd_server_t *server);
int cmd_server_start    (cmd_server_t *server, sock_port_t port);
int cmd_server_stop     (cmd_server_t *server);
int cmd_server_pause    (cmd_server_t *server);
int cmd_server_resume   (cmd_server_t *server);
//==============================================================================
void *cmd_server_worker (void *arg);
//==============================================================================
custom_remote_client_t *_cmd_remote_clients_next (cmd_server_t *cmd_server);
int                     _cmd_remote_clients_count(cmd_server_t *cmd_server);
//==============================================================================
int load_names();
//==============================================================================
int cmd_client_init     (cmd_client_t *client);
int cmd_client_start    (cmd_client_t *client, sock_port_t port, sock_host_t host);
int cmd_client_stop     (cmd_client_t *client);
int cmd_client_pause    (cmd_client_t *client);
int cmd_client_resume   (cmd_client_t *client);
int cmd_client_register (cmd_client_t *client);
//==============================================================================
void *cmd_client_worker (void *arg);
//==============================================================================
void *cmd_send_worker   (void *arg);
//==============================================================================
int on_cmd_accept       (void *sender, SOCKET socket, sock_host_t host);
int on_cmd_connect      (void *sender);
int on_cmd_disconnect   (void *sender);
int on_cmd_error        (void *sender, error_t *error);
int on_cmd_send         (void *sender);
int on_cmd_recv         (void *sender, char *buffer, int size);
int on_cmd_new_data     (void *sender, void *data);
//==============================================================================
int on_server_cmd_state (void *sender, sock_state_t state);
int on_client_cmd_state (void *sender, sock_state_t state);
//==============================================================================
static cmd_server_t _cmd_server;
static names_t      _names;
static map_t        _map;
//==============================================================================
// Visible only in streamer.c
int          _cmd_client_count = 0;
cmd_client_t _cmd_client[SOCK_WORKERS_COUNT];
//==============================================================================
static sock_active_t _cmd_active = ACTIVE_NONE;
//-----------------------------------------------------------------------------
sock_active_t _cmd_active_next()
{
  switch (_cmd_active) {
  case ACTIVE_NONE:
  case ACTIVE_SECOND:
    _cmd_active = ACTIVE_FIRST;
    break;

  case ACTIVE_FIRST:
    _cmd_active = ACTIVE_SECOND;
    break;

  default:
    _cmd_active = ACTIVE_NONE;
    break;
  }

  return _cmd_active;
}
//-----------------------------------------------------------------------------
sock_active_t _cmd_active_cur()
{
  return _cmd_active;
}
//-----------------------------------------------------------------------------
sock_active_t _cmd_active_neg()
{
  if(_cmd_active == ACTIVE_FIRST)
     return ACTIVE_SECOND;
  else
    return ACTIVE_FIRST;
}
//==============================================================================
int load_names()
{
  char *file_name = "../config/names.ejn";
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  _names.count = 0;

  FILE *f = fopen(file_name, "r");
  if(f == NULL)
    return make_last_error_fmt(ERROR_NORMAL, errno, "load_names, can not open file %s", file_name);

  while ((read = getline(&line, &len, f)) != -1)
  {
    _names.count++;

    char *token = strtok(line, "=");
    strcpy(_names.items[_names.count-1].session_id, token);

    token = strtok(NULL, "=");
    if(token[strlen(token)-1] == '\n')
      token[strlen(token)-1] = '\0';
    strcpy(_names.items[_names.count-1].name, token);
  }

  for(int i = 0; i < _names.count; i++)
    printf("%s=%s\n",
           _names.items[i].session_id,
           _names.items[i].name);

  fclose(f);
  if(line)
    free(line);

  return ERROR_NONE;
}
//==============================================================================
int load_map()
{
//  char *file_name = "../tracks/Brest.map";
//  char *file_name = "../tracks/Hungaroring.map";
  char *file_name = "../tracks/Mogilev.map";
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  _map.count = 0;

  FILE *f = fopen(file_name, "r");
  if(f == NULL)
    return make_last_error_fmt(ERROR_NORMAL, errno, "load_map, can not open file %s", file_name);

  while ((read = getline(&line, &len, f)) != -1)
  {
    _map.count++;

    map_item_t *map_item = &_map.items[_map.count-1];

    char *token = strtok(line, ";");
    memset(map_item->kind, 0, MAP_ITEM_SIZE);
    strcpy(map_item->kind, token);

    token = strtok(NULL, ";");
    memset(map_item->number, 0, MAP_ITEM_SIZE);
    strcpy(map_item->number, token);

    token = strtok(NULL, ";");
    memset(map_item->index, 0, MAP_ITEM_SIZE);
    strcpy(map_item->index, token);

    token = strtok(NULL, ";");
    memset(map_item->lat_f, 0, MAP_ITEM_SIZE);
    strcpy(map_item->lat_f, token);

    token = strtok(NULL, ";");
    memset(map_item->lon_f, 0, MAP_ITEM_SIZE);
    strcpy(map_item->lon_f, token);

    token = strtok(NULL, ";");
    memset(map_item->lat, 0, MAP_ITEM_SIZE);
    strcpy(map_item->lat, token);

    token = strtok(NULL, "=");
    if(token[strlen(token)-1] == '\n')
      token[strlen(token)-1] = '\0';
    memset(map_item->lon, 0, MAP_ITEM_SIZE);
    strcpy(map_item->lon, token);

//    if(_map.count >= 10)
//      break;
  }

  for(int i = 0; i < _map.count; i++)
    printf("%s  %s  %s  %s  %s  %s  %s\n",
           _map.items[i].kind,
           _map.items[i].number,
           _map.items[i].index,
           _map.items[i].lat_f,
           _map.items[i].lon_f,
           _map.items[i].lat,
           _map.items[i].lon);

  fclose(f);
  if(line)
    free(line);

  return ERROR_NONE;
}
//==============================================================================
const char *get_name_by_session_id(char *session_id)
{
  for(int i = 0; i < _names.count; i++)
    if(strcmp(_names.items[i].session_id, session_id) == 0)
      return (char*)_names.items[i].name;

  return session_id;
}
//==============================================================================
int cmd_server(sock_state_t state, sock_port_t port)
{
  sock_print_server_header(SOCK_MODE_CMD_SERVER, port);

  switch(state)
  {
    case STATE_NONE:
    {
      break;
    }
    case STATE_START:
    {
      cmd_server_start(&_cmd_server, port);
      break;
    }
    case STATE_STOP:
    {
      cmd_server_stop(&_cmd_server);
      break;
    }
    case STATE_PAUSE:
    {
      cmd_server_pause(&_cmd_server);
      break;
    }
    case STATE_RESUME:
    {
      cmd_server_resume(&_cmd_server);
      break;
    }
    default:;
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_status()
{
  clr_scr();

  print_custom_worker_info(&_cmd_server.custom_server.custom_worker , "cmd_server");

  print_custom_remote_clients_list_info(&_cmd_server.custom_remote_clients_list, "cmd_server");

  return ERROR_NONE;
}
//==============================================================================
int cmd_client(sock_state_t state, sock_port_t port, sock_host_t host, int count)
{
  if(count >= SOCK_WORKERS_COUNT)
    return make_last_error_fmt(ERROR_CRITICAL, errno, "cmd_client, too match count(%d)", count);

  sock_print_client_header(port, host);

  _cmd_client_count = count;

  for(int i = 0; i < _cmd_client_count; i++)
  {
    switch(state)
    {
      case STATE_NONE:
      {
        break;
      }
      case STATE_START:
      {
        cmd_client_start(&_cmd_client[i], port, host);
        break;
      }
      case STATE_STOP:
      {
        cmd_client_stop(&_cmd_client[i]);
        break;
      }
      case STATE_PAUSE:
      {
        cmd_client_pause(&_cmd_client[i]);
        break;
      }
      case STATE_RESUME:
      {
        cmd_client_resume(&_cmd_client[i]);
        break;
      }
      default:;
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_status()
{
  clr_scr();

  for(int i = 0; i < _cmd_client_count; i++)
    print_custom_worker_info(&_cmd_client[i].custom_client.custom_remote_client.custom_worker, "cmd_client");

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_init(cmd_server_t *server)
{
  custom_server_init(STATIC_CMD_SERVER_ID, &server->custom_server);

  custom_remote_clients_init(&server->custom_remote_clients_list);

  strcpy((char*)server->custom_server.custom_worker.session_id, STATIC_CMD_SERVER_NAME);
  server->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  server->custom_server.custom_worker.mode = SOCK_MODE_CMD_SERVER;

  server->custom_server.on_accept          = &on_cmd_accept;

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_start(cmd_server_t *server, sock_port_t port)
{
  if(server->custom_server.custom_worker.state == STATE_START)
    return make_last_error(ERROR_NORMAL, errno, "cmd_server_start, server already started");

  load_names();

  load_map();

  cmd_server_init(server);

  server->custom_server.custom_worker.port  = port;
  server->custom_server.custom_worker.state = STATE_STARTING;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&server->custom_server.work_thread, &tmp_attr, cmd_server_worker, (void*)server);

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_stop(cmd_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_STOPPING;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(server->custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      server->custom_remote_clients_list.items[i].custom_worker.state = STATE_STOPPING;

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_pause(cmd_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_PAUSING;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(server->custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      server->custom_remote_clients_list.items[i].custom_worker.state = STATE_PAUSING;

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_resume(cmd_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_RESUMING;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(server->custom_remote_clients_list.items[i].custom_worker.state == STATE_PAUSE)
      server->custom_remote_clients_list.items[i].custom_worker.state = STATE_RESUMING;

  return ERROR_NONE;
}
//==============================================================================
custom_remote_client_t *_cmd_remote_clients_next(cmd_server_t *cmd_server)
{
  custom_remote_client_t *tmp_client = _custom_remote_clients_next(&cmd_server->custom_remote_clients_list);

  if(tmp_client != NULL)
  {
    custom_remote_client_init(ID_GEN_NEW, tmp_client);

    tmp_client->protocol.on_new_in_data = on_cmd_new_data;

    tmp_client->custom_worker.type  = SOCK_TYPE_REMOTE_CLIENT;
    tmp_client->custom_worker.mode  = cmd_server->custom_server.custom_worker.mode;
    tmp_client->custom_worker.port  = cmd_server->custom_server.custom_worker.port;

    tmp_client->on_disconnect       = on_cmd_disconnect;
    tmp_client->on_error            = on_cmd_error;
    tmp_client->on_recv             = on_cmd_recv;
    tmp_client->on_send             = on_cmd_send;
  }

  return tmp_client;
}
//==============================================================================
int _cmd_remote_clients_count(cmd_server_t *cmd_server)
{
  return _custom_remote_clients_count(&cmd_server->custom_remote_clients_list);
}
//==============================================================================
int on_cmd_accept(void *sender, SOCKET socket, sock_host_t host)
{
  custom_remote_client_t *tmp_client = _cmd_remote_clients_next(&_cmd_server);
  if(tmp_client == 0)
    return make_last_error_fmt(ERROR_CRITICAL, errno, "cmd_accept, no available clients, socket: %d, host: %s", socket, host);

  tmp_client->custom_worker.on_state = on_server_cmd_state;

  time_t rawtime;
  time (&rawtime);
  tmp_client->connect_time = rawtime;

  tmp_client->custom_worker.state = STATE_STARTING;

  memcpy(&tmp_client->custom_worker.sock, &socket, sizeof(SOCKET));
  memcpy(tmp_client->custom_worker.host, host,   SOCK_HOST_SIZE);

  log_add_fmt(LOG_DEBUG,
              "cmd_accept, socket: %d, host: %s, port: %d",
              tmp_client->custom_worker.sock, tmp_client->custom_worker.host, tmp_client->custom_worker.port);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(&tmp_client->recv_thread, &tmp_attr, custom_recv_worker, (void*)tmp_client);
  pthread_create(&tmp_client->send_thread, &tmp_attr, cmd_send_worker,    (void*)tmp_client);

  return ERROR_NONE;
}
//==============================================================================
void *cmd_server_worker(void *arg)
{
  log_add("[BEGIN] cmd_server_worker", LOG_DEBUG);

  cmd_server_t *tmp_server = (cmd_server_t*)arg;

  custom_server_start(&tmp_server->custom_server.custom_worker);
  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  log_add("[END] cmd_server_worker", LOG_DEBUG);

  return NULL;
}
//==============================================================================
int cmd_client_init(cmd_client_t *client)
{
  custom_client_init(&client->custom_client);

  client->custom_client.custom_remote_client.custom_worker.type = SOCK_TYPE_CLIENT;
  client->custom_client.custom_remote_client.custom_worker.mode = SOCK_MODE_CMD_CLIENT;

  client->custom_client.on_connect                         = on_cmd_connect;

  client->custom_client.custom_remote_client.on_disconnect = on_cmd_disconnect;
  client->custom_client.custom_remote_client.on_error      = on_cmd_error;
  client->custom_client.custom_remote_client.on_recv       = on_cmd_recv;
  client->custom_client.custom_remote_client.on_send       = on_cmd_send;

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_start(cmd_client_t *client, sock_port_t port, sock_host_t host)
{
  cmd_client_init(client);

  protocol_init(&client->custom_client.custom_remote_client.protocol);

  client->custom_client.custom_remote_client.protocol.on_new_in_data  = on_cmd_new_data;
//  client->custom_client.custom_remote_client.protocol.on_new_out_data = cmd_new_data;
  client->custom_client.custom_remote_client.custom_worker.on_state = on_client_cmd_state;

  client->custom_client.custom_remote_client.custom_worker.port = port;
  strcpy((char*)client->custom_client.custom_remote_client.custom_worker.host, (char*)host);

  client->custom_client.custom_remote_client.custom_worker.state = STATE_STARTING;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&client->custom_client.work_thread, &tmp_attr, cmd_client_worker, (void*)client);

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_stop(cmd_client_t *client)
{
  client->custom_client.custom_remote_client.custom_worker.state = STATE_STOPPING;

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_pause(cmd_client_t *client)
{
  client->custom_client.custom_remote_client.custom_worker.state  = STATE_PAUSING;

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_resume(cmd_client_t *client)
{
  client->custom_client.custom_remote_client.custom_worker.state = STATE_RESUMING;

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_register(cmd_client_t *client)
{
  char tmp[128];
  sprintf(tmp, "sndtosr register %s", client->custom_client.custom_remote_client.custom_worker.session_id);
  handle_command_str(client, tmp);

  return ERROR_NONE;
}
//==============================================================================
void *cmd_client_worker(void *arg)
{
  log_add("[BEGIN] cmd_client_worker", LOG_DEBUG);

  cmd_client_t *tmp_client = (cmd_client_t*)arg;

  do
  {
    custom_client_start(&tmp_client->custom_client.custom_remote_client.custom_worker);
    custom_client_work (&tmp_client->custom_client);
    custom_worker_stop (&tmp_client->custom_client.custom_remote_client.custom_worker);
  }
  while(tmp_client->custom_client.custom_remote_client.custom_worker.state == STATE_START);

  log_add("[END] cmd_client_worker", LOG_DEBUG);

  return NULL;
}
//==============================================================================
int on_cmd_connect(void *sender)
{
  log_add("[BEGIN] cmd_connect", LOG_DEBUG);

  log_add("connected to server", LOG_INFO);

  custom_client_t *tmp_client = (custom_client_t*)sender;

  time_t rawtime;
  time (&rawtime);
  tmp_client->custom_remote_client.connect_time = rawtime;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&tmp_client->custom_remote_client.recv_thread,
                 &tmp_attr,
                 custom_recv_worker,
                 (void*)&tmp_client->custom_remote_client);

  pthread_create(&tmp_client->custom_remote_client.send_thread,
                 &tmp_attr,
                 cmd_send_worker,
                 (void*)&tmp_client->custom_remote_client);

  int status_send;
  pthread_join(tmp_client->custom_remote_client.recv_thread, (void**)&status_send);

  int status_recv;
  pthread_join(tmp_client->custom_remote_client.send_thread, (void**)&status_recv);

  log_add("[END] cmd_connect", LOG_DEBUG);

  return ERROR_NONE;
}
//==============================================================================
void *cmd_send_worker(void *arg)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  log_add_fmt(LOG_DEBUG, "[BEGIN] cmd_send_worker, socket: %d", tmp_sock);

  // TODO: analog on_connect - need to send clients list to ws - govnokod
  tmp_client->custom_worker.state = STATE_START;
  if(tmp_client->custom_worker.on_state != NULL)
    tmp_client->custom_worker.on_state(tmp_client, STATE_START);

  pack_protocol_t *tmp_protocol = &tmp_client->protocol;
  pack_packet_t   *tmp_pack;
  pack_buffer_t    tmp_buffer;
  pack_size_t      tmp_size;

  while(tmp_client->custom_worker.state == STATE_START)
  {
    tmp_pack = _protocol_next_pack(tmp_protocol);
    while(tmp_pack != NULL)
    {
      if(pack_to_buffer(tmp_pack, tmp_buffer, &tmp_size) != ERROR_NONE)
        continue;

      int tmp_cnt = protocol_bin_buffer_validate(tmp_buffer,
                                                 tmp_size,
                                                 PACK_VALIDATE_ONLY,
                                                 tmp_protocol,
                                                 (void*)tmp_client);
      if(tmp_cnt > 0)
      {
        if(sock_send(tmp_sock, (char*)tmp_buffer, (int)tmp_size) == ERROR_NONE)
        {
          if(tmp_client->on_send != 0)
            tmp_client->on_send((void*)tmp_client);
        }
        else
        {
          log_add_fmt(LOG_ERROR, "cmd_send_worker, error: %s", last_error()->message);
        }
      }
      else
      {
        log_add_fmt(LOG_ERROR, "cmd_send_worker, error: %s", last_error()->message);
      }

      tmp_pack = _protocol_next_pack(tmp_protocol);
    }

    usleep(1000);
  }

  tmp_client->custom_worker.state = STATE_STOP;

  log_add_fmt(LOG_DEBUG, "[END] cmd_send_worker, socket: %d", tmp_sock);

  return NULL;
}
//==============================================================================
int on_cmd_disconnect(void *sender)
{
  log_add("cmd_disconnect, disconnected from server", LOG_INFO);

  custom_client_t *tmp_client = (custom_client_t*)sender;

  time_t rawtime;
  time (&rawtime);
  tmp_client->custom_remote_client.connect_time = rawtime;

  return ERROR_NONE;
}
//==============================================================================
int on_cmd_error(void *sender, error_t *error)
{
  log_add_fmt(LOG_INFO, "cmd_error, message: %s", error->message);

  return ERROR_NONE;
}
//==============================================================================
int on_cmd_send(void *sender)
{
  return ERROR_NONE;
}
//==============================================================================
int on_cmd_recv(void *sender, char *buffer, int size)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)sender;

  pack_protocol_t *tmp_protocol = &tmp_client->protocol;

#ifdef USE_BINARY_PROTOCOL
  protocol_bin_buffer_validate((unsigned char*)buffer, size, PACK_VALIDATE_ADD, tmp_protocol, (void*)tmp_client);
#else
  protocol_txt_buffer_validate((unsigned char*)buffer, size, PACK_VALIDATE_ADD, tmp_protocol, (void*)tmp_client);
#endif

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_send_cmd(int argc, ...)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_t *tmp_client = &_cmd_server.custom_remote_clients_list.items[i];

    if(tmp_client->custom_worker.state == STATE_START)
    {
      pack_protocol_t *tmp_protocol = &tmp_client->protocol;

      protocol_begin(tmp_protocol);

      va_list params;
      va_start(params, argc);

      char *cmd = va_arg(params, char*);
      protocol_add_cmd((unsigned char*)cmd, tmp_protocol);

      for(int i = 1; i < argc; i++)
      {
        char *param = va_arg(params, char*);
        protocol_add_param_as_string((unsigned char*)param, tmp_protocol);
      }

      va_end(params);

      protocol_end(tmp_protocol);
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_send_pack(pack_packet_t *pack)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_t *tmp_client = &_cmd_server.custom_remote_clients_list.items[i];

    if(tmp_client->custom_worker.state == STATE_START)
    {
      pack_protocol_t *tmp_protocol = &tmp_client->protocol;

      protocol_begin(tmp_protocol);

      protocol_assign_pack(tmp_protocol, pack);

      protocol_end(tmp_protocol);
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_send_cmd(int argc, ...)
{
  for(int i = 0; i < _cmd_client_count; i++)
  {
    pack_protocol_t *tmp_protocol = &_cmd_client[i].custom_client.custom_remote_client.protocol;

    protocol_begin(tmp_protocol);

    va_list tmp_params;
    va_start(tmp_params, argc);

    char *tmp_cmd = va_arg(tmp_params, char*);
    protocol_add_cmd((unsigned char*)tmp_cmd, tmp_protocol);

    for(int i = 1; i < argc; i++)
    {
      char *tmp_param = va_arg(tmp_params, char*);
      protocol_add_param_as_string((unsigned char*)tmp_param, tmp_protocol);
    }

    va_end(tmp_params);

    protocol_end(tmp_protocol);
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_send_pack(pack_packet_t *pack)
{
  for(int i = 0; i < _cmd_client_count; i++)
  {
    pack_protocol_t *tmp_protocol = &_cmd_client[i].custom_client.custom_remote_client.protocol;

    protocol_begin(tmp_protocol);

    protocol_assign_pack(tmp_protocol, pack);

    protocol_end(tmp_protocol);
  }

  return ERROR_NONE;
}
//==============================================================================
int on_cmd_new_data(void *sender, void *data)
{
//  log_add("cmd_new_data", LOG_INFO);

  custom_remote_client_t *tmp_client = (custom_remote_client_t*)sender;

  pack_packet_t *tmp_packet = (pack_packet_t*)data;

  if(_pack_is_command(tmp_packet))
  {
    return handle_command_pack(sender, tmp_packet);
  }
  else
  {
    pack_buffer_t csv;
    int result = pack_values_to_csv(tmp_packet, ';', csv);
    if(result != ERROR_NONE)
    {
      log_add_fmt(LOG_ERROR, "cmd_new_data, pack_values_to_csv, result: %d", result);
    }
    else
    {
      int len = strlen((char*)csv);

      int wrote = report_add(tmp_client->report, (char*)csv);
      if(wrote != (len+1))
      {
        log_add_fmt(LOG_ERROR, "cmd_new_data, report_add, len: %d, wrote: %d", len, wrote);
      }
    }

    // ACTIVE_FIRST or ACTIVE_SECOND
    if(tmp_client->active_state)
    {
      pack_add_as_int(tmp_packet, (unsigned char*)"ACT", tmp_client->active_state);

      return ws_server_send_pack(SOCK_SEND_TO_ALL, tmp_packet);
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int on_server_cmd_state(void *sender, sock_state_t state)
{
  return ERROR_NONE;
}
//==============================================================================
int on_client_cmd_state (void *sender, sock_state_t state)
{
  if(state == STATE_START)
    cmd_client_register((cmd_client_t*)sender);

  return ERROR_NONE;
}
//==============================================================================
int cmd_remote_client_list(pack_packet_t *pack)
{
  if(pack == NULL)
    return make_last_error(ERROR_NORMAL, errno, "cmd_remote_clients_list, pack == NULL");

  pack_init(pack);
  pack_add_cmd(pack, (unsigned char*)"clients");

  if(_cmd_remote_clients_count(&_cmd_server) == 0)
    return make_last_error(ERROR_NONE, errno, "cmd_remote_clients_list, no cmd clients connected");

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_t *tmp_remote_client = &_cmd_server.custom_remote_clients_list.items[i];
    custom_worker_t *tmp_custom_worker = &tmp_remote_client->custom_worker;

    if(tmp_custom_worker->state == STATE_START)
    {
      pack_packet_t tmp_pack;
      pack_init(&tmp_pack);
      pack_add_as_int   (&tmp_pack, (unsigned char*)"_ID", tmp_custom_worker->id);
      pack_add_as_string(&tmp_pack, (unsigned char*)"NAM", tmp_custom_worker->name);
      pack_add_as_int   (&tmp_pack, (unsigned char*)"STA", tmp_custom_worker->state);
      pack_add_as_int   (&tmp_pack, (unsigned char*)"ACT", tmp_remote_client->active_state);
      pack_add_as_int   (&tmp_pack, (unsigned char*)"REG", tmp_remote_client->register_state);

      pack_add_as_pack(pack, (unsigned char*)PACK_PARAM_KEY, &tmp_pack);
    }
  };

  print_pack(pack, "clients", FALSE, FALSE, TRUE, TRUE);

  return ERROR_NONE;
}
//==============================================================================
int cmd_map(pack_packet_t *pack)
{
  if(pack == NULL)
    return make_last_error(ERROR_NORMAL, errno, "cmd_map, pack == NULL");

  pack_init(pack);
  pack_add_cmd(pack, (unsigned char*)"map");

  for(int i = 0; i < _map.count; i++)
  {
    pack_packet_t tmp_pack;
    pack_init(&tmp_pack);
    pack_add_as_string(&tmp_pack, (unsigned char*)"KND", (unsigned char*)_map.items[i].kind);
    pack_add_as_string(&tmp_pack, (unsigned char*)"NUM", (unsigned char*)_map.items[i].number);
    pack_add_as_string(&tmp_pack, (unsigned char*)"IND", (unsigned char*)_map.items[i].index);
    pack_add_as_string(&tmp_pack, (unsigned char*)"LAF", (unsigned char*)_map.items[i].lat_f);
    pack_add_as_string(&tmp_pack, (unsigned char*)"LOF", (unsigned char*)_map.items[i].lon_f);
    pack_add_as_string(&tmp_pack, (unsigned char*)"_LA", (unsigned char*)_map.items[i].lat);
    pack_add_as_string(&tmp_pack, (unsigned char*)"_LO", (unsigned char*)_map.items[i].lon);

    pack_add_as_pack(pack, (unsigned char*)PACK_PARAM_KEY, &tmp_pack);
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_remote_client_activate(sock_id_t id, sock_active_t active)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.id == id)
      {
        sock_active_t cur_active;

        if(active == ACTIVE_NEXT)
          cur_active = _cmd_active_next();
        else
          cur_active = active;

        time_t rawtime;
        time (&rawtime);
        _cmd_server.custom_remote_clients_list.items[i].active_time = rawtime;

        _cmd_server.custom_remote_clients_list.items[i].active_state = cur_active;

        switch (cur_active)
        {
          case ACTIVE_FIRST:
            log_add_fmt(LOG_INFO, "acivate first(%d)", id);
            break;
          case ACTIVE_SECOND:
            log_add_fmt(LOG_INFO, "acivate second(%d)", id);
            break;
          default:
            log_add_fmt(LOG_INFO, "deacivate(%d)", id);
            break;
        }

        pack_packet_t tmp_packet;
        cmd_remote_client_list(&tmp_packet);
        return ws_server_send_pack(SOCK_SEND_TO_ALL, &tmp_packet);
      }
  }

  return make_last_error_fmt(ERROR_NORMAL, errno, "to activate the client not found, id: %d", id);
}
//==============================================================================
int cmd_remote_client_activate_all(sock_active_t active, sock_active_t except)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
    {
      sock_active_t cur_active = _cmd_server.custom_remote_clients_list.items[i].active_state;

      sock_active_t cur_except;
      if(except == ACTIVE_NEXT)
        cur_except = _cmd_active_cur();
      else
        cur_except = except;

      if(cur_active == cur_except)
        continue;

      _cmd_server.custom_remote_clients_list.items[i].active_state = active;
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_remote_client_register(sock_id_t id, sock_name_t session_id)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.id == id)
      {
        custom_remote_client_t *client = &_cmd_server.custom_remote_clients_list.items[i];

        // session_id
        strcpy((char*)client->custom_worker.session_id, (char*)session_id);

        // name
        strcpy((char*)client->custom_worker.name, get_name_by_session_id((char*)session_id));

        // time
        time_t rawtime;
        time (&rawtime);
        client->register_time = rawtime;

        // state
        client->register_state = REGISTER_OK;

        log_add_fmt(LOG_INFO,
                    "cmd_remote_clients_register, id: %d, session_id: %s, name: %s",
                    client->custom_worker.id,
                    client->custom_worker.session_id,
                    client->custom_worker.name);

        pack_packet_t tmp_packet;
        cmd_remote_client_list(&tmp_packet);
        return ws_server_send_pack(SOCK_SEND_TO_ALL, &tmp_packet);
      }
  }

  return make_last_error_fmt(ERROR_NORMAL,
                             errno,
                             "cmd_remote_clients_register, client not found, id: %d, session_id: %s",
                             id,
                             session_id);
}
//==============================================================================
