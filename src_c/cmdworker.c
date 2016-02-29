//==============================================================================
//==============================================================================
#include "cmdworker.h"
#include "socket_utils.h"
#include "socket.h"
#include "utils.h"
#include "log.h"
//==============================================================================
int cmd_server_init (cmd_server_t *server);
int cmd_server_start(cmd_server_t *server, sock_port_t port);
int cmd_server_stop (cmd_server_t *server);
int cmd_server_pause(cmd_server_t *server);
//==============================================================================
void *cmd_server_worker(void *arg);
//==============================================================================
int cmd_accept(void *sender, SOCKET socket);
//==============================================================================
custom_remote_client_t *cmd_server_next_client();
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
void *cmd_recv_worker(void *arg);
void *cmd_send_worker(void *arg);
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
  sock_print_custom_worker_info(&_cmd_server.custom_server.custom_worker, "cmd_server");
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
  sock_print_custom_worker_info(&_cmd_client.custom_client.custom_worker, "cmd_client");
}
//==============================================================================
int cmd_server_init(cmd_server_t *server)
{
  server->custom_remote_clients_list.index    = 0;
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    custom_worker_init(&server->custom_remote_clients_list.items[i].custom_worker);

  custom_worker_init(&server->custom_server.custom_worker);

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
custom_remote_client_t *cmd_server_next_client()
{
}
//==============================================================================
int cmd_accept(void *sender, SOCKET socket)
{
  char tmp[1024];
  sprintf(tmp, "cmd_accept, socket: %d", socket);
  log_add(tmp, LOG_DEBUG);

  SOCKET *s = malloc(sizeof(SOCKET));
  memcpy(s, &socket, sizeof(SOCKET));

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(NULL, &tmp_attr, cmd_recv_worker, (void*)s);
  pthread_create(NULL, &tmp_attr, cmd_send_worker, (void*)s);

  /*
  #ifdef SOCK_USE_FREE_INDEX
  int tmp_index;
  if(sock_find_free_index(server, &tmp_index))
  {
    sock_worker_t *tmp_worker = &server->clients.items[tmp_index];
  #else
  if(server->clients.index < SOCK_WORKERS_COUNT)
  {
    sock_worker_t *tmp_worker = &server->clients.items[server->clients.index];
    server->clients.index++;
  #endif
    server->clients.last_id++;

    sock_worker_init(tmp_worker);

    tmp_worker->custom_worker.id        = server->clients.last_id;
    tmp_worker->custom_worker.type      = SOCK_TYPE_REMOTE_CLIENT;
    tmp_worker->custom_worker.mode      = server->worker.custom_worker.mode;
    tmp_worker->custom_worker.port      = server->worker.custom_worker.port;
    tmp_worker->custom_worker.sock      = tmp_client;
    tmp_worker->custom_worker.state     = SOCK_STATE_START;
    strcpy(tmp_worker->custom_worker.host, tmp_host);

    sock_do_work(tmp_worker, 0);
  }
  else
  {
    log_add("sock_server_work, no available workers", LOG_CRITICAL_ERROR);
    server->worker.custom_worker.state = SOCK_STATE_STOP;
    return SOCK_ERROR;
  };
  */
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
  custom_worker_init(&client->custom_client.custom_worker);

  client->custom_client.on_connect    = cmd_connect;
  client->custom_client.on_disconnect = cmd_disconnect;
}
//==============================================================================
int cmd_client_start(cmd_client_t *client, sock_port_t port, sock_host_t host)
{
}
//==============================================================================
int cmd_client_stop(cmd_client_t *client)
{
  client->custom_client.custom_worker.state = SOCK_STATE_STOP;
}
//==============================================================================
int cmd_client_pause(cmd_client_t *client)
{
  client->custom_client.custom_worker.state = SOCK_STATE_PAUSE;
}
//==============================================================================
void *cmd_client_worker(void *arg)
{
  log_add("BEGIN cmd_client_worker", LOG_DEBUG);

  cmd_client_t *tmp_client = (cmd_client_t*)arg;

  do
  {
    custom_client_start(&tmp_client->custom_client.custom_worker);
    custom_client_work (&tmp_client->custom_client);
    custom_worker_stop (&tmp_client->custom_client.custom_worker);
  }
  while(tmp_client->custom_client.custom_worker.state == SOCK_STATE_START);

  log_add("END cmd_client_worker", LOG_DEBUG);
}
//==============================================================================
int cmd_connect(void *sender)
{
  /*
  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(&worker->send_thread,    &tmp_attr, sock_send_worker, (void*)worker);
  pthread_create(&worker->receive_thread, &tmp_attr, sock_recv_worker, (void*)&worker->custom_worker);

  if(wait)
  {
    int status_send;
    pthread_join(worker->send_thread,    (void**)&status_send);
    int status_recv;
    pthread_join(worker->receive_thread, (void**)&status_recv);
  };
  */
}
//==============================================================================
int cmd_disconnect(void *sender)
{
}
//==============================================================================
void *cmd_recv_worker(void *arg)
{
}
//==============================================================================
void *cmd_send_worker(void *arg)
{
}
//==============================================================================
