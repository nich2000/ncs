//==============================================================================
//==============================================================================
#include <stdarg.h>
#include <string.h>

#include "streamer.h"
#include "exec.h"
#include "customworker.h"
#include "cmdworker.h"
#include "wsworker.h"
#include "socket_utils.h"
#include "socket.h"
#include "utils.h"
#include "ncs_log.h"
#include "protocol.h"
#include "protocol_types.h"
//==============================================================================
int cmd_server_init (cmd_server_t *server);
int cmd_server_start(cmd_server_t *server, sock_port_t port);
int cmd_server_stop (cmd_server_t *server);
int cmd_server_pause(cmd_server_t *server);
int cmd_server_resume(cmd_server_t *server);
//==============================================================================
void *cmd_server_worker(void *arg);
//==============================================================================
custom_remote_client_t *_cmd_server_remote_clients_next(cmd_server_t *cmd_server);
//==============================================================================
int cmd_client_init (cmd_client_t *client);
int cmd_client_start(cmd_client_t *client, sock_port_t port, sock_host_t host);
int cmd_client_stop (cmd_client_t *client);
int cmd_client_pause(cmd_client_t *client);
int cmd_client_resume(cmd_client_t *client);
//==============================================================================
void *cmd_client_worker(void *arg);
//==============================================================================
void *cmd_send_worker(void *arg);
//==============================================================================
int cmd_accept    (void *sender, SOCKET socket, sock_host_t host);
int cmd_connect   (void *sender);
int cmd_disconnect(void *sender);
int cmd_error     (void *sender, error_t *error);
int cmd_send      (void *sender);
int cmd_recv      (void *sender, unsigned char *buffer, int size);
int cmd_new_data  (void *sender, void *data);
//==============================================================================
int          _cmd_server_id = 0;
cmd_server_t _cmd_server;
//==============================================================================
int          _cmd_client_id    = 0;
int          _cmd_client_count = 0;
cmd_client_t _cmd_client[SOCK_WORKERS_COUNT];
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
  {
    log_add("cmd_client, too match count", LOG_ERROR_CRITICAL);
    return ERROR_CRITICAL;
  }

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
  custom_server_init(&server->custom_server);

  custom_remote_clients_list_init(&server->custom_remote_clients_list);

