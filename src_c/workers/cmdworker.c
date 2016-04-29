//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: cmdworker.c
 */
//==============================================================================
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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
#include "map.h"
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
int on_cmd_error        (void *sender, ncs_error_t *error);
int on_cmd_send         (void *sender);
int on_cmd_recv         (void *sender, char *buffer, int size);
int on_cmd_new_data     (void *sender, void *data);
//==============================================================================
int on_server_cmd_state (void *sender, sock_state_t state);
int on_client_cmd_state (void *sender, sock_state_t state);
//==============================================================================
BOOL session_relay_to_web = TRUE;
//==============================================================================
static cmd_server_t  _cmd_server;
static names_t       _names;
static int           _cmd_client_count = 0;
//==============================================================================
static pthread_mutex_t mutex_client_register = PTHREAD_MUTEX_INITIALIZER;
//==============================================================================
cmd_clients_t _cmd_clients;
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
int cmd_client_count()
{
  return _cmd_client_count;
}
//==============================================================================
cmd_clients_t *cmd_clients()
{
  return &_cmd_clients;
}
//==============================================================================
int load_names()
{
  char full_file_name[256] = "../config/names.ejn";
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  _names.count = 0;

  FILE *f = fopen(full_file_name, "r");
  if(f == NULL)
    return make_last_error_fmt(ERROR_NORMAL, errno, "load_names, can not open file %s", full_file_name);

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

  log_add_fmt(LOG_INFO, "[CMD] load_names, file: %s, count: %d",
              full_file_name, _names.count);

//  for(int i = 0; i < _names.count; i++)
//    printf("%s=%s\n",
//           _names.items[i].session_id,
//           _names.items[i].name);

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
        cmd_client_start(&_cmd_clients[i], port, host);
        break;
      }
      case STATE_STOP:
      {
        cmd_client_stop(&_cmd_clients[i]);
        break;
      }
      case STATE_PAUSE:
      {
        cmd_client_pause(&_cmd_clients[i]);
        break;
      }
      case STATE_RESUME:
      {
        cmd_client_resume(&_cmd_clients[i]);
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
  {
    print_custom_worker_info(&_cmd_clients[i].custom_client.custom_remote_client.custom_worker, "worker");
    print_custom_remote_client_info(&_cmd_clients[i].custom_client.custom_remote_client, "client");
  }

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
  return _custom_remote_clients_count_con(&cmd_server->custom_remote_clients_list);
}
//==============================================================================
int on_cmd_accept(void *sender, SOCKET socket, sock_host_t host)
{
  custom_server_t *tmp_server = (custom_server_t*)sender;

  custom_remote_client_t *tmp_client = _cmd_remote_clients_next(&_cmd_server);
  if(tmp_client == 0)
    return make_last_error_fmt(ERROR_CRITICAL, errno, "cmd_accept, no available clients, server id: %d, socket: %d, host: %s",
                               tmp_server->custom_worker.id, socket, host);

  tmp_client->custom_worker.on_state = on_server_cmd_state;

  time_t rawtime;
  time (&rawtime);
  tmp_client->connect_state = CONNECTED;
  tmp_client->connect_time = rawtime;

  tmp_client->custom_worker.state = STATE_STARTING;

  memcpy(&tmp_client->custom_worker.sock, &socket, sizeof(SOCKET));
  memcpy(tmp_client->custom_worker.host, host,   SOCK_HOST_SIZE);

  log_add_fmt(LOG_INFO, "[CMD] cmd_accept, server id: %d, worker id: %d, host: %s",
              tmp_server->custom_worker.id,
              tmp_client->custom_worker.id, tmp_client->custom_worker.host);

  char tmp_name[64];
  sprintf(tmp_name, "%d", tmp_client->custom_worker.id);
  #ifdef WRITE_REPORT
  tmp_client->report = report_open(tmp_name);
  #endif
  #ifdef WRITE_SESSION
  tmp_client->session = session_open(tmp_name);
  #endif
  #ifdef WRITE_STAT
  tmp_client->stat = stat_open(tmp_name);
  #endif

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
  cmd_server_t *tmp_server = (cmd_server_t*)arg;

  log_add_fmt(LOG_DEBUG, "[CMD] [BEGIN] cmd_server_worker, server id: %d",
              tmp_server->custom_server.custom_worker.id);

  custom_server_start(&tmp_server->custom_server.custom_worker);
  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  log_add_fmt(LOG_DEBUG, "[CMD] [END] cmd_server_worker, server id: %d",
              tmp_server->custom_server.custom_worker.id);

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
  pthread_mutex_lock(&mutex_client_register);

  log_add_fmt(LOG_INFO, "cmd_client_register, worker id: %d, session id: %s",
              client->custom_client.custom_remote_client.custom_worker.id,
              client->custom_client.custom_remote_client.custom_worker.session_id);

  char tmp[128];
  sprintf(tmp, "%s %d %s %s",
          CMD_SND_TO_SERVER,
          client->custom_client.custom_remote_client.custom_worker.id,
          CMD_CMD_REGISTER,
          client->custom_client.custom_remote_client.custom_worker.session_id);
  handle_command_str(client, tmp);

  pthread_mutex_unlock(&mutex_client_register);

  return ERROR_NONE;
}
//==============================================================================
// TODO: пока непонятно нужна ли эта функция вообще
int cmd_client_register_result(cmd_client_t *client, int result)
{
  log_add_fmt(LOG_INFO, "cmd_client_register_result: %d, worker id: %d",
              client->custom_client.custom_remote_client.custom_worker.id, result);

  return ERROR_NONE;
}
//==============================================================================
void *cmd_client_worker(void *arg)
{
  cmd_client_t *tmp_client = (cmd_client_t*)arg;

  log_add_fmt(LOG_DEBUG, "[CMD] [BEGIN] cmd_client_worker, worker id: %d",
              tmp_client->custom_client.custom_remote_client.custom_worker.id);

  do
  {
    custom_client_start(&tmp_client->custom_client.custom_remote_client.custom_worker);
    custom_client_work (&tmp_client->custom_client);
    custom_worker_stop (&tmp_client->custom_client.custom_remote_client.custom_worker);
  }
  while(tmp_client->custom_client.custom_remote_client.custom_worker.state == STATE_START);

  log_add_fmt(LOG_DEBUG, "[CMD] [END] cmd_client_worker, worker id: %d",
              tmp_client->custom_client.custom_remote_client.custom_worker.id);

  return NULL;
}
//==============================================================================
int on_cmd_connect(void *sender)
{
  custom_client_t *tmp_client = (custom_client_t*)sender;

  log_add_fmt(LOG_DEBUG, "[CMD] [BEGIN] cmd_connect, worker id: %d",
              tmp_client->custom_remote_client.custom_worker.id);

  time_t rawtime;
  time (&rawtime);
  tmp_client->custom_remote_client.connect_state = CONNECTED;
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

  log_add_fmt(LOG_DEBUG, "[CMD] [END] cmd_connect, worker id: %d",
              tmp_client->custom_remote_client.custom_worker.id);

  return ERROR_NONE;
}
//==============================================================================
//int dump(void *pack, long size, pack_buffer_t buffer)
//{
//  unsigned int i;
//  const unsigned char *const px = (unsigned char*)pack;

