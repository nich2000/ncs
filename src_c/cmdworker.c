//==============================================================================
//==============================================================================
#include "cmdworker.h"
#include "socket_utils.h"
#include "socket.h"
#include "utils.h"
#include "ncs_log.h"
#include "protocol_types.h"
//==============================================================================
int cmd_server_init (cmd_server_t *server);
int cmd_server_start(cmd_server_t *server, sock_port_t port);
int cmd_server_stop (cmd_server_t *server);
int cmd_server_pause(cmd_server_t *server);
//==============================================================================
void *cmd_server_worker(void *arg);
//==============================================================================
int cmd_accept(void *sender, SOCKET socket, sock_host_t host);
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
int cmd_connect(void *sender);
int cmd_disconnect(void *sender);
//==============================================================================
void *cmd_send_worker(void *arg);
//==============================================================================
int cmd_send(void *sender);
int cmd_recv(void *sender, char *buffer, int size);
//==============================================================================
int          _cmd_server_id = 0;
cmd_server_t _cmd_server;
//==============================================================================
int          _cmd_client_id = 0;
cmd_client_t _cmd_client;
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

  return SOCK_OK;
}
//==============================================================================
int cmd_server_status()
{
  clr_scr();

  sock_print_custom_worker_info(&_cmd_server.custom_server.custom_worker, "cmd_server");

  sock_print_custom_remote_clients_list_info(&_cmd_server.custom_remote_clients_list, "cmd_server");
}
//==============================================================================
int cmd_client(sock_state_t state, sock_port_t port, sock_host_t host)
{
  sock_print_client_header(port, host);

  switch(state)
  {
    case SOCK_STATE_NONE:
    {
      break;
    }
    case SOCK_STATE_START:
    {
      cmd_client_start(&_cmd_client, port, host);
      break;
    }
    case SOCK_STATE_STOP:
    {
      cmd_client_stop(&_cmd_client);
      break;
    }
    case SOCK_STATE_PAUSE:
    {
      cmd_client_pause(&_cmd_client);
      break;
    }
    default:;
  };

  return SOCK_OK;
}
//==============================================================================
int cmd_client_status()
{
  clr_scr();

  sock_print_custom_worker_info(&_cmd_client.custom_client.custom_remote_client.custom_worker, "cmd_client");
}
//==============================================================================
int cmd_server_init(cmd_server_t *server)
{
  custom_server_init(&server->custom_server);

  custom_remote_clients_list_init(&server->custom_remote_clients_list);

  server->custom_server.custom_worker.id   = _cmd_server_id++;
  server->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  server->custom_server.custom_worker.mode = SOCK_MODE_CMD_SERVER;

  server->custom_server.on_accept          = &cmd_accept;
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

  return pthread_create(&server->custom_server.work_thread, &tmp_attr, cmd_server_worker, (void*)server);
}
//==============================================================================
int cmd_server_stop(cmd_server_t *server)
{
  server->custom_server.custom_worker.state = SOCK_STATE_STOP;
}
//==============================================================================
int cmd_server_pause(cmd_server_t *worker)
{
  worker->custom_server.custom_worker.state = SOCK_STATE_PAUSE;
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

      tmp_client->custom_worker.id    = _cmd_server.custom_remote_clients_list.next_id++;
      tmp_client->custom_worker.type  = SOCK_TYPE_REMOTE_CLIENT;
      tmp_client->custom_worker.mode  = _cmd_server.custom_server.custom_worker.mode;
      tmp_client->custom_worker.port  = _cmd_server.custom_server.custom_worker.port;
      tmp_client->custom_worker.state = SOCK_STATE_START;

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
    log_add(tmp, LOG_CRITICAL_ERROR);
    return SOCK_ERROR;
  };

  memcpy(&tmp_client->custom_worker.sock, &socket, sizeof(SOCKET));
  memcpy(tmp_client->custom_worker.host, host,   SOCK_HOST_SIZE);

  sprintf(tmp, "cmd_accept, socket: %d, host: %s", tmp_client->custom_worker.sock, tmp_client->custom_worker.host);
  log_add(tmp, LOG_DEBUG);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(&tmp_client->recv_thread, &tmp_attr, custom_recv_worker, (void*)tmp_client);
  pthread_create(&tmp_client->send_thread, &tmp_attr, cmd_send_worker,    (void*)tmp_client);
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

  client->custom_client.on_connect = cmd_connect;
  client->custom_client.custom_remote_client.on_disconnect = cmd_disconnect;
}
//==============================================================================
int cmd_client_start(cmd_client_t *client, sock_port_t port, sock_host_t host)
{
  cmd_client_init(client);

  client->custom_client.custom_remote_client.custom_worker.port = port;
  client->custom_client.custom_remote_client.custom_worker.state = SOCK_STATE_START;
  strcpy(client->custom_client.custom_remote_client.custom_worker.host, host);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  return pthread_create(&client->custom_client.work_thread, &tmp_attr, cmd_client_worker, (void*)client);
}
//==============================================================================
int cmd_client_stop(cmd_client_t *client)
{
  client->custom_client.custom_remote_client.custom_worker.state = SOCK_STATE_STOP;
}
//==============================================================================
int cmd_client_pause(cmd_client_t *client)
{
  client->custom_client.custom_remote_client.custom_worker.state = SOCK_STATE_PAUSE;
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
}
//==============================================================================
int cmd_disconnect(void *sender)
{
  log_add("cmd_disconnect", LOG_DEBUG);
}
//==============================================================================
void *cmd_send_worker(void *arg)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  char tmp[1024];
  sprintf(tmp, "BEGIN cmd_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

//  pack_protocol_t *tmp_protocol;
//  pack_packet_t   *tmp_pack;
//  pack_buffer_t    tmp_buffer;
//  int              tmp_size;

  while(tmp_client->custom_worker.state == SOCK_STATE_START)
  {
//    tmp_pack = _pack_next(&tmp_worker->protocol);
//    while(tmp_pack != NULL)
//    {
//      if(pack_packet_to_buffer(tmp_pack, tmp_buffer, &tmp_size) != PACK_OK)
//        continue;

//      int tmp_cnt = pack_buffer_validate(tmp_buffer, tmp_size, PACK_VALIDATE_ONLY, &tmp_worker->protocol);

//      if(tmp_cnt > 0)
//      {
//        sock_send(tmp_sock, tmp_buffer, tmp_size);
//      }

//      tmp_pack = _pack_next(&tmp_worker->protocol);
//    }

    usleep(1000);
  }

  sprintf(tmp, "END cmd_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);
}
//==============================================================================
int cmd_send(void *sender)
{
}
//==============================================================================
int cmd_recv(void *sender, char *buffer, int size)
{
}
//==============================================================================
