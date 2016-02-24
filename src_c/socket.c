//==============================================================================
//==============================================================================
#include <unistd.h>
#include <pthread.h>

#include "log.h"
#include "protocol_utils.h"

#include "socket.h"
#include "wsworker.h"
#include "webworker.h"
#include "test.h"
//==============================================================================
#ifdef DEBUG_MODE
static int in_packets_count  = 0;
static int out_packets_count = 0;
#endif
//==============================================================================
int sock_init();
int sock_deinit();
//==============================================================================
int sock_worker_init(sock_worker_t *worker);
//==============================================================================
void *sock_server_worker(void *arg);
//==============================================================================
int sock_server_init (sock_server_t *server);
int sock_server_start(sock_worker_t *worker);
int sock_server_work (sock_server_t *server);
int sock_server_stop (sock_worker_t *worker);
//==============================================================================
void *sock_client_worker(void *arg);
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
int sock_handle_buffer(pack_buffer buffer, pack_size size, sock_worker_t *worker);
//==============================================================================
int sock_stream_print(sock_worker_t *worker, pack_type out, char *prefix, int clear, int buffer, int pack, int csv);
int sock_route_to_ws (sock_worker_t *worker);
int sock_send_cmd    (sock_worker_t *worker, int argc, ...);
int sock_exec_cmd    (sock_worker_t *worker);
//==============================================================================
int sock_version(char *version)
{
  strncpy((char *)version, SOCK_VERSION, SOCK_VERSION_SIZE);

  return SOCK_OK;
}
//==============================================================================
int sock_worker_init(sock_worker_t *worker)
{
  pack_protocol_init(&worker->protocol);

  worker->is_active          = 1;
  worker->worker_kill_flag   = 0;
  worker->sender_kill_flag   = 0;
  worker->receiver_kill_flag = 0;

  worker->send_thread        = 0;
  worker->receive_thread     = 0;

  worker->exec_cmd           = 0;
  worker->handshake          = 0;
  worker->is_locked          = 0;

  worker->in_massage         = NULL;
  worker->in_message_size    = 0;

  worker->out_message        = NULL;
  worker->out_message_size   = 0;
}
//==============================================================================
int sock_exit(sock_worker_t *worker)
{
  worker->worker_kill_flag = 1;
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
  log_add("----------", LOG_INFO);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

//  if(server != 0)
//    free(server);
//  server = (sock_server_t*)malloc(sizeof(sock_server_t));

//  sprintf(tmp, "sock_server, server.addr: %u", server);
//  log_add(tmp, LOG_INFO);

  server->worker.id   = 0;
  server->worker.type = SOCK_TYPE_SERVER;
  server->worker.mode = mode;
  server->worker.port = port;

  memset(server->worker.host, '0', sizeof(server->worker.host));

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

  sock_server_init (tmp_server);
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

  worker->id   = 0;
  worker->type = SOCK_TYPE_CLIENT;
  worker->mode = SOCK_MODE_CLIENT;
  worker->port = port;

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
  while(!tmp_worker->worker_kill_flag);

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
}
//==============================================================================
int sock_server_start(sock_worker_t *worker)
{
  log_add("BEGIN sock_server_start", LOG_DEBUG);

  char tmp[128];
  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_server_start(SERVER_STREAMER), Port: %d", worker->port);
  else
    sprintf(tmp, "sock_server_start(CLIENT_STREAMER), Port: %d", worker->port);
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
int sock_server_work(sock_server_t *server)
{
  log_add("BEGIN sock_server_work", LOG_DEBUG);

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);

//  sprintf(tmp, "sock_server_work, server.addr: %u", server);
//  log_add(tmp, LOG_INFO);

  while(!server->worker.worker_kill_flag)
  {
    SOCKET tmp_client;

    char tmp_host[20];

    log_add("sock_server_work, accept", LOG_DEBUG);
    tmp_client = accept(server->worker.sock, (struct sockaddr *)&addr, (int *)&addrlen);
    if(tmp_client == INVALID_SOCKET)
    {
      sprintf(tmp, "sock_server_work, accept, Error: %d", sock_get_error());
      log_add(tmp, LOG_ERROR);
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

    sock_find_free_index

    if(server->clients.index < SOCK_WORKERS_COUNT)
    {
      sock_worker_t *tmp_worker = &server->clients.items[server->clients.index];

      tmp_worker->id   = server->clients.last_id;
      tmp_worker->type = SOCK_TYPE_REMOTE_CLIENT;
      tmp_worker->mode = server->worker.mode;
      tmp_worker->port = server->worker.port;
      tmp_worker->sock = tmp_client;

      strcpy(tmp_worker->host, tmp_host);

      sock_worker_init(tmp_worker);

      sock_do_work(tmp_worker, 0);

      server->clients.index++;
      server->clients.last_id++;
    }
    else
      return SOCK_ERROR;
  };

  log_add("END sock_server_work", LOG_DEBUG);

  return SOCK_OK;
}
//==============================================================================
int sock_server_stop(sock_worker_t *worker)
{
  log_add("sock_server_stop", LOG_INFO);
  closesocket(worker->sock);
  return SOCK_OK;
}
//==============================================================================
int sock_client_init(sock_worker_t *worker)
{
  sock_worker_init(worker);

  streamer_init(&worker->streamer, &worker->protocol);
}
//==============================================================================
int sock_client_start(sock_worker_t *worker)
{
  log_add("BEGIN sock_client_start", LOG_INFO);

  char tmp[128];

  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_client_start(SERVER_STREAMER), Port: %d, Host: %s", worker->port, worker->host);
  else
    sprintf(tmp, "sock_client_start(CLIENT_STREAMER), Port: %d, Host: %s", worker->port, worker->host);
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

  log_add("sock_client_work, connect", LOG_INFO);
  while(!worker->worker_kill_flag)
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
int sock_stream_print(sock_worker_t *worker, pack_type out, char *prefix, int clear, int buffer, int pack, int csv)
{
  if(clear)
    clr_scr();

  if(!buffer && !pack && !csv)
    return PACK_OK;

  char tmp[1024];

  if(buffer)
  {
    pack_buffer  buffer;
    pack_size    size;

    pack_current_packet_to_buffer(out, buffer, &size, &worker->protocol);

    #ifdef SOCK_PACK_MODE
    bytes_to_hex(buffer, (pack_size)size, tmp);
    log_add(tmp, LOG_DEBUG);
    #else
    add_to_log(buffer, LOG_DEBUG);
    #endif
  };

  if(pack || csv)
  {
    pack_packet *tmp_pack = _pack_pack_current(out, &worker->protocol);

    if(pack)
    {
      sprintf(tmp, "%s\n", prefix);
//      sprintf(tmp, "%sNumber: %d\n", tmp, tmp_pack->number);
      pack_key     key;
      pack_value   valueS;
      pack_size tmp_words_count = _pack_words_count(tmp_pack);
//      sprintf(tmp, "%sWords: %d\n", tmp, tmp_words_count);
      for(pack_size i = 0; i < tmp_words_count; i++)
        if(pack_val_by_index_as_string(tmp_pack, i, key, valueS) == PACK_OK)
          sprintf(tmp, "%s%s: %s\n", tmp, key, valueS);
      log_add(tmp, LOG_INFO);
    };

    if(csv)
    {
      pack_buffer csv;
      pack_values_to_csv(tmp_pack, ';', csv);
      log_add(csv, LOG_DATA);
    };
  };

  return PACK_OK;
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
      pack_key   tmp_key;
      pack_value tmp_param;
      if(pack_param_by_index_as_string(tmp_pack, 1, tmp_key, tmp_param) == PACK_OK)
      {
        if(strcmp(tmp_param, "on") == 0)
          streamer_start(&worker->streamer);
        else
          streamer_stop(&worker->streamer);
      };
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
  while(!tmp_worker->sender_kill_flag)
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

          #ifdef DEBUG_MODE
          if(tmp_cnt != PACK_ERROR)
          {
            out_packets_count += tmp_cnt;
            #ifdef SOCK_EXTRA_LOGS
            char tmp[32];
            sprintf(tmp, "out_packets_count: %d", out_packets_count);
            log_add(tmp, LOG_DEBUG);
            #endif
          };
          #endif

          if(tmp_cnt > 0)
          {
//            sprintf(tmp, "sock_send_worker, sock_do_send, sock: %d", tmp_sock);
//            log_add(tmp, LOG_INFO);

//            sock_stream_print(tmp_worker, PACK_OUT, "send", 0, 0, 1, 0);

            if(sock_do_send(tmp_sock, tmp_buffer, (int)tmp_size) == SOCK_ERROR)
              tmp_errors++;

            if((tmp_errors > SOCK_ERRORS_COUNT) || tmp_worker->sender_kill_flag)
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
//            log_add(tmp_worker->out_message, LOG_DEBUG);

            if(sock_do_send(tmp_sock, tmp_worker->out_message, tmp_worker->out_message_size) == SOCK_ERROR)
              tmp_errors++;

            // TODO утечка
//            free(tmp_worker->out_message);
            tmp_worker->out_message = NULL;
            tmp_worker->out_message_size = 0;

            if((tmp_errors > SOCK_ERRORS_COUNT) || tmp_worker->sender_kill_flag)
              break;
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
//              log_add(tmp_worker->out_message, LOG_DEBUG);

              if(sock_do_send(tmp_sock, tmp_worker->out_message, tmp_worker->out_message_size) == SOCK_ERROR)
                tmp_errors++;

              // TODO утечка
//              free(tmp_worker->out_message);
              tmp_worker->out_message = NULL;
              tmp_worker->out_message_size = 0;

              if((tmp_errors > SOCK_ERRORS_COUNT) || tmp_worker->sender_kill_flag)
                break;
            }
        }
        else
        {
          if(!tmp_worker->is_locked)
            if((tmp_worker->out_message != NULL) && (strlen(tmp_worker->out_message) != 0))
            {
//              log_add(tmp_worker->out_message, LOG_DEBUG);

              if(sock_do_send(tmp_sock, tmp_worker->out_message, strlen(tmp_worker->out_message)) == SOCK_ERROR)
                tmp_errors++;

              // TODO утечка
//              free(tmp_worker->out_message);
              tmp_worker->out_message = NULL;
              tmp_worker->out_message_size = 0;

              tmp_worker->handshake = 1;

              if((tmp_errors > SOCK_ERRORS_COUNT) || tmp_worker->sender_kill_flag)
                break;
            }
        }
        break;
      };
    };

    if((tmp_errors > SOCK_ERRORS_COUNT) || tmp_worker->sender_kill_flag)
      break;

    usleep(1000);
  }

  sprintf(tmp, "END sock_send_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  tmp_worker->sender_kill_flag = 1;
  tmp_worker->receiver_kill_flag = 1;

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

  while(!tmp_worker->receiver_kill_flag)
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

      sock_handle_buffer(buffer, (pack_size)size, tmp_worker);
    };
  }

  sprintf(tmp, "END sock_recv_worker, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  tmp_worker->sender_kill_flag = 1;
  tmp_worker->receiver_kill_flag = 1;

  return NULL;
}
//==============================================================================
int sock_handle_buffer(pack_buffer buffer, pack_size size, sock_worker_t *worker)
{
  switch(worker->mode)
  {
    case SOCK_MODE_CLIENT:
    case SOCK_MODE_SERVER:
    {
      int cnt = pack_buffer_validate(buffer, (pack_size)size, PACK_VALIDATE_ADD, &worker->protocol);

      #ifdef DEBUG_MODE
      if(cnt != PACK_ERROR)
      {
        in_packets_count += cnt;
        #ifdef SOCK_EXTRA_LOGS
        char tmp[32];
        sprintf(tmp, "in_packets_count: %d", in_packets_count);
        log_add(tmp, LOG_DEBUG);
        #endif
      };
      #endif

      if(cnt > 0)
      {
        if(worker->mode == SOCK_MODE_CLIENT)
          sock_stream_print(worker, PACK_IN, "receive", 0, 0, 1, 0);
        sock_exec_cmd(worker);
        sock_route_to_ws(worker);
      };
      break;
    }
    case SOCK_MODE_WEB_SERVER:
    {
      worker->is_locked++;

      worker->out_message = (char*)malloc(1024 * 1024);
      web_handle_buffer((char*)buffer, worker->out_message, &worker->out_message_size);

      worker->is_locked--;

      break;
    }
    case SOCK_MODE_WS_SERVER:
    {
//      log_add(buffer, LOG_INFO);

      if(worker->handshake)
      {
        ws_handle_buffer(buffer);
      }
      else
      {
//        log_add(buffer, LOG_DEBUG);

        char *tmp_message = (char*)malloc(10000);
        ws_hand_shake((char*)buffer, tmp_message);

        worker->is_locked++;

        worker->out_message = (char*)malloc(10000);

        strcpy(worker->out_message, "HTTP/1.1 101 Switching Protocols\r\n");
        strcat(worker->out_message, "Upgrade: websocket\r\n");
        strcat(worker->out_message, "Connection: Upgrade\r\n");
        char tmp[10240];
        sprintf(tmp, "Sec-WebSocket-Accept: %s\r\n\r\n", tmp_message);
        strcat(worker->out_message, tmp);

        worker->out_message[strlen(worker->out_message)+1] = '\0';

        worker->is_locked--;

        // TODO утечка
//        free(tmp_message);

//        log_add(worker->out_message, LOG_DEBUG);
      };
      break;
    }
  }
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
// TODO correct ws frame
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