//  for (i = 0; i < size; ++i)
//  {
//    //if(i % (sizeof(int) * 8) == 0)
//    //{
//    //  printf("\n%08X ", i);
//    //}
//    //else if(i % 4 == 0)
//    //{
//    //  printf(" ");
//    //}
//    //printf("%02X", px[i]);
//  }

//  printf("\n\n");
//  return 0;
//}
//==============================================================================
void *cmd_send_worker(void *arg)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  log_add_fmt(LOG_DEBUG, "[CMD] [BEGIN] cmd_send_worker, worker id: %d",
              tmp_client->custom_worker.id);

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
      if(pack_to_buffer(tmp_pack, tmp_buffer, &tmp_size) > ERROR_WARNING)
      {
        log_add_fmt(LOG_ERROR, "cmd_send_worker, protocol id: %d, worker id: %d, worker name: %s\n" \
                               "message: %s",
                    tmp_client->protocol.id, tmp_client->custom_worker.id, tmp_client->custom_worker.name,
                    last_error()->message);
        goto next_step;
      }

      int res = protocol_bin_buffer_validate(tmp_buffer,
                                                 tmp_size,
                                                 PACK_VALIDATE_ONLY,
                                                 tmp_protocol,
                                                 (void*)tmp_client);
      if(res == ERROR_NONE)
      {
        if(sock_send(tmp_sock, (char*)tmp_buffer, (int)tmp_size) == ERROR_NONE)
        {
          if(tmp_client->on_send != 0)
            tmp_client->on_send((void*)tmp_client);
        }
        else
        {
          log_add_fmt(LOG_ERROR, "[CMD] cmd_send_worker, worker id: %d,\nmessage: %s",
                      tmp_client->custom_worker.id, last_error()->message);
        }
      }
      else
      {
        log_add_fmt(LOG_ERROR, "[CMD] cmd_send_worker, worker id: %d,\nmessage: %s",
                    tmp_client->custom_worker.id, last_error()->message);
      }

      next_step:
      tmp_pack = _protocol_next_pack(tmp_protocol);
    }

    usleep(1000);
  }

  tmp_client->custom_worker.state = STATE_STOP;

  log_add_fmt(LOG_DEBUG, "[CMD] [END] cmd_send_worker, worker id: %d",
              tmp_client->custom_worker.id);

  return NULL;
}
//==============================================================================
int on_cmd_disconnect(void *sender)
{
  custom_client_t *tmp_client = (custom_client_t*)sender;

  log_add_fmt(LOG_INFO, "[CMD] on_cmd_disconnect, disconnected from server, worker id: %d",
              tmp_client->custom_remote_client.custom_worker.id);

  time_t rawtime;
  time (&rawtime);
  tmp_client->custom_remote_client.connect_state = DISCONNECTED;
  tmp_client->custom_remote_client.disconnect_time = rawtime;

  #ifdef WRITE_REPORT
  report_close(tmp_client->custom_remote_client.report);
  #endif
  #ifdef WRITE_SESSION
  session_close(tmp_client->custom_remote_client.session);
  #endif
  #ifdef WRITE_STAT
  stat_close(tmp_client->custom_remote_client.stat);
  #endif

  return ERROR_NONE;
}
//==============================================================================
int on_cmd_error(void *sender, ncs_error_t *error)
{
  log_add_fmt(LOG_INFO, "[CMD] cmd_error,\nmessage: %s",
              error->message);

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

  int res;
#ifdef USE_BINARY_PROTOCOL
  res = protocol_bin_buffer_validate((unsigned char*)buffer, size, PACK_VALIDATE_ADD, tmp_protocol, (void*)tmp_client);
#else
  res = protocol_txt_buffer_validate((unsigned char*)buffer, size, PACK_VALIDATE_ADD, tmp_protocol, (void*)tmp_client);
#endif

  if(res >= ERROR_WARNING)
    log_add_fmt(LOG_ERROR, "on_cmd_recv,\nmessage: %s",
                last_error()->message);

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
int cmd_client_send_cmd(sock_id_t client_id, int argc, ...)
{
//  log_add(LOG_INFO, "cmd_client_send_cmd");

  for(int i = 0; i < _cmd_client_count; i++)
  {
    if(_cmd_clients[i].custom_client.custom_remote_client.custom_worker.id != client_id)
      continue;

    pack_protocol_t *tmp_protocol = &_cmd_clients[i].custom_client.custom_remote_client.protocol;

//    log_add_fmt(LOG_INFO, "cmd_client_send_cmd, protocol id: %d",
//            tmp_protocol->id);

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
int cmd_client_send_pack(sock_id_t client_id, pack_packet_t *pack)
{
//  log_add(LOG_INFO, "cmd_client_send_pack");

  for(int i = 0; i < _cmd_client_count; i++)
  {
    if(_cmd_clients[i].custom_client.custom_remote_client.custom_worker.id != client_id)
      continue;

    pack_protocol_t *tmp_protocol = &_cmd_clients[i].custom_client.custom_remote_client.protocol;

//    log_add_fmt(LOG_INFO, "cmd_client_send_pack, protocol id: %d",
//            tmp_protocol->id);

    protocol_begin(tmp_protocol);

    protocol_assign_pack(tmp_protocol, pack);

    protocol_end(tmp_protocol);
  }

  return ERROR_NONE;
}
//==============================================================================
int on_cmd_new_data(void *sender, void *data)
{
//  log_add(LOG_INFO, "on_cmd_new_data");

  custom_remote_client_t *tmp_client = (custom_remote_client_t*)sender;
  pack_packet_t *tmp_packet = (pack_packet_t*)data;

  if(_pack_is_command(tmp_packet))
  {
    return handle_command_pack(sender, tmp_packet);
  }
  else
  {
    #ifdef WRITE_SESSION
    pack_buffer_t csv;
    int result = pack_values_to_csv(tmp_packet, ';', csv);
    if(result >= ERROR_WARNING)
    {
      log_add_fmt(LOG_ERROR, "[CMD] cmd_new_data, pack_values_to_csv, worker id: %d, result: %d",
                  tmp_client->custom_worker.id, result);
    }
    else
    {
      int len = strlen((char*)csv);
      int wrote = session_add(tmp_client->session, (char*)csv);
      if(wrote != (len+1))
      {
        log_add_fmt(LOG_ERROR, "[CMD] cmd_new_data, session_add, worker id: %d, len: %d, wrote: %d",
                    tmp_client->custom_worker.id, len, wrote);
      }
    }
    #endif

    #ifdef STREAM_TO_WS
    // ACTIVE_FIRST or ACTIVE_SECOND
    if((session_relay_to_web) && (tmp_client->active_state))
      return ws_server_send_data(SOCK_SEND_TO_ALL, tmp_packet, tmp_client->active_state);
    #endif
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

    if(tmp_custom_worker->state != STATE_STOP)
    {
      pack_packet_t tmp_pack;
      pack_init(&tmp_pack);

      pack_add_as_int   (&tmp_pack, (unsigned char*)"_ID", tmp_custom_worker->id);
      pack_add_as_string(&tmp_pack, (unsigned char*)"NAM", tmp_custom_worker->name);
      pack_add_as_string(&tmp_pack, (unsigned char*)"SES", tmp_custom_worker->session_id);
      pack_add_as_int   (&tmp_pack, (unsigned char*)"STA", tmp_custom_worker->state);
      pack_add_as_int   (&tmp_pack, (unsigned char*)"CON", tmp_remote_client->connect_state);
      pack_add_as_int   (&tmp_pack, (unsigned char*)"ACT", tmp_remote_client->active_state);
      pack_add_as_int   (&tmp_pack, (unsigned char*)"REG", tmp_remote_client->register_state);

      pack_add_as_pack(pack, (unsigned char*)PACK_PARAM_KEY, &tmp_pack);
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_map(pack_packet_t *pack)
{
  if(pack == NULL)
    return make_last_error(ERROR_NORMAL, errno, "cmd_map, pack == NULL");

  pack_init(pack);
  pack_add_cmd(pack, (unsigned char*)"map");

  for(int i = 0; i < map()->count; i++)
  {
    pack_packet_t tmp_pack;
    pack_init(&tmp_pack);

    pack_add_as_string(&tmp_pack, (unsigned char*)"KND", (unsigned char*)map()->items[i].kind);
    pack_add_as_string(&tmp_pack, (unsigned char*)"NUM", (unsigned char*)map()->items[i].number);
    pack_add_as_string(&tmp_pack, (unsigned char*)"IND", (unsigned char*)map()->items[i].index);
    pack_add_as_string(&tmp_pack, (unsigned char*)"LAF", (unsigned char*)map()->items[i].lat_f);
    pack_add_as_string(&tmp_pack, (unsigned char*)"LOF", (unsigned char*)map()->items[i].lon_f);
    pack_add_as_string(&tmp_pack, (unsigned char*)"_LA", (unsigned char*)map()->items[i].lat);
    pack_add_as_string(&tmp_pack, (unsigned char*)"_LO", (unsigned char*)map()->items[i].lon);

    pack_add_as_pack(pack, (unsigned char*)PACK_PARAM_KEY, &tmp_pack);
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_remote_client_activate(sock_id_t id, sock_active_t active)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_t *tmp_client = &_cmd_server.custom_remote_clients_list.items[i];

    if(tmp_client->custom_worker.state == STATE_START)
    {
      sock_id_t tmp_id = tmp_client->custom_worker.id;

      if(tmp_id == id)
      {
        sock_active_t cur_active;

        if(active == ACTIVE_NEXT)
          cur_active = _cmd_active_next();
        else
          cur_active = active;

        time_t rawtime;
        time (&rawtime);
        tmp_client->active_time = rawtime;

        tmp_client->active_state = cur_active;

        switch (cur_active)
        {
          case ACTIVE_FIRST:
            log_add_fmt(LOG_INFO, "[CMD] cmd_remote_client_activate, acivate first, worker id: %d",
                        id);
            break;
          case ACTIVE_SECOND:
            log_add_fmt(LOG_INFO, "[CMD] cmd_remote_client_activate, acivate second, worker id: %d",
                        id);
            break;
          default:
            log_add_fmt(LOG_INFO, "[CMD] cmd_remote_client_activate, deacivate, worker id: %d",
                        id);
            break;
        }

        pack_packet_t tmp_packet;
        cmd_remote_client_list(&tmp_packet);
        return ws_server_send_pack(SOCK_SEND_TO_ALL, &tmp_packet);
      }
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
//  log_add_fmt(LOG_DEBUG, "[CMD] cmd_remote_client_register, worker id: %d, session id: %s",
//              id, session_id);

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_t *client = &_cmd_server.custom_remote_clients_list.items[i];

    if(client->custom_worker.state == STATE_START)
      if(client->custom_worker.id == id)
      {
//        log_add_fmt(LOG_DEBUG, "[CMD] cmd_remote_client_register, client found, worker id: %d, session id: %s",
//                    id, session_id);

        time_t rawtime;
        time(&rawtime);
        client->register_time = rawtime;

        client->register_state = REGISTER_OK;

        strcpy((char*)client->custom_worker.session_id, (char*)session_id);
        strcpy((char*)client->custom_worker.name, get_name_by_session_id((char*)session_id));

        log_add_fmt(LOG_INFO, "[CMD] cmd_remote_clients_register, worker id: %d, session_id: %s, name: %s",
                    client->custom_worker.id,
                    client->custom_worker.session_id,
                    client->custom_worker.name);

        int res = ERROR_NONE;

        if(res == ERROR_NONE)
        {
          pack_packet_t tmp_packet;
          cmd_remote_client_list(&tmp_packet);
          res = ws_server_send_pack(SOCK_SEND_TO_ALL, &tmp_packet);
        }

        return res;
      }
  }

  char tmp[256];
  sprintf(tmp, "[CMD] cmd_remote_clients_register, client not found, worker id: %d, session id: %s",
          id, session_id);
  log_add(LOG_ERROR, tmp);
  return make_last_error(ERROR_NORMAL, errno, tmp);
}
//==============================================================================
