//==============================================================================
//==============================================================================
#include "cmdworker.h"
#include "socket_utils.h"
#include "utils.h"
#include "log.h"
//==============================================================================
int cmd_server_init (cmd_worker_t *worker);
int cmd_server_start(cmd_worker_t *worker, sock_port_t port);
int cmd_server_work (cmd_worker_t *worker);
int cmd_server_stop (cmd_worker_t *worker);
int cmd_server_pause(cmd_worker_t *worker);
//==============================================================================
int cmd_accept(SOCKET socket);
void *cmd_server_worker(void *arg);
void *cmd_recv_worker(void *arg);
void *cmd_send_worker(void *arg);
//==============================================================================
int          _cmd_server_id = 0;
cmd_worker_t _cmd_server;
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
  print_custom_worker_info(&_cmd_server.custom_server.custom_worker, "cmd_server");
}
//==============================================================================
int cmd_server_init (cmd_worker_t *worker)
{
  custom_worker_init(&worker->custom_server.custom_worker);

  worker->custom_server.custom_worker.id   = _cmd_server_id++;
  worker->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  worker->custom_server.custom_worker.mode = SOCK_MODE_CMD_SERVER;
  worker->custom_server.on_accept          = &cmd_accept;
}
//==============================================================================
int cmd_server_start(cmd_worker_t *worker, sock_port_t port)
{
  cmd_server_init(worker);

  worker->custom_server.custom_worker.port  = port;
  worker->custom_server.custom_worker.state = SOCK_STATE_START;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  return pthread_create(&worker->custom_server.custom_worker.work_thread, &tmp_attr, cmd_server_worker, (void*)worker);
}
//==============================================================================
int cmd_server_work (cmd_worker_t *worker)
{

}
//==============================================================================
int cmd_server_stop (cmd_worker_t *worker)
{
  worker->custom_server.custom_worker.state = SOCK_STATE_STOP;
}
//==============================================================================
int cmd_server_pause(cmd_worker_t *worker)
{
  worker->custom_server.custom_worker.state = SOCK_STATE_PAUSE;
}
//==============================================================================
int cmd_accept(SOCKET socket)
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
}
//==============================================================================
void *cmd_server_worker(void *arg)
{
  log_add("BEGIN cmd_server_worker", LOG_DEBUG);

  cmd_worker_t *tmp_server = (cmd_worker_t*)arg;

  custom_server_start(&tmp_server->custom_server.custom_worker);
  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  log_add("END cmd_server_worker", LOG_DEBUG);
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
