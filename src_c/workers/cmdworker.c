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
#include "ncs_pack_utils.h"
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
int           _cmd_server_id = 0;
cmd_server_t  _cmd_server;
//==============================================================================
int          _cmd_client_count = 0;
cmd_client_t _cmd_client[SOCK_WORKERS_COUNT];
//==============================================================================
sock_active_t _cmd_active = ACTIVE_FIRST;
//-----------------------------------------------------------------------------
sock_active_t _cmd_active_next()
{
  if(_cmd_active == ACTIVE_FIRST)
    _cmd_active = ACTIVE_SECOND;
  else
    _cmd_active = ACTIVE_FIRST;

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
  custom_server_init(STATIC_CMD_SERVER_ID, &server->custom_server);

  custom_remote_clients_init(&server->custom_remote_clients_list);

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
  {
    log_add_fmt(LOG_ERROR_CRITICAL,
                "cmd_accept, no available clients, socket: %d, host: %s",
                socket, host);
    return ERROR_CRITICAL;
  }

  tmp_client->custom_worker.on_state = on_server_cmd_state;

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
  char *tmp[128];
  sprintf(tmp, "sndtosr register %s", client->custom_client.custom_remote_client.custom_worker.name);
  handle_command_str(client, tmp);
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

  // TODO - analog on_connect - need to send clients list to ws - govnokod
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
      pack_add_as_int(tmp_packet, "ACT", tmp_client->active_state);

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
int cmd_remote_clients_list(pack_packet_t *pack)
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
      pack_add_as_int   (&tmp_pack, "_ID", tmp_custom_worker->id);
      pack_add_as_string(&tmp_pack, "NAM", tmp_custom_worker->name);
      pack_add_as_string(&tmp_pack, "CAP", tmp_custom_worker->caption);
      pack_add_as_int   (&tmp_pack, "STA", tmp_custom_worker->state);
      pack_add_as_int   (&tmp_pack, "ACT", tmp_remote_client->active_state);
      pack_add_as_int   (&tmp_pack, "REG", tmp_remote_client->register_state);

      pack_add_as_pack(pack, (unsigned char*)PACK_PARAM_KEY, &tmp_pack);
    }
  };

  print_pack(pack, "clients", FALSE, FALSE, TRUE, TRUE);

  return ERROR_NONE;
}
//==============================================================================
int cmd_remote_clients_activate(sock_id_t id, sock_active_t active)
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
        cmd_remote_clients_list(&tmp_packet);
        return ws_server_send_pack(SOCK_SEND_TO_ALL, &tmp_packet);
      }
  }

  return make_last_error();
}
//==============================================================================
int cmd_remote_clients_activate_all(sock_active_t active, sock_active_t except)
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
int cmd_remote_clients_register(sock_id_t id, sock_name_t name)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.state == STATE_START)
      if(_cmd_server.custom_remote_clients_list.items[i].custom_worker.id == id)
      {
        strcpy(_cmd_server.custom_remote_clients_list.items[i].custom_worker.name, name);

        _cmd_server.custom_remote_clients_list.items[i].register_state = REGISTER_OK;

        log_add_fmt(LOG_INFO, "register(%d)", id);

        pack_packet_t tmp_packet;
        cmd_remote_clients_list(&tmp_packet);
        return ws_server_send_pack(SOCK_SEND_TO_ALL, &tmp_packet);
      }
  }

  return make_last_error();
}
//==============================================================================
