//==============================================================================
//==============================================================================
#include <unistd.h>
#include <pthread.h>

#include "log.h"
#include "protocol_utils.h"

#include "socket.h"
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
int sock_client_init (sock_client_t *client);
int sock_client_start(sock_worker_t *worker);
int sock_client_work (sock_worker_t *worker);
int sock_client_stop (sock_worker_t *worker);
//==============================================================================
int sock_do_work(sock_worker_t * worker, int wait);
//==============================================================================
void *sock_recv_worker(void *arg);
void *sock_send_worker(void *arg);
//==============================================================================
int sock_do_send(SOCKET sock, pack_buffer buffer, int  size);
//==============================================================================
int sock_handle_buffer(pack_buffer buffer, pack_size size, sock_worker_t *worker);
//==============================================================================
int sock_stream_print(sock_worker_t *worker, pack_type out, int clear, int buffer, int pack, int csv);
int sock_route_data  (sock_worker_t *worker);
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
//  id
//  mode
//  port
//  host
//  worker
//  sock

  pack_protocol_init(&worker->protocol);

  worker->worker_kill_flag   =  0;
  worker->sender_kill_flag   =  0;
  worker->receiver_kill_flag =  0;
  worker->send_thread        =  0;
  worker->receive_thread     =  0;
  worker->exec_cmd           =  0;
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
  log_set_name("server_log.txt");

  log_add("----------", LOG_INFO);
  char tmp_pack_version[32];
  pack_version(tmp_pack_version);
  char tmp_sock_version[32];
  sock_version(tmp_sock_version);
  sprintf(tmp, "Sock version: %s, Pack version: %s", tmp_sock_version, tmp_pack_version);
  log_add(tmp, LOG_INFO);
  switch(mode)
  {
    case SOCK_MODE_SERVER:
    sprintf(tmp, "%s", "MODE_SERVER");
    break;
    case SOCK_MODE_WS_SERVER:
    sprintf(tmp, "%s", "WS_SERVER");
    break;
    case SOCK_MODE_WEB_SERVER:
    sprintf(tmp, "%s", "WEB_SERVER");
    break;
  }
  sprintf(tmp, "Server(%s), port: %d", tmp, port);
  log_add(tmp, LOG_INFO);
  log_add("----------", LOG_INFO);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

