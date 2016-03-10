//==============================================================================
//==============================================================================
#ifdef __linux__
  #include <sys/select.h>
#endif

#include "socket.h"
#include "ncs_log.h"
#include "ncs_error.h"
#include "protocol_types.h"
//==============================================================================
const char *sock_version()
{
  return SOCK_VERSION;
}
//==============================================================================
/*
int sock_worker_init(sock_worker_t *worker)
{
  custom_worker_init(&worker->custom_worker);

  worker->send_thread        = 0;
  worker->receive_thread     = 0;
//  worker->exec_cmd           = 0;
  worker->handshake          = 0;
  worker->in_massage         = NULL;
  worker->in_message_size    = 0;
  worker->out_message        = NULL;
  worker->out_message_size   = 0;

  pack_protocol_init(&worker->protocol);

//  streamer_init(&worker->streamer, &worker->protocol);
}
*/
//==============================================================================
/*
int sock_server(sock_server_t *server, sock_port_t port, sock_mode_t mode)
{
  sock_print_server_header(mode, port);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

//  if(server != 0)
//    free(server);
//  server = (sock_server_t*)malloc(sizeof(sock_server_t));

  sock_server_init(server);

  server->worker.custom_worker.id        = 0;
  server->worker.custom_worker.type      = SOCK_TYPE_SERVER;
  server->worker.custom_worker.mode      = mode;
  server->worker.custom_worker.port      = port;
  server->worker.custom_worker.state     = SOCK_STATE_START;

  return pthread_create(&server->worker.custom_worker.work_thread, &tmp_attr, sock_server_worker, (void*)server);
}
*/
//==============================================================================
/*
void *sock_server_worker(void *arg)
{
  log_add("BEGIN sock_server_worker", LOG_DEBUG);

  sock_server_t *tmp_server = (sock_server_t*)arg;

//  char tmp[128];
//  sprintf(tmp, "sock_server_worker, server.addr: %u", tmp_server);
//  log_add(tmp, LOG_INFO);

  custom_server_start(&tmp_server->worker.custom_worker);
  sock_server_work   (tmp_server);
  custom_worker_stop (&tmp_server->worker.custom_worker);

  log_add("END sock_server_worker", LOG_DEBUG);
}
*/
//==============================================================================
/*
int sock_client(int port, char *host, sock_worker_t *worker)
{
  sock_print_client_header(port, host);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

//  if(worker != 0)
//    free(worker);
//  worker = (sock_worker_t*)malloc(sizeof(sock_worker_t));

  sock_worker_init(worker);

  worker->custom_worker.id         = 0;
  worker->custom_worker.type       = SOCK_TYPE_CLIENT;
  worker->custom_worker.mode       = SOCK_MODE_CMD_CLIENT;
  worker->custom_worker.port       = port;
  worker->custom_worker.state      = SOCK_STATE_START;
  strcpy(worker->custom_worker.host, host);

  return pthread_create(&worker->custom_worker.work_thread, &tmp_attr, sock_client_worker, (void*)worker);
}
*/
//==============================================================================
/*
void *sock_client_worker(void *arg)
{
  log_add("BEGIN sock_client_worker", LOG_DEBUG);

  sock_worker_t *tmp_worker = (sock_worker_t*)arg;

  do
  {
    sock_client_init(tmp_worker);
    custom_client_start(&tmp_worker->custom_worker);
//    sock_client_work(tmp_worker);
    custom_worker_stop(&tmp_worker->custom_worker);
  }
  while(tmp_worker->custom_worker.state == SOCK_STATE_START);

  log_add("END sock_client_worker", LOG_DEBUG);
}
*/
//==============================================================================
int sock_error()
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
    sprintf(tmp, "sock_init, WSAStartup, Error: %d", sock_error());
    log_add(tmp, LOG_ERROR);

    WSACleanup();

    return 1;
  }
  else
    log_add("sock_init, WSAStartup OK", LOG_DEBUG);
