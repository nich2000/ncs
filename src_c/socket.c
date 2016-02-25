//==============================================================================
//==============================================================================
#include <unistd.h>
#include <pthread.h>

#include "log.h"
#include "protocol_utils.h"
#include "socket.h"
#include "cmdworker.h"
#include "wsworker.h"
#include "webworker.h"
#include "test.h"
//==============================================================================
int sock_init();
int sock_deinit();
//==============================================================================
int sock_worker_init(sock_worker_t *worker);
//==============================================================================
void *sock_server_worker(void *arg);
void *sock_client_worker(void *arg);
//==============================================================================
int sock_server_init (sock_server_t *server);
int sock_server_start(sock_worker_t *worker);
int sock_server_work (sock_server_t *server);
int sock_server_stop (sock_worker_t *worker);
//==============================================================================
int sock_client_init (sock_worker_t *worker);
int sock_client_start(sock_worker_t *worker);
int sock_client_work (sock_worker_t *worker);
int sock_client_stop (sock_worker_t *worker);
//==============================================================================
int sock_do_work(sock_worker_t * worker, int wait);
//==============================================================================
void *sock_recv_worker(void *arg);
void *sock_send_worker(void *arg);
//==============================================================================
int sock_do_send(SOCKET sock, char *buffer, int  size);
//==============================================================================
int sock_route_to_ws (sock_worker_t *worker);
int sock_send_cmd    (sock_worker_t *worker, int argc, ...);
int sock_exec_cmd    (sock_worker_t *worker);
//==============================================================================
//==============================================================================
int sock_version(char *version)
{
  strncpy((char *)version, SOCK_VERSION, SOCK_VERSION_SIZE);

  return SOCK_OK;
}
//==============================================================================
int sock_worker_init(sock_worker_t *worker)
{
  worker->id                 = 0;
  worker->type               = SOCK_TYPE_UNKNOWN;
  worker->mode               = SOCK_MODE_UNKNOWN;
  worker->port               = 0;
  memset(worker->host, 0, SOCK_HOST_SIZE);
  worker->sock               = INVALID_SOCKET;

  worker->is_active          = SOCK_FALSE;

  worker->work_thread        = 0;
  worker->send_thread        = 0;
  worker->receive_thread     = 0;

  worker->exec_cmd           = 0;

  worker->handshake          = 0;
  worker->is_locked          = 0;

  worker->in_massage         = NULL;
  worker->in_message_size    = 0;

  worker->out_message        = NULL;
  worker->out_message_size   = 0;

  pack_protocol_init(&worker->protocol);

//  streamer_init(&worker->streamer, &worker->protocol);
}
//==============================================================================
int sock_exit(sock_worker_t *worker)
{
  worker->is_active = SOCK_FALSE;
}
//==============================================================================
int sock_server(int port, sock_server_t *server, sock_mode_t mode)
{
  char tmp[128];

  log_add("----------", LOG_INFO);
  char tmp_pack_version[PACK_VALUE_SIZE];
  pack_version(tmp_pack_version);
  char tmp_sock_version[SOCK_VERSION_SIZE];
  sock_version(tmp_sock_version);
  sprintf(tmp, "Sock version: %s, Pack version: %s", tmp_sock_version, tmp_pack_version);
  log_add(tmp, LOG_INFO);
  switch(mode)
  {
    case SOCK_MODE_SERVER:
    sprintf(tmp, "Server(SERVER), port: %d", port);
    break;
    case SOCK_MODE_WS_SERVER:
    sprintf(tmp, "Server(WS_SERVER), port: %d", port);
    break;
    case SOCK_MODE_WEB_SERVER:
    sprintf(tmp, "Server(WEB_SERVER), port: %d", port);
    break;
  }
  log_add(tmp, LOG_INFO);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

//  if(server != 0)
//    free(server);
//  server = (sock_server_t*)malloc(sizeof(sock_server_t));

  sock_server_init(server);

  server->worker.id        = 0;
  server->worker.type      = SOCK_TYPE_SERVER;
  server->worker.mode      = mode;
  server->worker.port      = port;
  server->worker.is_active = SOCK_TRUE;

  return pthread_create(&server->worker.work_thread, &tmp_attr, sock_server_worker, (void*)server);
}
//==============================================================================
void *sock_server_worker(void *arg)
{
  log_add("BEGIN sock_server_worker", LOG_DEBUG);

  sock_server_t *tmp_server = (sock_server_t*)arg;

//  char tmp[128];
//  sprintf(tmp, "sock_server_worker, server.addr: %u", tmp_server);
//  log_add(tmp, LOG_INFO);

  sock_init();

  sock_server_start(&tmp_server->worker);
  sock_server_work (tmp_server);
  sock_server_stop (&tmp_server->worker);

  sock_deinit();

  log_add("END sock_server_worker", LOG_DEBUG);
}
//==============================================================================
int sock_client(int port, char *host, sock_worker_t *worker)
{
  char tmp[128];

  log_add("----------", LOG_INFO);
  char tmp_pack_version[PACK_VERSION_SIZE];
  pack_version(tmp_pack_version);
  char tmp_sock_version[SOCK_VERSION_SIZE];
  sock_version(tmp_sock_version);
  sprintf(tmp, "Sock version: %s, Pack version: %s", tmp_sock_version, tmp_pack_version);
  log_add(tmp, LOG_INFO);
  sprintf(tmp, "Client, port: %d, host: %s", port, host);
  log_add(tmp, LOG_INFO);
  log_add("----------", LOG_INFO);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

//  if(worker != 0)
//    free(worker);
//  worker = (sock_worker_t*)malloc(sizeof(sock_worker_t));

  sock_worker_init(worker);

  worker->id         = 0;
  worker->type       = SOCK_TYPE_CLIENT;
  worker->mode       = SOCK_MODE_CLIENT;
  worker->port       = port;
  worker->is_active = SOCK_TRUE;
  strcpy(worker->host, host);

  return pthread_create(&worker->work_thread, &tmp_attr, sock_client_worker, (void*)worker);
}
//==============================================================================
void *sock_client_worker(void *arg)
{
  log_add("BEGIN sock_client_worker", LOG_DEBUG);

  sock_worker_t *tmp_worker = (sock_worker_t*)arg;

  sock_init();

  do
  {
    sock_client_init (tmp_worker);
    sock_client_start(tmp_worker);
    sock_client_work (tmp_worker);
    sock_client_stop (tmp_worker);
  }
  while(tmp_worker->is_active);

  sock_deinit();

  log_add("END sock_client_worker", LOG_DEBUG);
}
//==============================================================================
int sock_get_error()
{
#ifdef __linux__
  return errno;
#elif _WIN32
  return WSAGetLastError();
#else
#endif
}
//==============================================================================
int sock_init()
{
#ifdef __linux__
#elif _WIN32
  WSADATA wsaData;

  if (WSAStartup(WINSOCK_VERSION, &wsaData))
  {
    char *tmp;
    sprintf(tmp, "sock_init, WSAStartup, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);

    WSACleanup();

    return 1;
  }
  else
    log_add("sock_init, WSAStartup OK", LOG_DEBUG);
#else
#endif

  return SOCK_OK;
}
//==============================================================================
int sock_deinit()
{
#ifdef __linux__
#elif _WIN32
  if (WSACleanup())
  {
    char tmp[128];
    sprintf(tmp, "sock_deinit, WSACleanup, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
    log_add("sock_deinit, WSACleanup OK", LOG_DEBUG);
#else
#endif

  return SOCK_OK;
}
//==============================================================================
int sock_server_init(sock_server_t *server)
{
  sock_worker_init(&server->worker);

  server->clients.last_id  = 0;
  server->clients.index    = 0;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    sock_worker_init(&server->clients.items[i]);
}
//==============================================================================
int sock_server_start(sock_worker_t *worker)
{
  log_add("BEGIN sock_server_start", LOG_DEBUG);

  char tmp[128];
  sprintf(tmp, "sock_server_start, Port: %d", worker->port);
  log_add(tmp, LOG_DEBUG);

  worker->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker->sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_server_start, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
    log_add("sock_server_start, socket", LOG_DEBUG);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker->port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(worker->sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_server_start, bind, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 2;
  }
  else
    log_add("sock_server_start, bind", LOG_DEBUG);

  if (listen(worker->sock, SOMAXCONN) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_server, listen, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 3;
  }
  else
    log_add("sock_server_start, listen", LOG_DEBUG);

  log_add("END sock_server_start", LOG_DEBUG);

  return SOCK_OK;
}
//==============================================================================
int sock_find_free_index(sock_server_t *server, int *index)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(!server->clients.items[i].is_active)
    {
      *index = i;
      return SOCK_TRUE;
    }
  return SOCK_FALSE;
}
//==============================================================================
int sock_server_work(sock_server_t *server)
{
  log_add("BEGIN sock_server_work", LOG_DEBUG);
  log_add("Server started", LOG_INFO);
  log_add("----------", LOG_INFO);

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);

//  sprintf(tmp, "sock_server_work, server.addr: %u", server);
//  log_add(tmp, LOG_INFO);

  while(server->worker.is_active)
  {
    SOCKET tmp_client;

    char tmp_host[20];

    log_add("sock_server_work, accepting...", LOG_DEBUG);
    tmp_client = accept(server->worker.sock, (struct sockaddr *)&addr, (int *)&addrlen);
    if(tmp_client == INVALID_SOCKET)
    {
      sprintf(tmp, "sock_server_work, accept, Error: %d", sock_get_error());
      log_add(tmp, LOG_ERROR);
      server->worker.is_active = SOCK_FALSE;
      return SOCK_ERROR;
    }
    else
    {
      struct sockaddr_in sin;
      int len = sizeof(sin);
      getsockname(tmp_client, (struct sockaddr *)&sin, &len);
      strcpy(tmp_host, inet_ntoa(sin.sin_addr));
      sprintf(tmp, "sock_server_work, accepted, socket: %d, ip: %s, port: %d",
              tmp_client, tmp_host, ntohs(sin.sin_port));
      log_add(tmp, LOG_INFO);
    };

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

      tmp_worker->id        = server->clients.last_id;
      tmp_worker->type      = SOCK_TYPE_REMOTE_CLIENT;
      tmp_worker->mode      = server->worker.mode;
      tmp_worker->port      = server->worker.port;
      tmp_worker->sock      = tmp_client;
      tmp_worker->is_active = SOCK_TRUE;
      strcpy(tmp_worker->host, tmp_host);

      sock_do_work(tmp_worker, 0);
    }
    else
    {
      log_add("sock_server_work, no available workers", LOG_CRITICAL_ERROR);
      server->worker.is_active = SOCK_FALSE;
      return SOCK_ERROR;
    };
  };

  log_add("END sock_server_work", LOG_DEBUG);

  return SOCK_OK;
}
//==============================================================================
int sock_server_stop(sock_worker_t *worker)
{
  log_add("sock_server_stop", LOG_DEBUG);
  closesocket(worker->sock);
  return SOCK_OK;
}
//==============================================================================
int sock_client_init(sock_worker_t *worker)
{
  sock_worker_init(worker);
}
//==============================================================================
int sock_client_start(sock_worker_t *worker)
{
  log_add("BEGIN sock_client_start", LOG_INFO);

  char tmp[128];
  sprintf(tmp, "sock_client_start, Port: %d, Host: %s", worker->port, worker->host);
  log_add(tmp, LOG_INFO);

  worker->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker->sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_client_start, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
  {
    sprintf(tmp, "sock_client_start, socket: %d", worker->sock);
    log_add(tmp, LOG_DEBUG);
  };

  log_add("END sock_client_start", LOG_INFO);
}
//==============================================================================
int sock_client_work(sock_worker_t *worker)
{
  log_add("BEGIN sock_client_work", LOG_INFO);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker->port);
  addr.sin_addr.s_addr = inet_addr(worker->host);

  log_add("sock_client_work, connecting...", LOG_INFO);
  while(worker->is_active)
  {
    if(connect(worker->sock, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
    {
      #ifdef SOCK_EXTRA_LOGS
      char tmp[128];
      sprintf(tmp, "sock_client_work, connect, try in %d seconds, Error: %d", SOCK_WAIT_CONNECT, sock_get_error());
      log_add(tmp, LOG_ERROR);
      #endif
      sleep(SOCK_WAIT_CONNECT);
      continue;
    }
    else
      break;
  };
  log_add("sock_client_work, connected", LOG_INFO);

  sock_do_work(worker, 1);

  log_add("END sock_client_work", LOG_INFO);
}
//==============================================================================
int sock_client_stop(sock_worker_t *worker)
{
  log_add("sock_client_stop", LOG_INFO);
  closesocket(worker->sock);
  return SOCK_OK;
}
//==============================================================================
int sock_do_work(sock_worker_t *worker, int wait)
{
  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(&worker->send_thread,    &tmp_attr, sock_send_worker, (void*)worker);
  pthread_create(&worker->receive_thread, &tmp_attr, sock_recv_worker, (void*)worker);

  if(wait)
  {
    int status_send;
    pthread_join(worker->send_thread,    (void**)&status_send);
    int status_recv;
    pthread_join(worker->receive_thread, (void**)&status_recv);
  };
}
//==============================================================================
int sock_route_to_ws(sock_worker_t *worker)
{
  return SOCK_ERROR;
}
//==============================================================================
int sock_send_cmd(sock_worker_t *worker, int argc, ...)
{
  if(worker == NULL)
    return SOCK_ERROR;

  pack_protocol *protocol = &worker->protocol;

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

  return SOCK_OK;
}
//==============================================================================
int sock_exec_cmd(sock_worker_t *worker)
{
  pack_packet *tmp_pack = _pack_pack_current(PACK_IN, &worker->protocol);
  pack_value tmp_cmd;

  if(pack_command(tmp_pack, tmp_cmd) == PACK_OK)
  {
    if(strcmp(tmp_cmd, "hui") == 0)
    {
      log_add("CMD: hui", LOG_INFO);
      sock_send_cmd(worker, 1, "huipizdadjigurda");
    }
    else if(strcmp(tmp_cmd, "stream") == 0)
    {
//      pack_key   tmp_key;
//      pack_value tmp_param;
//      if(pack_param_by_index_as_string(tmp_pack, 1, tmp_key, tmp_param) == PACK_OK)
//      {
//        if(strcmp(tmp_param, "on") == 0)
//          streamer_start(&worker->streamer);
//        else
//          streamer_stop(&worker->streamer);
//      };
    };
  };

  return SOCK_ERROR;
}
//==============================================================================
void *sock_send_worker(void *arg)
{
  sock_worker_t *tmp_worker = (sock_worker_t*)arg;
  SOCKET         tmp_sock = tmp_worker->sock;

  char tmp[1024];
  sprintf(tmp, "BEGIN sock_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  pack_buffer    tmp_buffer;
  pack_size      tmp_size = 0;
  pack_packet   *tmp_pack = 0;
  int            tmp_errors = 0;

  // TODO sock_do_send вызывается из разных мест
  while(tmp_worker->is_active)
  {
    switch(tmp_worker->mode)
    {
      case SOCK_MODE_CLIENT:;
      case SOCK_MODE_SERVER:;
      {
        tmp_pack = _pack_next(&tmp_worker->protocol);
        while(tmp_pack != NULL)
        {
          // TODO Думаю, нужно перенести это куда то под switch(tmp_worker->mode)
          if(pack_packet_to_buffer(tmp_pack, tmp_buffer, &tmp_size) != PACK_OK)
            continue;

          int tmp_cnt = pack_buffer_validate(tmp_buffer, tmp_size, PACK_VALIDATE_ONLY, &tmp_worker->protocol);

          if(tmp_cnt > 0)
          {
//            sprintf(tmp, "sock_send_worker, sock_do_send, sock: %d", tmp_sock);
//            log_add(tmp, LOG_INFO);

//            pack_print(tmp_worker, PACK_OUT, "send", 0, 0, 1, 0);

            if(sock_do_send(tmp_sock, tmp_buffer, (int)tmp_size) == SOCK_ERROR)
              tmp_errors++;

            if((tmp_errors > SOCK_ERRORS_COUNT) || (!tmp_worker->is_active))
              break;
          }
          // TODO двойной вызов этой функции: до и в цикле
          tmp_pack = _pack_next(&tmp_worker->protocol);
        }
        break;
      };
      case SOCK_MODE_WEB_SERVER:
      {
        if(!tmp_worker->is_locked)
          if((tmp_worker->out_message != NULL) && (tmp_worker->out_message_size != 0))
          {
            sprintf(tmp, "sock_send_worker, sock_do_send, size: %d, socket: %d", tmp_worker->out_message_size, tmp_sock);
            log_add(tmp, LOG_INFO);

            sock_do_send(tmp_sock, tmp_worker->out_message, tmp_worker->out_message_size);

            free(tmp_worker->out_message);

            tmp_worker->is_active = SOCK_FALSE;
          }
        break;
      };
      case SOCK_MODE_WS_SERVER:
      {
        if(tmp_worker->handshake)
        {
          if(!tmp_worker->is_locked)
            if((tmp_worker->out_message != NULL) && (tmp_worker->out_message_size != 0))
            {
              if(sock_do_send(tmp_sock, tmp_worker->out_message, tmp_worker->out_message_size) == SOCK_ERROR)
                tmp_errors++;

              free(tmp_worker->out_message);
              tmp_worker->out_message = NULL;
              tmp_worker->out_message_size = 0;

              if((tmp_errors > SOCK_ERRORS_COUNT) || (!tmp_worker->is_active))
                break;
            }
        }
        else
        {
          if(!tmp_worker->is_locked)
            if((tmp_worker->out_message != NULL) && (strlen(tmp_worker->out_message) != 0))
            {
              if(sock_do_send(tmp_sock, tmp_worker->out_message, strlen(tmp_worker->out_message)) == SOCK_ERROR)
                tmp_errors++;

              free(tmp_worker->out_message);
              tmp_worker->out_message = NULL;
              tmp_worker->out_message_size = 0;

              tmp_worker->handshake = 1;

              if((tmp_errors > SOCK_ERRORS_COUNT) || (!tmp_worker->is_active))
                break;
            }
        }
        break;
      };
    };

    if((tmp_errors > SOCK_ERRORS_COUNT) || (!tmp_worker->is_active))
      tmp_worker->is_active = SOCK_FALSE;

    usleep(1000);
  }

  sprintf(tmp, "END sock_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  return NULL;
}
//==============================================================================
void *sock_recv_worker(void *arg)
{
  sock_worker_t *tmp_worker = (sock_worker_t*)arg;
  SOCKET tmp_sock = tmp_worker->sock;

  char tmp[1024];
  sprintf(tmp, "BEGIN sock_recv_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  pack_buffer buffer;
  int         size = 0;
  int         retval = 0;
  int         tmp_errors = 0;

  while(tmp_worker->is_active)
  {
    struct timeval tv;
    tv.tv_sec  = SOCK_WAIT_SELECT;
    tv.tv_usec = 0;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(tmp_sock, &rfds);

    retval = select(1, &rfds, NULL, NULL, &tv);
    if (retval == SOCKET_ERROR)
    {
      sprintf(tmp, "sock_recv_worker, select, socket: %d, Error: %d", tmp_sock, sock_get_error());
      log_add(tmp, LOG_ERROR);
      tmp_errors++;
      if(tmp_errors > SOCK_ERRORS_COUNT)
        break;
      else
        continue;
    }
    else if(!retval)
    {
      #ifdef SOCK_EXTRA_LOGS
      sprintf(tmp, "sock_recv_worker, select, socket: %d, empty for %d seconds", tmp_sock, SOCK_WAIT_SELECT);
      log_add(tmp, LOG_WARNING);
      #endif
      continue;
    }
    else
    {
      size = recv(tmp_sock, buffer, PACK_BUFFER_SIZE, 0);
      if(size == SOCKET_ERROR)
      {
        sprintf(tmp, "sock_recv_worker, recv, socket: %d, Error: %d", tmp_sock, sock_get_error());
        log_add(tmp, LOG_ERROR);
        tmp_errors++;
        if(tmp_errors > SOCK_ERRORS_COUNT)
          break;
        else
          continue;
      }
      else if(!size)
      {
        sprintf(tmp, "sock_recv_worker, recv, socket: %d, socket closed", tmp_sock);
        log_add(tmp, LOG_WARNING);
        break;
      }
      else if(size > PACK_BUFFER_SIZE)
      {
        sprintf(tmp, "sock_recv_worker, recv, socket: %d, buffer too big(%d/%d)", tmp_sock, size, PACK_BUFFER_SIZE);
        log_add(tmp, LOG_CRITICAL_ERROR);
        break;
      }

      #ifdef SOCK_EXTRA_LOGS
      sprintf(tmp, "sock_recv_worker, socket: %d, recv size: %d", tmp_sock, size);
      log_add(tmp, LOG_INFO);
      bytes_to_hex(buffer, (pack_size)size, tmp);
      log_add(tmp, LOG_INFO);
      #endif

      switch(tmp_worker->mode)
      {
        case SOCK_MODE_CLIENT:
        case SOCK_MODE_SERVER:
        {
//          cmd_handle_connection();
          break;
        }
        case SOCK_MODE_WEB_SERVER:
        {
//          tmp_worker->is_locked++;
//          tmp_worker->out_message = (char*)malloc(1024 * 1024);
//          web_handle_connection((char*)buffer, tmp_worker->out_message, &tmp_worker->out_message_size);
//          tmp_worker->is_locked--;
          break;
        }
        case SOCK_MODE_WS_SERVER:
        {
//          tmp_worker->is_locked++;
//          tmp_worker->out_message = (char*)malloc(1024 * 1024);
//          ws_handle_connection((char*)buffer, tmp_worker->out_message, &tmp_worker->out_message_size);
//          tmp_worker->is_locked--;
          break;
        }
      }
    }
  }

  sprintf(tmp, "END sock_recv_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  tmp_worker->is_active = SOCK_FALSE;

  return NULL;
}
//==============================================================================
int sock_do_send(SOCKET sock, char *buffer, int size)
{
  #ifdef SOCK_RANDOM_BUFFER
  int tmp_index = 0;
  int real_buffer_size = rand() % SOCK_BUFFER_SIZE + 1;
  unsigned char tmp_buffer[real_buffer_size];

  while(tmp_index < size)
  {
    int tmp_cnt;
    if((size - tmp_index) > real_buffer_size)
      tmp_cnt = real_buffer_size;
    else
      tmp_cnt = (size - tmp_index);
    memcpy(tmp_buffer, &buffer[tmp_index], tmp_cnt);
    tmp_index += tmp_cnt;

    int res = send(sock, buffer, size, 0);
    if(res == SOCKET_ERROR)
    {
      char tmp[128];
      sprintf(tmp, "sock_do_send, send, Error: %u", sock_get_error());
      log_add(tmp, LOG_ERROR);
    }
    else
    {
      #ifdef SOCK_EXTRA_LOGS
      char tmp[256];
      sprintf(tmp, "sock_do_send, send size: %d", res);
      log_add(tmp, LOG_INFO);
      bytes_to_hex(buffer, (pack_size)size, tmp);
      log_add(tmp, LOG_INFO);
      #endif
    }
  }
  #else
  int res = send(sock, buffer, size, 0);
  if(res == SOCKET_ERROR)
  {
    char tmp[128];
    sprintf(tmp, "sock_do_send, send, Error: %u", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return SOCK_ERROR;
  }
  else
  {
    #ifdef SOCK_EXTRA_LOGS
    char tmp[256];
    sprintf(tmp, "sock_do_send, send size: %d", res);
    log_add(tmp, LOG_INFO);
    bytes_to_hex(buffer, (pack_size)size, tmp);
    log_add(tmp, LOG_INFO);
    #endif
  }
  #endif

  return SOCK_OK;
}
//==============================================================================
int sock_server_send_cmd(sock_server_t *server, int argc, ...)
{
  if(server == NULL)
    return SOCK_ERROR;

  for(int i = 0; i < server->clients.index; i++)
  {
    pack_protocol *protocol = &server->clients.items[i].protocol;

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
  };

  return SOCK_OK;
}
//==============================================================================
int soch_server_exec_cmd(sock_server_t *server, int argc, ...)
{
  return SOCK_OK;
}
//==============================================================================
int sock_client_send_cmd(sock_worker_t *worker, int argc, ...)
{
  if(worker == NULL)
    return SOCK_ERROR;

  pack_protocol *protocol = &worker->protocol;

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

  return SOCK_OK;
}
//==============================================================================
int soch_client_exec_cmd(sock_worker_t *worker, int argc, ...)
{
  return SOCK_OK;
}
//==============================================================================
// http://stackoverflow.com/questions/8125507/how-can-i-send-and-receive-websocket-messages-on-the-server-side
int sock_server_send_to_ws(sock_server_t *server, int argc, ...)
{
  if(server == NULL)
    return SOCK_ERROR;

  va_list params;
  va_start(params, argc);

  char *cmd = va_arg(params, char*);

  for(int i = 0; i < server->clients.index; i++)
  {
    sock_worker_t *tmp_worker = &server->clients.items[i];

    tmp_worker->is_locked++;

    tmp_worker->out_message = (char*)malloc(1024);
    ws_make_frame(TEXT_FRAME, cmd, strlen(cmd), tmp_worker->out_message, 1024);

    tmp_worker->is_locked--;
  };

  va_end(params);

  return SOCK_OK;
}
//==============================================================================