//  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
//  {
//    custom_remote_client_t *tmp_client = &server->custom_remote_clients_list.items[i];
//    tmp_client->protocol.on_new_in_data  = cmd_new_data;
//    tmp_client->protocol.on_new_out_data = cmd_new_data;
//  }

  server->custom_server.custom_worker.id   = _cmd_server_id++;
  server->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  server->custom_server.custom_worker.mode = SOCK_MODE_CMD_SERVER;

  server->custom_server.on_accept          = &cmd_accept;

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_start(cmd_server_t *server, sock_port_t port)
{
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
custom_remote_client_t *_cmd_server_remote_clients_next(cmd_server_t *cmd_server)
{
  custom_remote_client_t *tmp_client = _custom_server_remote_clients_next(&cmd_server->custom_remote_clients_list);

  if(tmp_client != NULL)
  {
    custom_remote_client_init(tmp_client);

    tmp_client->protocol.on_new_in_data = cmd_new_data;

    tmp_client->custom_worker.id    = cmd_server->custom_remote_clients_list.next_id++;
    tmp_client->custom_worker.type  = SOCK_TYPE_REMOTE_CLIENT;
    tmp_client->custom_worker.mode  = cmd_server->custom_server.custom_worker.mode;
    tmp_client->custom_worker.port  = cmd_server->custom_server.custom_worker.port;

    tmp_client->on_disconnect       = cmd_disconnect;
    tmp_client->on_error            = cmd_error;
    tmp_client->on_recv             = cmd_recv;
    tmp_client->on_send             = cmd_send;
  }

  return tmp_client;
}
//==============================================================================
int cmd_accept(void *sender, SOCKET socket, sock_host_t host)
{
  custom_remote_client_t *tmp_client = _cmd_server_remote_clients_next(&_cmd_server);

  if(tmp_client == 0)
  {
    log_add_fmt(LOG_ERROR_CRITICAL,
                "cmd_accept, no available clients, socket: %d, host: %s",
                socket, host);
    return ERROR_CRITICAL;
  }

  tmp_client->custom_worker.state = STATE_STARTING;
  if(tmp_client->custom_worker.on_state != NULL)
    tmp_client->custom_worker.on_state(tmp_client, STATE_STARTING);

  memcpy(&tmp_client->custom_worker.sock, &socket, sizeof(SOCKET));
  memcpy(tmp_client->custom_worker.host, host,   SOCK_HOST_SIZE);

  log_add_fmt(LOG_DEBUG,
              "cmd_accept, socket: %d, host: %s, port: %d",
              tmp_client->custom_worker.sock, tmp_client->custom_worker.host, tmp_client->custom_worker.port);

  ws_server_send_clients();

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

  client->custom_client.custom_remote_client.custom_worker.id = _cmd_client_id++;
  client->custom_client.custom_remote_client.custom_worker.type = SOCK_TYPE_CLIENT;
  client->custom_client.custom_remote_client.custom_worker.mode = SOCK_MODE_CMD_CLIENT;

  client->custom_client.on_connect                         = cmd_connect;

  client->custom_client.custom_remote_client.on_disconnect = cmd_disconnect;
  client->custom_client.custom_remote_client.on_error      = cmd_error;
  client->custom_client.custom_remote_client.on_recv       = cmd_recv;
  client->custom_client.custom_remote_client.on_send       = cmd_send;

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_start(cmd_client_t *client, sock_port_t port, sock_host_t host)
{
  cmd_client_init(client);

  protocol_init(&client->custom_client.custom_remote_client.protocol);

  client->custom_client.custom_remote_client.protocol.on_new_in_data  = cmd_new_data;
//  client->custom_client.custom_remote_client.protocol.on_new_out_data = cmd_new_data;

  client->custom_client.custom_remote_client.custom_worker.port = port;
  strcpy((char*)client->custom_client.custom_remote_client.custom_worker.host, (char*)host);

  client->custom_client.custom_remote_client.custom_worker.state = STATE_STARTING;
  if(client->custom_client.custom_remote_client.custom_worker.on_state != NULL)
    client->custom_client.custom_remote_client.custom_worker.on_state(client, STATE_STARTING);

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
int cmd_connect(void *sender)
{
  log_add("[BEGIN] cmd_connect", LOG_DEBUG);

  log_add("connected to server", LOG_INFO);

  custom_client_t *tmp_client = (custom_client_t*)sender;

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

      int tmp_cnt = protocol_buffer_validate(tmp_buffer, tmp_size, PACK_VALIDATE_ONLY,
                                         tmp_protocol, (void*)tmp_client);

      if(tmp_cnt > 0)
      {
        if(sock_send(tmp_sock, (char*)tmp_buffer, (int)tmp_size) == ERROR_NONE)
          if(tmp_client->on_send != 0)
            tmp_client->on_send((void*)tmp_client);
      }

      tmp_pack = _protocol_next_pack(tmp_protocol);
    }

    usleep(1000);
  }

  tmp_client->custom_worker.state = STATE_STOP;
  if(tmp_client->custom_worker.on_state != NULL)
    tmp_client->custom_worker.on_state(tmp_client, STATE_STOP);

  log_add_fmt(LOG_DEBUG, "[END] cmd_send_worker, socket: %d", tmp_sock);

  return NULL;
}
//==============================================================================
int cmd_disconnect(void *sender)
{
  log_add("cmd_disconnect, disconnected from server", LOG_INFO);

  return ERROR_NONE;
}
//==============================================================================
int cmd_error(void *sender, error_t *error)
{
  log_add_fmt(LOG_INFO, "cmd_error, message: %s", error->message);

  return ERROR_NONE;
}
//==============================================================================
int cmd_send(void *sender)
{
  #ifdef PRINT_SND_PACK
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)sender;

  pack_protocol_t *tmp_protocol = &tmp_client->protocol;

  pack_packet_t *tmp_packet = _protocol_current_pack(PACK_OUT, tmp_protocol);

  print_pack(tmp_packet, "cmd_send", 1, 0, 1, 0);
  #endif

  return ERROR_NONE;
}
//==============================================================================
int cmd_recv(void *sender, unsigned char *buffer, int size)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)sender;

  pack_protocol_t *tmp_protocol = &tmp_client->protocol;

  protocol_buffer_validate(buffer, size, PACK_VALIDATE_ADD, tmp_protocol, (void*)tmp_client);

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

      unsigned char *cmd = va_arg(params, unsigned char*);
      protocol_add_cmd(cmd, tmp_protocol);

      for(int i = 1; i < argc; i++)
      {
        unsigned char *param = va_arg(params, unsigned char*);
        protocol_add_param_as_string(param, tmp_protocol);
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

    unsigned char *tmp_cmd = va_arg(tmp_params, unsigned char*);
    protocol_add_cmd(tmp_cmd, tmp_protocol);

    for(int i = 1; i < argc; i++)
    {
      unsigned char *tmp_param = va_arg(tmp_params, unsigned char*);
      protocol_add_param_as_string(tmp_param, tmp_protocol);
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
int cmd_new_data(void *sender, void *data)
{
//  log_add("cmd_new_data", LOG_INFO);

  custom_remote_client_t *tmp_client = (custom_remote_client_t*)sender;

  pack_packet_t *tmp_packet = (pack_packet_t*)data;

  if(_pack_is_command(tmp_packet))
  {
    return handle_command_pack(tmp_packet);
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

    if(tmp_client->active)
      ws_server_send_pack(tmp_packet);
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_list(pack_packet_t *pack)
{
  if(pack == NULL)
    return ERROR_NORMAL;

  pack_init(pack);

  pack_add_cmd(pack, (unsigned char*)"clients");

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
    {
      char tmp[128];
      sprintf(tmp,
              "%s %d",
              _cmd_server.custom_remote_clients_list.items[i].custom_worker.name,
              _cmd_server.custom_remote_clients_list.items[i].custom_worker.id);
      pack_add_param(pack, (unsigned char*)tmp);
    }
  };

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_activate(sock_id_t id)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.id == id)
        _cmd_server.custom_remote_clients_list.items[i].active = TRUE;
  }

  return ERROR_NONE;
}
//==============================================================================