#else
#endif

  return ERROR_NONE;
}
//==============================================================================
int sock_deinit()
{
#ifdef __linux__
#elif _WIN32
  if (WSACleanup())
  {
    char tmp[128];
    sprintf(tmp, "sock_deinit, WSACleanup, Error: %d", sock_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
    log_add("sock_deinit, WSACleanup OK", LOG_DEBUG);
#else
#endif

  return ERROR_NONE;
}
//==============================================================================
/*
int sock_server_init(sock_server_t *server)
{
  sock_worker_init(&server->worker);

  server->clients.last_id  = 0;
  server->clients.index    = 0;

  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    sock_worker_init(&server->clients.items[i]);
}
*/
//==============================================================================
/*
int sock_server_start(sock_worker_t *worker)
{
  log_add("BEGIN sock_server_start", LOG_DEBUG);

  char tmp[128];
  sprintf(tmp, "sock_server_start, Port: %d", worker->custom.port);
  log_add(tmp, LOG_DEBUG);

  worker->custom.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker->custom.sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_server_start, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
    log_add("sock_server_start, socket", LOG_DEBUG);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker->custom.port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(worker->custom.sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_server_start, bind, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 2;
  }
  else
    log_add("sock_server_start, bind", LOG_DEBUG);

  if (listen(worker->custom.sock, SOMAXCONN) == SOCKET_ERROR)
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
*/
//==============================================================================
/*
int sock_find_free_index(sock_server_t *server, int *index)
{
  for(int i = 0; i < SOCK_WORKERS_COUNT; i++)
    if(server->clients.items[i].custom_worker.state == SOCK_STATE_STOP)
    {
      *index = i;
      return SOCK_TRUE;
    }
  return SOCK_FALSE;
}
*/
//==============================================================================
/*
int sock_server_work(sock_server_t *server)
{
  log_add("BEGIN sock_server_work", LOG_DEBUG);
  log_add("Server started", LOG_INFO);
  log_add("----------", LOG_INFO);

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);

  while(server->worker.custom_worker.state == SOCK_STATE_START)
  {
    SOCKET tmp_client;

    char tmp_host[20];

    log_add("sock_server_work, accepting...", LOG_DEBUG);
    tmp_client = accept(server->worker.custom_worker.sock, (struct sockaddr *)&addr, (int *)&addrlen);
    if(tmp_client == INVALID_SOCKET)
    {
      sprintf(tmp, "sock_server_work, accept, error: %d", sock_get_error());
      log_add(tmp, LOG_ERROR);
      server->worker.custom_worker.state == SOCK_STATE_STOP;
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
  };

  log_add("END sock_server_work", LOG_DEBUG);

  return SOCK_OK;
}
*/
//==============================================================================
/*
int sock_server_stop(sock_worker_t *worker)
{
  log_add("sock_server_stop", LOG_DEBUG);
  closesocket(worker->custom.sock);
  return SOCK_OK;
}
*/
//==============================================================================
/*
int sock_client_init(sock_worker_t *worker)
{
  sock_worker_init(worker);
}
*/
//==============================================================================
/*
int sock_client_start(sock_worker_t *worker)
{
  log_add("BEGIN sock_client_start", LOG_INFO);

  char tmp[128];
  sprintf(tmp, "sock_client_start, Port: %d, Host: %s", worker->custom.port, worker->custom.host);
  log_add(tmp, LOG_INFO);

  worker->custom.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker->custom.sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_client_start, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
  {
    sprintf(tmp, "sock_client_start, socket: %d", worker->custom.sock);
    log_add(tmp, LOG_DEBUG);
  };

  log_add("END sock_client_start", LOG_INFO);
}
*/
//==============================================================================
/*
int sock_client_work(sock_worker_t *worker)
{
  log_add("BEGIN sock_client_work", LOG_INFO);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker->custom_worker.port);
  addr.sin_addr.s_addr = inet_addr(worker->custom_worker.host);

  log_add("sock_client_work, connecting...", LOG_INFO);
  while(worker->custom_worker.state == SOCK_STATE_START)
  {
    if(connect(worker->custom_worker.sock, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
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
*/
//==============================================================================
/*
int sock_client_stop(sock_worker_t *worker)
{
  log_add("sock_client_stop", LOG_INFO);
  closesocket(worker->custom.sock);
  return SOCK_OK;
}
*/
//==============================================================================
/*
int sock_do_work(sock_worker_t *worker, int wait)
{
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
}
*/
//==============================================================================
/*
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
*/
//==============================================================================
/*
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
*/
//==============================================================================
/*
void *sock_send_worker(void *arg)
{
  sock_worker_t *tmp_worker = (sock_worker_t*)arg;
  SOCKET         tmp_sock = tmp_worker->custom_worker.sock;

  char tmp[1024];
  sprintf(tmp, "BEGIN sock_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  pack_buffer    tmp_buffer;
  pack_size      tmp_size = 0;
  pack_packet   *tmp_pack = 0;
  int            tmp_errors = 0;

  // TODO sock_send вызывается из разных мест
  while(tmp_worker->custom_worker.state == SOCK_STATE_START)
  {
    switch(tmp_worker->custom_worker.mode)
    {
      case SOCK_MODE_CMD_CLIENT:;
      case SOCK_MODE_CMD_SERVER:;
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
//            sprintf(tmp, "sock_send_worker, sock_send, sock: %d", tmp_sock);
//            log_add(tmp, LOG_INFO);

//            pack_print(tmp_worker, PACK_OUT, "send", 0, 0, 1, 0);

            if(sock_send(tmp_sock, tmp_buffer, (int)tmp_size) == SOCK_ERROR)
              tmp_errors++;

            if((tmp_errors > SOCK_ERRORS_COUNT) || (!tmp_worker->custom_worker.state == SOCK_STATE_START))
              break;
          }
          // TODO двойной вызов этой функции: до и в цикле
          tmp_pack = _pack_next(&tmp_worker->protocol);
        }
        break;
      };
      case SOCK_MODE_WEB_SERVER:
      {
        if(!tmp_worker->custom_worker.is_locked)
          if((tmp_worker->out_message != NULL) && (tmp_worker->out_message_size != 0))
          {
            sprintf(tmp, "sock_send_worker, sock_send, size: %d, socket: %d", tmp_worker->out_message_size, tmp_sock);
            log_add(tmp, LOG_INFO);

            sock_send(tmp_sock, tmp_worker->out_message, tmp_worker->out_message_size);

            free(tmp_worker->out_message);

            tmp_worker->custom_worker.state = SOCK_STATE_STOP;
          }
        break;
      };
      case SOCK_MODE_WS_SERVER:
      {
        if(tmp_worker->handshake)
        {
          if(!tmp_worker->custom_worker.is_locked)
            if((tmp_worker->out_message != NULL) && (tmp_worker->out_message_size != 0))
            {
              if(sock_send(tmp_sock, tmp_worker->out_message, tmp_worker->out_message_size) == SOCK_ERROR)
                tmp_errors++;

              free(tmp_worker->out_message);
              tmp_worker->out_message = NULL;
              tmp_worker->out_message_size = 0;

              if((tmp_errors > SOCK_ERRORS_COUNT) || (tmp_worker->custom_worker.state == SOCK_STATE_STOP))
                break;
            }
        }
        else
        {
          if(!tmp_worker->custom_worker.is_locked)
            if((tmp_worker->out_message != NULL) && (strlen(tmp_worker->out_message) != 0))
            {
              if(sock_send(tmp_sock, tmp_worker->out_message, strlen(tmp_worker->out_message)) == SOCK_ERROR)
                tmp_errors++;

              free(tmp_worker->out_message);
              tmp_worker->out_message = NULL;
              tmp_worker->out_message_size = 0;

              tmp_worker->handshake = 1;

              if((tmp_errors > SOCK_ERRORS_COUNT) || (tmp_worker->custom_worker.state == SOCK_STATE_STOP))
                break;
            }
        }
        break;
      };
    };

    if((tmp_errors > SOCK_ERRORS_COUNT) || (tmp_worker->custom_worker.state == SOCK_STATE_STOP))
      tmp_worker->custom_worker.state = SOCK_STATE_STOP;

    usleep(1000);
  }

  sprintf(tmp, "END sock_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  return NULL;
}
*/
//==============================================================================
/*
void *sock_recv_worker(void *arg)
{
  custom_worker_t *tmp_worker = (custom_worker_t*)arg;
  SOCKET tmp_sock = tmp_worker->sock;

  char tmp[1024];
  sprintf(tmp, "BEGIN sock_recv_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  pack_buffer tmp_buffer;
  int         tmp_size;

  while(tmp_worker->state == SOCK_STATE_START)
  {
    if(sock_recv(tmp_sock, tmp_buffer, &tmp_size))
    {
//      tmp_worker->on_recv();

      switch(tmp_worker->mode)
      {
        case SOCK_MODE_CMD_CLIENT:
        case SOCK_MODE_CMD_SERVER:
        {
//          if(tmp_size > PACK_BUFFER_SIZE)
//          {
//            sprintf(tmp, "sock_recv_worker, recv, socket: %d, buffer too big(%d/%d)", tmp_sock, tmp_size, PACK_BUFFER_SIZE);
//            log_add(tmp, LOG_CRITICAL_ERROR);
//            break;
//          }
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

  tmp_worker->state = SOCK_STATE_STOP;

  return NULL;
}
*/
//==============================================================================
int sock_recv(SOCKET sock, char *buffer, int *size)
{
  char tmp[1024];

  struct timeval tv;
  fd_set rfds;

  FD_ZERO(&rfds);
  FD_SET(sock, &rfds);

  tv.tv_sec  = SOCK_WAIT_SELECT;
  tv.tv_usec = 0;

  int retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
  if (retval == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_recv, select, socket: %d, error: %d", sock, sock_error());
    make_error(ERROR_NORMAL, retval, tmp);
    log_add(tmp, LOG_ERROR);
    return ERROR_NORMAL;
  }
  else if(!retval)
  {
    sprintf(tmp, "sock_recv, select, socket: %d, empty for %d seconds", sock, SOCK_WAIT_SELECT);
    make_error(ERROR_WARNING, retval, tmp);
    log_add(tmp, LOG_EXTRA);
    return ERROR_WARNING;
  }
  else
  {
    *size = recv(sock, buffer, PACK_BUFFER_SIZE, 0);
    if(*size == SOCKET_ERROR)
    {
      sprintf(tmp, "sock_recv, recv, socket: %d, error: %d", sock, sock_error());
      make_error(ERROR_NORMAL, *size, tmp);
      log_add(tmp, LOG_ERROR);
      return ERROR_NORMAL;
    }
    else if(*size == 0)
    {
      sprintf(tmp, "sock_recv, recv, socket: %d, socket closed", sock);
      make_error(ERROR_WARNING, *size, tmp);
      log_add(tmp, LOG_WARNING);
      return ERROR_WARNING;
    }

    #ifdef USE_EXTRA_LOGS
    log_add_fmt(LOG_EXTRA, "sock_recv, socket: %d, recv size: %d", sock, size);
    log_add_fmt(LOG_EXTRA, "sock_recv, buffer:\n%s", buffer);
    bytes_to_hex(buffer, (pack_size_t)size, tmp);
    log_add_fmt(LOG_EXTRA, "sock_recv, hex buffer:\n%s", buffer);
    #endif

    return ERROR_NONE;
  }
}
//==============================================================================
int sock_send(SOCKET sock, char *buffer, int size)
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
      sprintf(tmp, "sock_send, send, Error: %u", sock_get_error());
      log_add(tmp, LOG_ERROR);
    }
    else
    {
      #ifdef SOCK_EXTRA_LOGS
      log_add_fmt(LOG_EXTRA, "sock_send, socket: %d, send size: %d", sock, size);
      log_add_fmt(LOG_EXTRA, "sock_send, buffer:\n%s", buffer);
      bytes_to_hex(buffer, (pack_size_t)size, tmp);
      log_add_fmt(LOG_EXTRA, "sock_send, hex buffer:\n%s", buffer);
      #endif
    }
  }
  #else
  int res = send(sock, buffer, size, 0);
  if(res == SOCKET_ERROR)
  {
    char tmp[128];
    sprintf(tmp, "sock_send, send, Error: %u", sock_error());
    log_add(tmp, LOG_ERROR);
    return ERROR_NORMAL;
  }
  else
  {
    #ifdef SOCK_EXTRA_LOGS
    log_add_fmt(LOG_EXTRA, "sock_send, socket: %d, send size: %d", sock, size);
    log_add_fmt(LOG_EXTRA, "sock_send, buffer:\n%s", buffer);
    bytes_to_hex(buffer, (pack_size_t)size, tmp);
    log_add_fmt(LOG_EXTRA, "sock_send, hex buffer:\n%s", buffer);
    #endif
  }
  #endif

  return ERROR_NONE;
}
//==============================================================================
/*
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
*/
//==============================================================================
/*
int soch_server_exec_cmd(sock_server_t *server, int argc, ...)
{
  return SOCK_OK;
}
*/
//==============================================================================
/*
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
*/
//==============================================================================
/*
int soch_client_exec_cmd(sock_worker_t *worker, int argc, ...)
{
  return SOCK_OK;
}
*/
//==============================================================================
// http://stackoverflow.com/questions/8125507/how-can-i-send-and-receive-websocket-messages-on-the-server-side
/*
int sock_server_send_to_ws(sock_server_t *server, int argc, ...)
{
//  if(server == NULL)
//    return SOCK_ERROR;

//  va_list params;
//  va_start(params, argc);

//  char *cmd = va_arg(params, char*);

//  for(int i = 0; i < server->clients.index; i++)
//  {
//    sock_worker_t *tmp_worker = &server->clients.items[i];

//    tmp_worker->custom_worker.is_locked++;

//    tmp_worker->out_message = (char*)malloc(1024);
//    ws_make_frame(TEXT_FRAME, cmd, strlen(cmd), tmp_worker->out_message, 1024);

//    tmp_worker->custom_worker.is_locked--;
//  };

//  va_end(params);

  return SOCK_OK;
}
*/
//==============================================================================
