//==============================================================================
//==============================================================================
#include <stdarg.h>
#include <string.h>

#include "cmdworker.h"
#include "wsworker.h"
#include "socket_utils.h"
#include "socket.h"
#include "utils.h"
#include "ncs_log.h"
#include "protocol.h"
#include "protocol_types.h"
#include "protocol_utils.h"
//==============================================================================
int cmd_server_init (cmd_server_t *server);
int cmd_server_start(cmd_server_t *server, sock_port_t port);
int cmd_server_stop (cmd_server_t *server);
int cmd_server_pause(cmd_server_t *server);
//==============================================================================
void *cmd_server_worker(void *arg);
//==============================================================================
custom_remote_client_t *cmd_server_remote_clients_next();
int _cmd_server_remote_clients_count(custom_remote_clients_list_t *clients_list);
//==============================================================================
int cmd_client_init (cmd_client_t *client);
int cmd_client_start(cmd_client_t *client, sock_port_t port, sock_host_t host);
int cmd_client_stop (cmd_client_t *client);
int cmd_client_pause(cmd_client_t *client);
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
int cmd_recv      (void *sender, char *buffer, int size);
int cmd_new_data  (void *sender, void *data);
//==============================================================================
int cmd_exec(pack_packet_t *packet);
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
    case SOCK_STATE_NONE:
    {
      break;
    }
    case SOCK_STATE_START:
    {
      cmd_server_start(&_cmd_server, port);
      break;
    }
    case SOCK_STATE_STOP:
    {
      cmd_server_stop(&_cmd_server);
      break;
    }
    case SOCK_STATE_PAUSE:
    {
      cmd_server_pause(&_cmd_server);
      break;
    }
    default:;
  };

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_status()
{
  clr_scr();

  sock_print_custom_worker_info(&_cmd_server.custom_server.custom_worker, "cmd_server");

  sock_print_custom_remote_clients_list_info(&_cmd_server.custom_remote_clients_list, "cmd_server");

  return ERROR_NONE;
}
//==============================================================================
int cmd_client(sock_state_t state, sock_port_t port, sock_host_t host, int count)
{
  sock_print_client_header(port, host);

  if(count >= SOCK_WORKERS_COUNT)
  {
    log_add("cmd_client, too match count", LOG_ERROR_CRITICAL);
    return ERROR_CRITICAL;
  }

  _cmd_client_count = count;

  for(int i = 0; i < _cmd_client_count; i++)
  {
    switch(state)
    {
      case SOCK_STATE_NONE:
      {
        break;
      }
      case SOCK_STATE_START:
      {
        cmd_client_start(&_cmd_client[i], port, host);
        break;
      }
      case SOCK_STATE_STOP:
      {
        cmd_client_stop(&_cmd_client[i]);
        break;
      }
      case SOCK_STATE_PAUSE:
      {
        cmd_client_pause(&_cmd_client[i]);
        break;
      }
      default:;
    };
  };

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_status()
{
  clr_scr();

  for(int i = 0; i < _cmd_client_count; i++)
    sock_print_custom_worker_info(&_cmd_client[i].custom_client.custom_remote_client.custom_worker, "cmd_client");

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
//  };

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
  server->custom_server.custom_worker.state = SOCK_STATE_START;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(&server->custom_server.work_thread, &tmp_attr, cmd_server_worker, (void*)server);

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_stop(cmd_server_t *server)
{
  server->custom_server.custom_worker.state = SOCK_STATE_STOP;

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_pause(cmd_server_t *worker)
{
  worker->custom_server.custom_worker.state = SOCK_STATE_PAUSE;

  return ERROR_NONE;
}
//==============================================================================
int _cmd_server_remote_clients_count(custom_remote_clients_list_t *clients_list)
{
  int tmp_count = 0;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(clients_list->items[i].custom_worker.state == SOCK_STATE_START)
      tmp_count++;

  return tmp_count;
}
//==============================================================================
custom_remote_client_t *cmd_server_remote_clients_next()
{
  custom_remote_client_t *tmp_client = 0;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.state == SOCK_STATE_STOP)
    {
      tmp_client = &_cmd_server.custom_remote_clients_list.items[i];

      custom_remote_client_init(tmp_client);

      tmp_client->protocol.on_new_in_data = cmd_new_data;

      tmp_client->custom_worker.id    = _cmd_server.custom_remote_clients_list.next_id++;
      tmp_client->custom_worker.type  = SOCK_TYPE_REMOTE_CLIENT;
      tmp_client->custom_worker.mode  = _cmd_server.custom_server.custom_worker.mode;
      tmp_client->custom_worker.port  = _cmd_server.custom_server.custom_worker.port;
      tmp_client->custom_worker.state = SOCK_STATE_START;

      tmp_client->on_disconnect       = cmd_disconnect;
      tmp_client->on_error            = cmd_error;
      tmp_client->on_recv             = cmd_recv;
      tmp_client->on_send             = cmd_send;

      break;
    };

  return tmp_client;
}
//==============================================================================
int cmd_accept(void *sender, SOCKET socket, sock_host_t host)
{
  custom_remote_client_t *tmp_client = cmd_server_remote_clients_next();

  char tmp[256];

  if(tmp_client == 0)
  {
    sprintf(tmp,
            "no available clients, cmd_accept, socket: %d, host: %s",
            tmp_client->custom_worker.sock, tmp_client->custom_worker.host);
    log_add(tmp, LOG_ERROR_CRITICAL);
    return ERROR_NORMAL;
  };

  memcpy(&tmp_client->custom_worker.sock, &socket, sizeof(SOCKET));
  memcpy(tmp_client->custom_worker.host, host,   SOCK_HOST_SIZE);

  sprintf(tmp, "remote client connected, host: %s", tmp_client->custom_worker.host);
  log_add(tmp, LOG_INFO);

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
  log_add("BEGIN cmd_server_worker", LOG_DEBUG);

  cmd_server_t *tmp_server = (cmd_server_t*)arg;

  custom_server_start(&tmp_server->custom_server.custom_worker);
  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  log_add("END cmd_server_worker", LOG_DEBUG);
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

  pack_protocol_init(&client->custom_client.custom_remote_client.protocol);

  client->custom_client.custom_remote_client.protocol.on_new_in_data  = cmd_new_data;
//  client->custom_client.custom_remote_client.protocol.on_new_out_data = cmd_new_data;

  client->custom_client.custom_remote_client.custom_worker.port = port;
  client->custom_client.custom_remote_client.custom_worker.state = SOCK_STATE_START;
  strcpy(client->custom_client.custom_remote_client.custom_worker.host, host);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(&client->custom_client.work_thread, &tmp_attr, cmd_client_worker, (void*)client);

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_stop(cmd_client_t *client)
{
  client->custom_client.custom_remote_client.custom_worker.state = SOCK_STATE_STOP;

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_pause(cmd_client_t *client)
{
  client->custom_client.custom_remote_client.custom_worker.state = SOCK_STATE_PAUSE;

  return ERROR_NONE;
}
//==============================================================================
void *cmd_client_worker(void *arg)
{
  log_add("BEGIN cmd_client_worker", LOG_DEBUG);

  cmd_client_t *tmp_client = (cmd_client_t*)arg;

  do
  {
    custom_client_start(&tmp_client->custom_client.custom_remote_client.custom_worker);
    custom_client_work (&tmp_client->custom_client);
    custom_worker_stop (&tmp_client->custom_client.custom_remote_client.custom_worker);
  }
  while(tmp_client->custom_client.custom_remote_client.custom_worker.state == SOCK_STATE_START);

  log_add("END cmd_client_worker", LOG_DEBUG);
}
//==============================================================================
int cmd_connect(void *sender)
{
  log_add("BEGIN cmd_connect", LOG_DEBUG);

  custom_client_t *tmp_client = (custom_client_t*)sender;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

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

  log_add("END cmd_connect", LOG_DEBUG);

  return ERROR_NONE;
}
//==============================================================================
void *cmd_send_worker(void *arg)
{
//  log_add("cmd_send_worker", LOG_INFO);

  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  char tmp[1024];
  sprintf(tmp, "BEGIN cmd_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  pack_protocol_t *tmp_protocol = &tmp_client->protocol;
  pack_packet_t   *tmp_pack;
  pack_buffer_t    tmp_buffer;
  pack_size_t      tmp_size;

  while(tmp_client->custom_worker.state == SOCK_STATE_START)
  {
    tmp_pack = _pack_next(tmp_protocol);
    while(tmp_pack != NULL)
    {
      if(pack_packet_to_buffer(tmp_pack, tmp_buffer, &tmp_size) != ERROR_NONE)
        continue;

      int tmp_cnt = pack_buffer_validate(tmp_buffer, tmp_size, PACK_VALIDATE_ONLY,
                                         tmp_protocol, (void*)tmp_client);

      if(tmp_cnt > 0)
      {
        if(sock_send(tmp_sock, tmp_buffer, (int)tmp_size) == ERROR_NONE)
          if(tmp_client->on_send != 0)
            tmp_client->on_send((void*)tmp_client);
      }

      tmp_pack = _pack_next(tmp_protocol);
    }

    usleep(1000);
  }

  sprintf(tmp, "END cmd_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);
}
//==============================================================================
int cmd_disconnect(void *sender)
{
  log_add("cmd_disconnect", LOG_DEBUG);

  return ERROR_NONE;
}
//==============================================================================
int cmd_error(void *sender, error_t *error)
{
  log_add("cmd_error", LOG_DEBUG);

  return ERROR_NONE;
}
//==============================================================================
int cmd_send(void *sender)
{
//  log_add("cmd_send", LOG_DEBUG);

  custom_remote_client_t *tmp_client = (custom_remote_client_t*)sender;

  pack_protocol_t *tmp_protocol = &tmp_client->protocol;

  pack_packet_t *tmp_packet = _pack_pack_current(PACK_OUT, tmp_protocol);

  pack_print(tmp_packet, "cmd_send", 1, 0, 1, 0);

  return ERROR_NONE;
}
//==============================================================================
int cmd_recv(void *sender, char *buffer, int size)
{
//  log_add("cmd_recv", LOG_INFO);

  custom_remote_client_t *tmp_client = (custom_remote_client_t*)sender;

  pack_protocol_t *tmp_protocol = &tmp_client->protocol;

  pack_buffer_validate(buffer, size, PACK_VALIDATE_ADD,
                       tmp_protocol, (void*)tmp_client);

  return ERROR_NONE;
}
//==============================================================================
int cmd_server_send(int argc, ...)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_t *tmp_client = &_cmd_server.custom_remote_clients_list.items[i];

    if(tmp_client->custom_worker.state == SOCK_STATE_START)
    {
      pack_protocol_t *protocol = &tmp_client->protocol;

      pack_begin(protocol);

      va_list params;
      va_start(params, argc);

      char *cmd = va_arg(params, char*);
      pack_add_cmd(cmd, protocol);

      for(int i = 1; i < argc; i++)
      {
        char *param = va_arg(params, char*);
        pack_add_param_as_string(param, protocol);
      };

      va_end(params);

      pack_end(protocol);
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_client_send(int argc, ...)
{
  for(int i = 0; i < _cmd_client_count; i++)
  {
    pack_protocol_t *tmp_protocol = &_cmd_client[i].custom_client.custom_remote_client.protocol;

    pack_begin(tmp_protocol);

    va_list tmp_params;
    va_start(tmp_params, argc);

    char *tmp_cmd = va_arg(tmp_params, char*);
    pack_add_cmd(tmp_cmd, tmp_protocol);

    for(int i = 1; i < argc; i++)
    {
      char *tmp_param = va_arg(tmp_params, char*);
      pack_add_param_as_string(tmp_param, tmp_protocol);
    };

    va_end(tmp_params);

    pack_end(tmp_protocol);
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_new_data(void *sender, void *data)
{
//  log_add("cmd_new_data", LOG_INFO);

  custom_remote_client_t *tmp_client = (custom_remote_client_t*)sender;

  pack_packet_t *tmp_packet = (pack_packet_t*)data;

  if(cmd_exec(tmp_packet) == ERROR_NONE)
  {
    log_add("cmd_exec", LOG_INFO);
  }
  else
  {
    char csv[256];
    int res = pack_values_to_csv(tmp_packet, ';', csv);
    if(res != ERROR_NONE)
    {
      log_add_fmt(LOG_ERROR, "cmd_new_data, res = %d", res);
    }
    else
    {
      int len = strlen(csv);

      int cnt = report_add(tmp_client->report, csv);
      if(cnt != (len+1))
      {
        log_add_fmt(LOG_ERROR, "cmd_new_data, cnt = %d, len = %d", cnt, len);
      }
    }
  }

  ws_server_route_pack(tmp_packet);

  return ERROR_NONE;
}
//==============================================================================
int cmd_exec(pack_packet_t *packet)
{
  pack_value_t tmp_command;
  pack_value_t tmp_param;
  pack_index_t tmp_index = 0;

  char tmp[1024];

  if(pack_command(packet, tmp_command) == ERROR_NONE)
  {
    sprintf(tmp, "Command: %s", tmp_command);

    while(pack_next_param(packet, &tmp_index, tmp_param) == ERROR_NONE)
    {
      sprintf(tmp, "%s\nParam: %s", tmp, tmp_param);
    }

    log_add(tmp, LOG_INFO);

    return ERROR_NONE;
  }
  else
    return ERROR_NORMAL;
}
//==============================================================================
