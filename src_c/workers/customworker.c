//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
// strcpy
#include <string.h>
#include <unistd.h>

#include "customworker.h"

#include "socket.h"
#include "ncs_log.h"
#include "protocol.h"
//==============================================================================
// ID генерится
// 1. для клиента
// 2. для remote клиента, т.е. CMD и WS клиенты будут пересекаться
// начинается с последнего статического ID
int custom_id = STATIC_WS_SERVER_ID + 1;
//==============================================================================
int custom_worker_init(int id, custom_worker_t *worker)
{
  if(id == ID_GEN_NEW)
    worker->id = custom_id++;
  else
    worker->id = id;

  worker->type               = SOCK_TYPE_UNKNOWN;
  worker->mode               = SOCK_MODE_UNKNOWN;
  worker->port               = 0;
  worker->sock               = INVALID_SOCKET;
  worker->state              = STATE_STOP;
  worker->is_locked          = FALSE;

  memset(worker->session_id, 0, PACK_VALUE_SIZE);
  memset(worker->name,       0, PACK_VALUE_SIZE);
  memset(worker->host,       0, SOCK_HOST_SIZE);

  worker->on_state           = NULL;
  worker->on_lock            = NULL;

  sock_name_t tmp;
  sprintf((char*)tmp, "%s_%d", SOCK_NAME_DEFAULT, worker->id);
  strcpy((char*)worker->session_id, (char*)tmp);
  strcpy((char*)worker->name,       (char*)tmp);

  return ERROR_NONE;
}
//==============================================================================
int custom_remote_client_init(int id, custom_remote_client_t *custom_remote_client)
{
  custom_worker_init(id, &custom_remote_client->custom_worker);

  custom_remote_client->send_thread      = 0;
  custom_remote_client->recv_thread      = 0;

  custom_remote_client->active_state     = ACTIVE_NONE;
  custom_remote_client->register_state   = REGISTER_NONE;

  custom_remote_client->connect_time     = 0;
  custom_remote_client->disconnect_time  = 0;
  custom_remote_client->active_time      = 0;
  custom_remote_client->register_time    = 0;

  // TODO: Временное явление 1
  protocol_init(&custom_remote_client->protocol);

  // TODO: Временное явление 2
  custom_remote_client->out_message      = NULL;
  custom_remote_client->out_message_size = 0;
  //TODO: Временное явление 3
  custom_remote_client->hand_shake       = FALSE;
  //TODO: Временное явление 4
  custom_remote_client->report           = 0;

  custom_remote_client->on_error         = NULL;
  custom_remote_client->on_send          = NULL;
  custom_remote_client->on_recv          = NULL;
  custom_remote_client->on_disconnect    = NULL;

  return ERROR_NONE;
}
//==============================================================================
int custom_remote_client_deinit(custom_remote_client_t *custom_remote_client)
{
  return ERROR_NONE;
}
//==============================================================================
int custom_remote_clients_init(custom_remote_clients_list_t *clients_list)
{
  clients_list->index   = 0;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
  {
    custom_remote_client_init(ID_DEFAULT, &clients_list->items[i]);

    protocol_init(&clients_list->items[i].protocol);
  }

  return ERROR_NONE;
}
//==============================================================================
int custom_server_init(int id, custom_server_t *custom_server)
{
  custom_worker_init(id, &custom_server->custom_worker);

  custom_server->work_thread = 0;

  custom_server->on_accept   = NULL;

  return ERROR_NONE;
}
//==============================================================================
int custom_client_init(custom_client_t *custom_client)
{
  custom_remote_client_init(ID_GEN_NEW, &custom_client->custom_remote_client);

  custom_client->work_thread = 0;

  custom_client->on_connect = NULL;

  return ERROR_NONE;
}
//==============================================================================
int custom_worker_start(custom_worker_t *worker)
{
  worker->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker->sock == INVALID_SOCKET)
    return make_last_error_fmt(ERROR_CRITICAL, INVALID_SOCKET,
                               "custom_worker_start, socket: INVALID_SOCKET, worker id: %d, error: %d",
                               worker->id, sock_error());

  int reuse = 1;
  setsockopt(worker->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

  log_add_fmt(LOG_DEBUG, "custom_worker_start, socket: %d, worker id: %d", worker->sock, worker->id);

  return ERROR_NONE;
}
//==============================================================================
int custom_server_start(custom_worker_t *worker)
{
  log_add_fmt(LOG_DEBUG, "custom_server_start, id: %d, port: %d", worker->id, worker->port);

  if(custom_worker_start(worker) >= ERROR_NORMAL)
    return ERROR_CRITICAL;

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker->port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(worker->sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    char tmp[128];
    sprintf(tmp, "custom_server_start, bind, server id: %d, error: %d", worker->id, sock_error());
    log_add(LOG_ERROR_CRITICAL, tmp);
    return make_last_error(ERROR_CRITICAL, SOCKET_ERROR, tmp);
  }
  else
    log_add_fmt(LOG_DEBUG, "custom_server_start, bind, server id: %d", worker->id);

  if (listen(worker->sock, SOMAXCONN) == SOCKET_ERROR)
  {
    char tmp[128];
    sprintf(tmp, "custom_server_start, bind, server id: %d, error: %d", worker->id, sock_error());
    log_add(LOG_ERROR_CRITICAL, tmp);
    return make_last_error(ERROR_CRITICAL, SOCKET_ERROR, tmp);
  }
  else
    log_add_fmt(LOG_DEBUG, "custom_server_start, listen, server id: %d", worker->id);

  return ERROR_NONE;
}
//==============================================================================
int custom_client_start(custom_worker_t *worker)
{
  log_add_fmt(LOG_DEBUG, "custom_client_start, id: %d, port: %d, host: %s",
              worker->id, worker->port, worker->host);

  if(custom_worker_start(worker) >= ERROR_NORMAL)
    return make_last_error(ERROR_CRITICAL, errno, "custom_client_start");

  return ERROR_NONE;
}
//==============================================================================
int custom_worker_stop(custom_worker_t *worker)
{
  log_add_fmt(LOG_DEBUG, "custom_worker_stop, worker: %d", worker->id);

  closesocket(worker->sock);

  return ERROR_NONE;
}
//==============================================================================
int custom_server_work(custom_server_t *server)
{
  log_add_fmt(LOG_DEBUG, "[BEGIN] custom_server_work, server id: %d", server->custom_worker.id);

  server->custom_worker.state = STATE_START;

  log_add_fmt(LOG_INFO, "server started, id: %d, port: %d...",
              server->custom_worker.id, server->custom_worker.port);

  int errors = 0;

  while(server->custom_worker.state == STATE_START)
  {
    SOCKET tmp_client;
    sock_host_t tmp_host;
    sock_port_t tmp_port;
    int res = sock_accept(server->custom_worker.sock, &tmp_client, tmp_host, &tmp_port);
    if(res == ERROR_NONE)
    {
      if(server->on_accept != 0)
      {
        log_add_fmt(LOG_DEBUG,
                    "custom_server_work, accepted, id: %d, socket: %d, host: %s, port: %d",
                    server->custom_worker.id, tmp_client, tmp_host, tmp_port);

        if(server->on_accept((void*)server, tmp_client, tmp_host) >= ERROR_NORMAL)
          log_add_fmt(LOG_ERROR, "custom_server_work, on_accept, id: %d, error: %s",
                      server->custom_worker.id, last_error()->message);
      }
    }
    else if(res >= ERROR_NORMAL)
    {
      log_add_fmt(LOG_ERROR, "custom_server_work, sock_accept, id: %d, error: %s",
                  server->custom_worker.id, last_error()->message);
      if(errors++ > SOCK_ERRORS_COUNT)
        server->custom_worker.state = STATE_STOPPING;
    }

    usleep(1000);
  }

  server->custom_worker.state = STATE_STOP;

  log_add_fmt(LOG_INFO, "server stopped, id: %d, port: %d",
              server->custom_worker.id, server->custom_worker.port);

  log_add_fmt(LOG_DEBUG, "[END] custom_server_work, server id: %d",
              server->custom_worker.id);

  return ERROR_NONE;
}
//==============================================================================
int custom_client_work(custom_client_t *client)
{
  log_add_fmt(LOG_DEBUG, "[BEGIN] custom_client_work, client id: %d",
              client->custom_remote_client.custom_worker.id);

  client->custom_remote_client.custom_worker.state = STATE_START;

  log_add_fmt(LOG_INFO, "custom_client_work, connecting to server, client id: %d...",
          client->custom_remote_client.custom_worker.id);
  while(client->custom_remote_client.custom_worker.state == STATE_START)
  {
    if(sock_connect(client->custom_remote_client.custom_worker.sock,
                    client->custom_remote_client.custom_worker.port,
                    client->custom_remote_client.custom_worker.host) >= ERROR_NORMAL)
    {
      char tmp[256];
      sprintf(tmp, "custom_client_work, sock_connect, client id: %d, try in %d seconds, Error: %d",
              client->custom_remote_client.custom_worker.id, SOCK_WAIT_CONNECT, sock_error());
      make_last_error(ERROR_WARNING, ERROR_WARNING, tmp);
      log_add(LOG_EXTRA, tmp);
      sleep(SOCK_WAIT_CONNECT);
      continue;
    }
    else
    {
      if(client->on_connect != 0)
        client->on_connect((void*)client);
    }

    usleep(1000);
  }

  client->custom_remote_client.custom_worker.state = STATE_STOP;

  log_add_fmt(LOG_DEBUG, "[END] custom_client_work, client id: %d",
              client->custom_remote_client.custom_worker.id);

  return ERROR_NONE;
}
//==============================================================================
void *custom_recv_worker(void *arg)
{
  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  log_add_fmt(LOG_DEBUG, "[BEGIN] custom_recv_worker, worker id: %d, socket: %d",
              tmp_client->custom_worker.id, tmp_sock);

  tmp_client->custom_worker.state = STATE_START;

//  char tmp_report_name[256];
//  sprintf(tmp_report_name, "report_%d", tmp_client->custom_worker.id);
//  gen_log_name(tmp_report_name);
//  tmp_client->report = report_open();

  sock_buffer_t tmp_buffer;
  int           tmp_size = 0;
  int           tmp_errors = 0;

  while(tmp_client->custom_worker.state == STATE_START)
  {
    int res = sock_recv(tmp_sock, (char*)tmp_buffer, &tmp_size);
    if(res == ERROR_NONE)
    {
      if(tmp_size > 0)
        if(tmp_client->on_recv != 0)
          tmp_client->on_recv(tmp_client, (char*)tmp_buffer, tmp_size);
    }
    else if(res == ERROR_WARNING)
    {
      if(tmp_size == 0)
      {
        if(tmp_client->on_disconnect != 0)
          tmp_client->on_disconnect((void*)tmp_client);
        break;
      }
    }
    else if(res >= ERROR_NORMAL)
    {
      if(tmp_client->on_error != 0)
        tmp_client->on_error((void*)tmp_client, last_error());

      if(tmp_errors++ > SOCK_ERRORS_COUNT)
        tmp_client->custom_worker.state = STATE_STOPPING;
    }

    usleep(1000);
  }

  tmp_client->custom_worker.state = STATE_STOP;

  report_close(tmp_client->report);

  log_add_fmt(LOG_DEBUG, "[END] custom_recv_worker, worker id: %d, socket: %d",
              tmp_client->custom_worker.id, tmp_sock);

  return NULL;
}
//==============================================================================
int _custom_remote_clients_count(custom_remote_clients_list_t *clients_list)
{
  int tmp_count = 0;

  if(clients_list != NULL)
    for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
      if(clients_list->items[i].custom_worker.state == STATE_START)
         tmp_count++;

  return tmp_count;
}
//==============================================================================
custom_remote_client_t *_custom_remote_clients_next(custom_remote_clients_list_t *clients_list)
{
  custom_remote_client_t *tmp_client = NULL;

  if(clients_list != NULL)
    for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
      if(clients_list->items[i].custom_worker.state == STATE_STOP)
      {
        tmp_client = &clients_list->items[i];
        break;
      }

  return tmp_client;
}
//==============================================================================