//  if(server != 0)
//    free(server);
//  server = (sock_server_t*)malloc(sizeof(sock_server_t));

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
  log_add("BEGIN sock_server_worker", LOG_INFO);

  sock_server_t *tmp_server = (sock_server_t*)arg;

  sock_init();

  sock_server_init (tmp_server);
  sock_server_start(&tmp_server->worker);
  sock_server_work (tmp_server);
  sock_server_stop (&tmp_server->worker);

  sock_deinit();

  log_add("END sock_server_worker", LOG_INFO);
}
//==============================================================================
int sock_client(int port, char *host, sock_client_t *client)
{
  char tmp[128];
  log_set_name("client_log.txt");

  log_add("----------", LOG_INFO);
  char tmp_pack_version[32];
  pack_version(tmp_pack_version);
  char tmp_sock_version[32];
  sock_version(tmp_sock_version);
  sprintf(tmp, "Sock version: %s, Pack version: %s", tmp_sock_version, tmp_pack_version);
  log_add(tmp, LOG_INFO);
  sprintf(tmp, "Client, port: %d, host: %s", port, host);
  log_add(tmp, LOG_INFO);
  log_add("----------", LOG_INFO);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

//  if(client != 0)
//    free(client);
//  client = (sock_client_t*)malloc(sizeof(sock_client_t));

  client->worker.id   = 0;
  client->worker.type = SOCK_TYPE_CLIENT;
  client->worker.mode = SOCK_MODE_CLIENT;
  client->worker.port = port;

  strcpy(client->worker.host, host);

  return pthread_create(&client->worker.work_thread, &tmp_attr, sock_client_worker, (void*)client);
}
//==============================================================================
void *sock_client_worker(void *arg)
{
  log_add("BEGIN sock_client_worker", LOG_INFO);

  sock_client_t *tmp_client = (sock_client_t*)arg;

  sock_init();

  do
  {
    sock_client_init (tmp_client);
    sock_client_start(&tmp_client->worker);
    sock_client_work (&tmp_client->worker);
    sock_client_stop (&tmp_client->worker);
  }
  while(!tmp_client->worker.worker_kill_flag);

  sock_deinit();

  log_add("END sock_client_worker", LOG_INFO);
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
  log_add("BEGIN sock_server_start", LOG_INFO);

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

  log_add("END sock_server_start", LOG_INFO);

  return SOCK_OK;
}
//==============================================================================
int sock_server_work(sock_server_t *server)
{
  log_add("BEGIN sock_server_work", LOG_INFO);

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);

  while(!server->worker.worker_kill_flag)
  {
    SOCKET tmp_client;

    log_add("sock_server_work, accept", LOG_INFO);
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
      sprintf(tmp, "sock_server_work, accepted, socket: %d, ip: %s, port: %d",
              tmp_client, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
      log_add(tmp, LOG_INFO);
    };

    if(server->clients.index < SOCK_WORKERS_COUNT)
    {
      sock_worker_t *tmp_worker = &server->clients.items[server->clients.index];

      tmp_worker->id   = server->clients.last_id;
      tmp_worker->type = SOCK_TYPE_REMOTE_CLIENT;
      tmp_worker->mode = server->worker.type;
      tmp_worker->port = server->worker.port;

      strcpy(tmp_worker->host, server->worker.host);

      tmp_worker->sock = tmp_client;

      sock_worker_init(tmp_worker);

      sock_do_work(tmp_worker, 0);

      server->clients.index++;
      if(server->clients.index > SOCK_WORKERS_COUNT)
        server->clients.index++;
      server->clients.last_id++;
    }
    else
      return SOCK_ERROR;
  };

  log_add("END sock_server_work", LOG_INFO);

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
int sock_client_init(sock_client_t *client)
{
  sock_worker_init(&client->worker);

  streamer_init(&client->streamer, &client->worker.protocol);
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

  pthread_create(&worker->send_thread, &tmp_attr, sock_send_worker, (void*)worker);
  pthread_create(&worker->receive_thread, &tmp_attr, sock_recv_worker, (void*)worker);

  if(wait)
  {
    int status;
    pthread_join(worker->send_thread, (void**)&status);
    pthread_join(worker->receive_thread, (void**)&status);
  };
}
//==============================================================================
int sock_stream_prepare()
{
//  #define TEST_SEND_COUNT  1
//  #define TEST_PACK_COUNT  1
//  #define TEST_WORD_COUNT  5
//  #define TEST_STRING_SIZE 5

//  if(((!SOCK_SERVER_STREAMER) && (tmp_worker->mode == SOCK_MODE_SERVER)) ||
//     ((SOCK_SERVER_STREAMER)  && (tmp_worker->mode == SOCK_MODE_CLIENT)))
//    continue;

//  if(counter >= TEST_SEND_COUNT)
//    continue;

//  counter++;

//  for(pack_size i = 0; i < TEST_PACK_COUNT; i++)
//  {
//    pack_begin(&tmp_worker->protocol);
//    pack_add_as_int("SOC", sock, &tmp_worker->protocol);
//    pack_add_as_int("CCC", counter, &tmp_worker->protocol);
//    pack_add_as_int("CN1", counter1++, &tmp_worker->protocol);
//    for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
//    {
//      if(i > 9)
//        sprintf(key, "I%d", i);
//      else
//        sprintf(key, "IN%d", i);

//      pack_add_as_int(key, rand(), &tmp_worker->protocol);
//    };
//    for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
//    {
//      if(i > 9)
//        sprintf(key, "S%d", i);
//      else
//        sprintf(key, "ST%d", i);

//      pack_size j = 0;
//      for(j = 0; j < TEST_STRING_SIZE; j++)
//        valueS[j] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 26];
//      valueS[j] = '\0';

//      pack_add_as_string(key, valueS, &tmp_worker->protocol);
//    };
//    for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
//    {
//      if(i > 9)
//        sprintf(key, "F%d", i);
//      else
//        sprintf(key, "FL%d", i);

//      float rnd = (float)rand()/(float)(RAND_MAX/1000);
//      pack_add_as_float(key, rnd, &tmp_worker->protocol);
//    };
//    pack_end(&tmp_worker->protocol);
//  }

//  sleep(1);
////    usleep(10000000);
}
//==============================================================================
// sock_stream_print(worker, PACK_OUT, 0, 0, 0, 0)
int sock_stream_print(sock_worker_t *worker, pack_type out, int clear, int buffer, int pack, int csv)
{
  if(clear)
    clrscr();

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
      sprintf(tmp, "Number: %d", tmp_pack->number);
      log_add(tmp, LOG_INFO);

      pack_key     key;
      pack_value   valueS;
      pack_size tmp_words_count = _pack_words_count(tmp_pack);
      for(pack_size i = 0; i < tmp_words_count; i++)
      {
        if(pack_val_by_index_as_string(tmp_pack, i, key, valueS) == PACK_OK)
        {
          sprintf(tmp, "%s: %s", key, valueS);
          log_add(tmp, LOG_INFO);
        }
      };
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
int sock_route_data(sock_worker_t *worker)
{
  return SOCK_ERROR;
}
//==============================================================================
int sock_exec_cmd(sock_worker_t *worker)
{
//  pack_packet *tmp_pack = _pack_pack_current(PACK_IN, &worker->protocol);
//  pack_value tmp_cmd;

//  if(pack_command(tmp_pack, tmp_cmd) == PACK_OK)
//  {
//    if(worker->mode == SOCK_MODE_CLIENT)
//    {
//      if(strcmp(tmp_cmd, "stream") == 0)
//      {
//        pack_key   tmp_key;
//        pack_value tmp_param;
//        if(pack_param_by_index_as_string(tmp_pack, 1, tmp_key, tmp_param) == PACK_OK)
//        {
//          if(strcmp(tmp_param, "on") == 0)
//            streamer_start(_streamer);
//          else
//            streamer_stop(_streamer);
//        };
//      };
//    };
//  };

  return SOCK_ERROR;
}
//==============================================================================
void *sock_send_worker(void *arg)
{
  sock_worker_t *tmp_worker = (sock_worker_t*)arg;
  SOCKET sock = tmp_worker->sock;

  char tmp[1024];
  sprintf(tmp, "BEGIN sock_send_worker, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  pack_buffer buffer;
  pack_size   size = 0;

  int tmp_errors = 0;

//  int tmp_counter = 0;

  while(!tmp_worker->sender_kill_flag)
  {
//    if(tmp_worker->mode == SOCK_MODE_CLIENT)
//    {
//      sleep(1);
//      continue;
//    }

    while(pack_queue_next(buffer, &size, &tmp_worker->protocol) != PACK_QUEUE_EMPTY)
    {
//      tmp_counter++;
//      if(tmp_counter > 10)
//      {
//        sleep(1);
//        continue;
//      };

      int cnt = pack_validate(buffer, size, 1, &tmp_worker->protocol);

      #ifdef DEBUG_MODE
      if(cnt != PACK_ERROR)
      {
        out_packets_count += cnt;
        #ifdef SOCK_EXTRA_LOGS
        char tmp[32];
        sprintf(tmp, "out_packets_count: %d", out_packets_count);
        log_add(tmp, LOG_DEBUG);
        #endif
      };
      #endif

      if(cnt > 0)
      {
        sock_stream_print(tmp_worker, PACK_OUT, 0, 0, 1, 0);

        if(sock_do_send(sock, buffer, (int)size) == SOCK_ERROR)
          tmp_errors++;
      }

      if((tmp_errors > SOCK_ERRORS_COUNT) || tmp_worker->sender_kill_flag)
        break;
    }

    if((tmp_errors > SOCK_ERRORS_COUNT) || tmp_worker->sender_kill_flag)
      break;
    else
      continue;
  }

  sprintf(tmp, "END sock_send_worker, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  tmp_worker->sender_kill_flag = 1;
  tmp_worker->receiver_kill_flag = 1;

  return NULL;
}
//==============================================================================
void *sock_recv_worker(void *arg)
{
  sock_worker_t *tmp_worker = (sock_worker_t*)arg;
  SOCKET sock = tmp_worker->sock;

  char tmp[1024];
  sprintf(tmp, "BEGIN sock_recv_worker, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  pack_buffer buffer;
  int         size = 0;

  fd_set rfds;
  struct timeval tv;
  int retval;

  tv.tv_sec  = SOCK_WAIT_SELECT;
  tv.tv_usec = 0;

  int tmp_errors = 0;

  while(!tmp_worker->receiver_kill_flag)
  {
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);

    retval = select(1, &rfds, NULL, NULL, &tv);
    if (retval == SOCKET_ERROR)
    {
      char tmp[128];
      sprintf(tmp, "sock_recv_worker, select, Error: %d", sock_get_error());
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
      sprintf(tmp, "sock_recv_worker, select, empty for %d seconds", SOCK_WAIT_SELECT);
      log_add(tmp, LOG_WARNING);
      #endif
      continue;
    }
    else
    {
      size = recv(sock, buffer, PACK_BUFFER_SIZE, 0);
      if(size == SOCKET_ERROR)
      {
        char tmp[128];
        sprintf(tmp, "sock_recv_worker, recv, Error: %d", sock_get_error());
        log_add(tmp, LOG_ERROR);
        tmp_errors++;
        if(tmp_errors > SOCK_ERRORS_COUNT)
          break;
        else
          continue;
      }
      else if(!size)
      {
        log_add("sock_recv_worker, recv, socket closed", LOG_WARNING);
        break;
      }
      else if(size > PACK_BUFFER_SIZE)
      {
        char tmp[256];
        sprintf(tmp, "sock_recv_worker, recv, buffer to big(%d/%d)", size, PACK_BUFFER_SIZE);
        log_add(tmp, LOG_CRITICAL_ERROR);
        break;
      }

      #ifdef SOCK_EXTRA_LOGS
      sprintf(tmp, "sock_recv_worker, recv size: %d", size);
      log_add(tmp, LOG_INFO);
      bytes_to_hex(buffer, (pack_size)size, tmp);
      log_add(tmp, LOG_INFO);
      #endif

      sock_handle_buffer(buffer, (pack_size)size, tmp_worker);
    };
  }

  sprintf(tmp, "END sock_recv_worker, socket: %d", sock);
  log_add(tmp, LOG_INFO);

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
      int cnt = pack_validate(buffer, (pack_size)size, 0, &worker->protocol);

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
        sock_stream_print(worker, PACK_IN, 1, 0, 1, 0);
        sock_route_data(worker);
        sock_exec_cmd(worker);
      };
      break;
    }
    case SOCK_MODE_WEB_SERVER:
    {
      log_add(buffer, LOG_DEBUG);
      break;
    }
    case SOCK_MODE_WS_SERVER:
    {
      log_add(buffer, LOG_DEBUG);
      break;
    }
  }
}
//==============================================================================
int sock_do_send(SOCKET sock, pack_buffer buffer, int size)
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
int sock_client_send_cmd(sock_client_t *client, int argc, ...)
{
  if(client == NULL)
    return SOCK_ERROR;

  pack_protocol *protocol = &client->worker.protocol;

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
int soch_client_exec_cmd(sock_client_t *client, int argc, ...)
{
  return SOCK_OK;
}
//==============================================================================
