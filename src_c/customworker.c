//==============================================================================
//==============================================================================
#include <stdio.h>

#include "customworker.h"
#include "socket.h"
#include "log.h"
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
  worker->work_thread        = 0;

  memset(worker->host, 0, SOCK_HOST_SIZE);
}
//==============================================================================
int custom_worker_start(custom_worker_t *worker)
{
  char tmp[128];

  worker->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker->sock == INVALID_SOCKET)
  {
    sprintf(tmp, "custom_worker_start, socket: INVALID_SOCKET, Error: %d", sock_error());
    log_add(tmp, LOG_CRITICAL_ERROR);
    return SOCK_ERROR;
  }
  else
  {
    sprintf(tmp, "custom_worker_start, socket: %d", worker->sock);
    log_add(tmp, LOG_DEBUG);
  };

  return SOCK_OK;
}
//==============================================================================
int custom_server_start(custom_worker_t *worker)
{
  log_add("BEGIN custom_server_start", LOG_DEBUG);

  char tmp[128];
  sprintf(tmp, "custom_server_start, Port: %d", worker->port);
  log_add(tmp, LOG_DEBUG);

  if(custom_worker_start(worker) == SOCK_ERROR)
    return SOCK_ERROR;

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker->port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(worker->sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "custom_server_start, bind, Error: %d", sock_error());
    log_add(tmp, LOG_CRITICAL_ERROR);
    return SOCK_ERROR;;
  }
  else
    log_add("custom_server_start, bind", LOG_DEBUG);

  if (listen(worker->sock, SOMAXCONN) == SOCKET_ERROR)
  {
    sprintf(tmp, "custom_server_start, listen, Error: %d", sock_error());
    log_add(tmp, LOG_CRITICAL_ERROR);
    return SOCK_ERROR;;
  }
  else
    log_add("custom_server_start, listen", LOG_DEBUG);

  log_add("END custom_server_start", LOG_DEBUG);

  return SOCK_OK;
}
//==============================================================================
int custom_client_start(custom_worker_t *worker)
{
  log_add("BEGIN custom_client_start", LOG_INFO);

  char tmp[128];
  sprintf(tmp, "custom_client_start, Port: %d, Host: %s", worker->port, worker->host);
  log_add(tmp, LOG_INFO);

  if(custom_worker_start(worker) == SOCK_ERROR)
    return SOCK_ERROR;

  log_add("END custom_client_start", LOG_INFO);

  return SOCK_OK;
}
//==============================================================================
int custom_worker_stop(custom_worker_t *worker)
{
  log_add("custom_worker_stop", LOG_DEBUG);

  closesocket(worker->sock);

  return SOCK_OK;
}
//==============================================================================
int custom_server_work(custom_server_t *server)
{
  log_add("BEGIN custom_server_work", LOG_DEBUG);
  log_add("Server started", LOG_INFO);
  log_add("----------", LOG_INFO);

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);

  while(server->custom_worker.state == SOCK_STATE_START)
  {
    log_add("custom_server_work, accepting...", LOG_DEBUG);

    SOCKET tmp_client = accept(server->custom_worker.sock, (struct sockaddr *)&addr, (int *)&addrlen);
    if(tmp_client == INVALID_SOCKET)
    {
      sprintf(tmp, "custom_server_work, accept, Error: %d", sock_error());
      log_add(tmp, LOG_ERROR);
      server->custom_worker.state == SOCK_STATE_STOP;
      return SOCK_ERROR;
    }
    else
    {
      char tmp_host[20];
      struct sockaddr_in sin;
      int len = sizeof(sin);
      getsockname(tmp_client, (struct sockaddr *)&sin, &len);
      strcpy(tmp_host, inet_ntoa(sin.sin_addr));
      sprintf(tmp, "custom_server_work, accepted, socket: %d, ip: %s, port: %d",
              tmp_client, tmp_host, ntohs(sin.sin_port));
      log_add(tmp, LOG_DEBUG);
    };

    server->on_accept((void*)server, tmp_client);
  };

  log_add("END custom_server_work", LOG_DEBUG);

  return SOCK_OK;
}
//==============================================================================
int custom_client_work(custom_client_t *client)
{
  log_add("BEGIN custom_client_work", LOG_INFO);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(client->custom_worker.port);
  addr.sin_addr.s_addr = inet_addr(client->custom_worker.host);

  log_add("custom_client_work, connecting...", LOG_INFO);
  while(client->custom_worker.state == SOCK_STATE_START)
  {
    if(connect(client->custom_worker.sock, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
    {
      #ifdef SOCK_EXTRA_LOGS
      char tmp[128];
      sprintf(tmp, "custom_client_work, connect, try in %d seconds, Error: %d", SOCK_WAIT_CONNECT, sock_get_error());
      log_add(tmp, LOG_ERROR);
      #endif
      sleep(SOCK_WAIT_CONNECT);
      continue;
    }
    else
    {
      log_add("custom_client_work, connected", LOG_INFO);

      client->on_connect((void*)client);
    }
  }

  log_add("END custom_client_work", LOG_INFO);
}
//==============================================================================
