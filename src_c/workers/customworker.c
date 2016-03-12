//==============================================================================
//==============================================================================
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "customworker.h"
#include "socket.h"
#include "ncs_log.h"
#include "protocol.h"
//==============================================================================
int custom_worker_init(custom_worker_t *worker)
{
  worker->id                 = 0;
  worker->type               = SOCK_TYPE_UNKNOWN;
  worker->mode               = SOCK_MODE_UNKNOWN;
  worker->port               = 0;
  worker->sock               = INVALID_SOCKET;
  worker->state              = SOCK_STATE_STOP;
  worker->is_locked          = 0;

  memset(worker->host, 0, SOCK_HOST_SIZE);

  return ERROR_NONE;
}
//==============================================================================
int custom_remote_client_init(custom_remote_client_t *custom_remote_client)
{
  custom_worker_init(&custom_remote_client->custom_worker);

  custom_remote_client->send_thread   = 0;
  custom_remote_client->recv_thread   = 0;

  // TODO Временное явление 1
  pack_protocol_init(&custom_remote_client->protocol);
  // TODO Временное явление 2
  custom_remote_client->out_message = 0;
  custom_remote_client->out_message_size = 0;
  //TODO  Временное явление 3
  custom_remote_client->hand_shake = SOCK_FALSE;
  //TODO  Временное явление 4
  custom_remote_client->report = 0;

  custom_remote_client->on_error      = 0;
  custom_remote_client->on_send       = 0;
  custom_remote_client->on_recv       = 0;
  custom_remote_client->on_disconnect = 0;

  return ERROR_NONE;
}
//==============================================================================
int custom_remote_client_deinit(custom_remote_client_t *custom_remote_client)
{
}
//==============================================================================
int custom_remote_clients_list_init(custom_remote_clients_list_t *custom_remote_clients_list)
{
  custom_remote_clients_list->index   = 0;
  custom_remote_clients_list->next_id = 0;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_init(&custom_remote_clients_list->items[i]);

    pack_protocol_init(&custom_remote_clients_list->items[i].protocol);
  };

  return ERROR_NONE;
}
//==============================================================================
int custom_server_init(custom_server_t *custom_server)
{
  custom_worker_init(&custom_server->custom_worker);

  custom_server->work_thread = 0;

  custom_server->on_accept   = 0;

  return ERROR_NONE;
}
//==============================================================================
int custom_client_init(custom_client_t *custom_client)
{
  custom_remote_client_init(&custom_client->custom_remote_client);

  custom_client->work_thread = 0;

  custom_client->on_connect = 0;

  return ERROR_NONE;
}
//==============================================================================
int custom_worker_start(custom_worker_t *worker)
{
  char tmp[128];

  worker->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker->sock == INVALID_SOCKET)
  {
    sprintf(tmp, "custom_worker_start, socket: INVALID_SOCKET, Error: %d", sock_error());
    make_last_error(ERROR_CRITICAL, INVALID_SOCKET, tmp);
    log_add(tmp, LOG_ERROR_CRITICAL);
    return ERROR_CRITICAL;
  }

  int reuse = 1;
  setsockopt(worker->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

  sprintf(tmp, "custom_worker_start, socket: %d", worker->sock);
  log_add(tmp, LOG_DEBUG);

  return ERROR_NONE;
}
//==============================================================================
int custom_server_start(custom_worker_t *worker)
{
  char tmp[128];
  sprintf(tmp, "custom_server_start, Port: %d", worker->port);
  log_add(tmp, LOG_DEBUG);

  if(custom_worker_start(worker) >= ERROR_NORMAL)
    return ERROR_CRITICAL;

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker->port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(worker->sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "custom_server_start, bind, error: %d", sock_error());
    make_last_error(ERROR_CRITICAL, SOCKET_ERROR, tmp);
    log_add(tmp, LOG_ERROR_CRITICAL);
    return ERROR_CRITICAL;
  }
  else
    log_add("custom_server_start, bind", LOG_DEBUG);

  if (listen(worker->sock, SOMAXCONN) == SOCKET_ERROR)
  {
    sprintf(tmp, "custom_server_start, listen, error: %d", sock_error());
    make_last_error(ERROR_CRITICAL, SOCKET_ERROR, tmp);
    log_add(tmp, LOG_ERROR_CRITICAL);
    return ERROR_CRITICAL;
  }
  else
    log_add("custom_server_start, listen", LOG_DEBUG);

  return ERROR_NONE;
}
//==============================================================================
int custom_client_start(custom_worker_t *worker)
{
  char tmp[128];
  sprintf(tmp, "custom_client_start, Port: %d, Host: %s", worker->port, worker->host);
  log_add(tmp, LOG_DEBUG);

  if(custom_worker_start(worker) >= ERROR_NORMAL)
    return ERROR_CRITICAL;

  return ERROR_NONE;
}
//==============================================================================
int custom_worker_stop(custom_worker_t *worker)
{
  log_add("custom_worker_stop", LOG_DEBUG);

  closesocket(worker->sock);

  return ERROR_NONE;
}
//==============================================================================
int custom_server_work(custom_server_t *server)
{
  log_add("BEGIN custom_server_work", LOG_DEBUG);
//  log_add("server started", LOG_INFO);
//  log_add("----------", LOG_INFO);

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);
  int errors = 0;

  while(server->custom_worker.state == SOCK_STATE_START)
  {
    log_add_fmt(LOG_DEBUG, "waiting for connect, port: %d...", server->custom_worker.port);

    SOCKET tmp_client = accept(server->custom_worker.sock, (struct sockaddr *)&addr, (int *)&addrlen);
    if(tmp_client == INVALID_SOCKET)
    {
      sprintf(tmp, "custom_server_work, accept, error: %d", sock_error());
      make_last_error(ERROR_NORMAL, INVALID_SOCKET, tmp);
      log_add(tmp, LOG_ERROR);
      errors++;
      if(errors > SOCK_ERRORS_COUNT)
      {
        server->custom_worker.state == SOCK_STATE_STOP;
        make_last_error(ERROR_CRITICAL, INVALID_SOCKET, tmp);
        log_add(tmp, LOG_ERROR_CRITICAL);
        return ERROR_CRITICAL;
      };
    }
    else
    {
      if(server->on_accept != 0)
      {
        sock_host_t tmp_host;
        struct sockaddr_in tmp_addr;
        int tmp_len = sizeof(tmp_addr);
        getsockname(tmp_client, (struct sockaddr *)&tmp_addr, &tmp_len);
        strcpy(tmp_host, inet_ntoa(tmp_addr.sin_addr));
        int tmp_port = ntohs(tmp_addr.sin_port);

        sprintf(tmp, "custom_server_work, accepted, socket: %d, host: %s, port: %d",
                tmp_client, tmp_host, tmp_port);
        log_add(tmp, LOG_DEBUG);

        if(server->on_accept((void*)server, tmp_client, tmp_host) != ERROR_NONE)
          log_add_fmt(LOG_ERROR, "custom_server_work, Error: %s", last_error()->message);
      }
    }

    usleep(1000);
  }

  log_add("END custom_server_work", LOG_DEBUG);

  return ERROR_NONE;
}
//==============================================================================
int custom_client_work(custom_client_t *client)
{
  log_add("BEGIN custom_client_work", LOG_DEBUG);
  log_add("client started", LOG_INFO);
  log_add("----------", LOG_INFO);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(client->custom_remote_client.custom_worker.port);
  addr.sin_addr.s_addr = inet_addr(client->custom_remote_client.custom_worker.host);

  log_add("connecting to server...", LOG_INFO);
  while(client->custom_remote_client.custom_worker.state == SOCK_STATE_START)
  {
    if(connect(client->custom_remote_client.custom_worker.sock, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
    {
      char tmp[256];
      sprintf(tmp, "custom_client_work, connect, try in %d seconds, Error: %d", SOCK_WAIT_CONNECT, sock_error());
      make_error(ERROR_WARNING, SOCKET_ERROR, tmp);
      log_add(tmp, LOG_EXTRA);
      sleep(SOCK_WAIT_CONNECT);
      continue;
    }
    else
    {
      if(client->on_connect != 0)
      {
        log_add("connected to server", LOG_INFO);

        client->on_connect((void*)client);
      };
    }

    usleep(1000);
  }

  log_add("END custom_client_work", LOG_DEBUG);

  return ERROR_NONE;
}
//==============================================================================
void *custom_recv_worker(void *arg)
{
//  log_add("custom_recv_worker", LOG_INFO);

  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  char tmp[256];
  sprintf(tmp, "BEGIN custom_recv_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  char tmp_report_name[256];
  sprintf(tmp_report_name, "report_%d.txt", tmp_client->custom_worker.id);
  tmp_client->report = report_open(tmp_report_name);

  pack_buffer_t tmp_buffer;
  int           tmp_size = 0;
  int           tmp_errors = 0;

  while(tmp_client->custom_worker.state == SOCK_STATE_START)
  {
    int retval = sock_recv(tmp_sock, tmp_buffer, &tmp_size);
    if(retval == ERROR_NONE)
    {
      if(tmp_size > 0)
        if(tmp_client->on_recv != 0)
          tmp_client->on_recv(tmp_client, tmp_buffer, tmp_size);
    }
    else if(retval == ERROR_WARNING)
    {
      if(tmp_size == 0)
      {
        if(tmp_client->on_disconnect != 0)
          tmp_client->on_disconnect((void*)tmp_client);
        break;
      }
    }
    else if(retval >= ERROR_NORMAL)
    {
      tmp_errors++;
      if(tmp_errors > SOCK_ERRORS_COUNT)
        break;

      if(tmp_client->on_error != 0)
        tmp_client->on_error((void*)tmp_client, last_error());
    }
    else
    {
//      log_add_fmt(LOG_INFO, "custom_recv_worker, message: %s", last_error()->message);
    }

    usleep(1000);
  }

  report_close(tmp_client->report);

  sprintf(tmp, "END custom_recv_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  return NULL;
}
//==============================================================================
