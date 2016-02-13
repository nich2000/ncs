//==============================================================================
//==============================================================================
#include <unistd.h>
#include <pthread.h>
//#include <malloc.h>

#include "log.h"
#include "protocol_utils.h"

#include "socket.h"
#include "test.h"
//==============================================================================
sock_worker      worker;
sock_worker_list clients; // MODE_SERVER
//==============================================================================
int sock_init();
int sock_deinit();
//==============================================================================
void *sock_server_worker(void *arg);
//==============================================================================
int sock_server_init();
int sock_server_start();
int sock_server_work();
int sock_server_stop();
//==============================================================================
void *sock_client_worker(void *arg);
//==============================================================================
int sock_client_init();
int sock_client_start();
int sock_client_work();
int sock_client_stop();
//==============================================================================
void *sock_recv_worker(void *arg);
void *sock_send_worker(void *arg);
//==============================================================================
int sock_do_send(SOCKET sock, pack_buffer buffer, int  size);
//==============================================================================
//==============================================================================
int sock_version(char *version)
{
  strncpy((char *)version, SOCK_VERSION, SOCK_VERSION_SIZE);

  return SOCK_OK;
}
//==============================================================================
void sock_print_all_data(sock_worker *worker)
{
//pack_packet *tmp_pack = _pack_pack_current(PACK_IN, &tmp_worker->protocol);
}
//==============================================================================
int sock_server(int port)
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

  sprintf(tmp, "Server mode, port: %d", port);
  log_add(tmp, LOG_INFO);

  log_add("----------", LOG_INFO);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  worker.port = port;
  memset(worker.host,'0', sizeof(worker.host));

  return pthread_create(&worker.worker, &tmp_attr, sock_server_worker, (void*)&worker);
}
//==============================================================================
void *sock_server_worker(void *arg)
{
  log_add("sock_server_worker started", LOG_DEBUG);

  sock_init();
  sock_server_init();
  sock_server_start();
  sock_server_work();
  sock_server_stop();
  sock_deinit();

  log_add("sock_server_worker finished", LOG_DEBUG);
}
//==============================================================================
int sock_client(int port, char *host)
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

  sprintf(tmp, "Client mode, port: %d, host: %s", port, host);
  log_add(tmp, LOG_INFO);

  log_add("----------", LOG_INFO);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  worker.port = port;
  strcpy(worker.host, host);

  return pthread_create(&worker.worker, &tmp_attr, sock_client_worker, (void*)&worker);
}
//==============================================================================
void *sock_client_worker(void *arg)
{
  log_add("sock_client_worker started", LOG_DEBUG);

  sock_init();
  sock_client_init();
  sock_client_start();
  sock_client_work();
  sock_client_stop();
  sock_deinit();

  log_add("sock_client_worker finished", LOG_DEBUG);
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

  return 0;
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

  return 0;
}
//==============================================================================
int sock_server_init()
{
  worker.id       = 0;
  worker.mode     = SOCK_MODE_SERVER;
  worker.sock     = INVALID_SOCKET;
  worker.sender   = 0;
  worker.receiver = 0;

  clients.last_id  = 0;
  clients.index    = 0;
}
//==============================================================================
int sock_server_start()
{
  char tmp[128];
  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_server_start(SERVER_STREAMER), Port: %d", worker.port);
  else
    sprintf(tmp, "sock_server_start(CLIENT_STREAMER), Port: %d", worker.port);
  log_add(tmp, LOG_DEBUG);

  worker.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker.sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_server_start, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
    log_add("sock_server_start, Create socket", LOG_DEBUG);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker.port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(worker.sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_server_start, bind, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 2;
  }
  else
    log_add("sock_server_start, Bind socket", LOG_DEBUG);

  if (listen(worker.sock, SOMAXCONN) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_server, listen, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 3;
  }
  else
    log_add("sock_server_start, Listen socket", LOG_DEBUG);

  return 0;
}
//==============================================================================
int sock_server_work()
{
  log_add("sock_server_work", LOG_INFO);
  log_add("----------", LOG_INFO);

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);

  while(1)
  {
    SOCKET tmp_client;

    tmp_client = accept(worker.sock, (struct sockaddr *)&addr, (int *)&addrlen);
    if(tmp_client == INVALID_SOCKET)
    {
      sprintf(tmp, "sock_server_work, accept, Error: %d", sock_get_error());
      log_add(tmp, LOG_ERROR);
      return 1;
    }
    else
    {
      struct sockaddr_in sin;
      int len = sizeof(sin);
      getsockname(tmp_client, (struct sockaddr *)&sin, &len);
      sprintf(tmp, "sock_server_work, accept, socket: %d, ip: %s, port: %d",
              tmp_client, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
      log_add(tmp, LOG_DEBUG);
    };

    if(clients.index < SOCK_WORKERS_COUNT)
    {
      pthread_attr_t tmp_attr;
      pthread_attr_init(&tmp_attr);
      pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

      sock_worker *tmp_worker = &clients.items[clients.index];

      pack_init(&tmp_worker->protocol);
      tmp_worker->sock = tmp_client;
      tmp_worker->id = clients.last_id;

      pthread_create(&tmp_worker->sender,   &tmp_attr, sock_send_worker, (void*)tmp_worker);
      pthread_create(&tmp_worker->receiver, &tmp_attr, sock_recv_worker, (void*)tmp_worker);

      clients.index++;
      clients.last_id++;
    }
    else
      return 3;
  };

  return 0;
}
//==============================================================================
int sock_server_stop()
{
  log_add("sock_server_stop", LOG_INFO);
  closesocket(worker.sock);
  return 0;
}
//==============================================================================
int sock_client_init()
{
  worker.id       = 0;
  worker.mode     = SOCK_MODE_CLIENT;
  worker.sock     = INVALID_SOCKET;
  worker.sender   = 0;
  worker.receiver = 0;
}
//==============================================================================
int sock_client_start()
{
  char tmp[128];

  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_client_start(SERVER_STREAMER), Port: %d, Host: %s", worker.port, worker.host);
  else
    sprintf(tmp, "sock_client_start(CLIENT_STREAMER), Port: %d, Host: %s", worker.port, worker.host);
  log_add(tmp, LOG_DEBUG);

  worker.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker.sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_client_start, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
  {
    sprintf(tmp, "sock_client_start, socket: %d", worker.sock);
    log_add(tmp, LOG_DEBUG);
  };

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker.port);
  addr.sin_addr.s_addr = inet_addr(worker.host);

  while(1)
  {
    if(connect(worker.sock, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
    {
      sprintf(tmp, "sock_client_start, connect, Error: %d", sock_get_error());
      log_add(tmp, LOG_ERROR);
      sleep(10);
      continue;
    }
    else
    {
      log_add("sock_client_start, connect", LOG_DEBUG);
      return 0;
    }
  }
}
//==============================================================================
int sock_client_work()
{
  log_add("sock_client_work", LOG_INFO);
  log_add("----------", LOG_INFO);

  pack_init(&worker.protocol);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(&worker.sender,   &tmp_attr, sock_send_worker, (void*)&worker);
  pthread_create(&worker.receiver, &tmp_attr, sock_recv_worker, (void*)&worker);

  int status;
  pthread_join(worker.sender  , (void**)&status);
  pthread_join(worker.receiver, (void**)&status);
}
//==============================================================================
int sock_client_stop()
{
  log_add("sock_client_stop", LOG_INFO);
  closesocket(worker.sock);
  return 0;
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
int sock_stream_print(pack_buffer buffer, pack_size size, sock_worker *tmp_worker)
{
  char tmp[1024];
  pack_key     key;
  pack_value   valueS;

//  clrscr();

  #ifdef SOCK_PACK_MODE
  bytes_to_hex(buffer, (pack_size)size, tmp);
  log_add(tmp, LOG_DEBUG);
  #else
  add_to_log(buffer, LOG_DEBUG);
  #endif

  pack_packet *tmp_pack = _pack_pack_current(PACK_IN, &tmp_worker->protocol);

  pack_size tmp_words_count = _pack_words_count(tmp_pack);
  for(pack_size i = 0; i < tmp_words_count; i++)
  {
    if(pack_val_by_index_as_string(tmp_pack, i, key, valueS) == PACK_OK)
    {
      sprintf(tmp, "%s: %s", key, valueS);
      log_add(tmp, LOG_INFO);
    }
  };

  pack_buffer csv;
  pack_values_to_csv(tmp_pack, ';', csv);
  log_add(csv, LOG_DATA);
}
//==============================================================================
void *sock_send_worker(void *arg)
{
  sock_worker *tmp_worker = (sock_worker*)arg;
  SOCKET sock = tmp_worker->sock;

  char tmp[1024];
  sprintf(tmp, "sock_send_worker started, socket: %d", sock);
  log_add(tmp, LOG_DEBUG);

  pack_buffer buffer;
  pack_size   size = 0;

  int tmp_errors = 0;

  while(1)
  {
    while(pack_queue_next(buffer, &size, &tmp_worker->protocol))
    {
      int res = pack_validate(buffer, size, 0, &tmp_worker->protocol);
      if(res == PACK_OK)
      {
        sock_stream_print(buffer, size, tmp_worker);

        if(sock_do_send(sock, buffer, (int)size) == SOCK_ERROR)
          tmp_errors++;
      }
    }

    if(tmp_errors > SOCK_ERRORS_COUNT)
      break;
    else
      continue;
  }

  sprintf(tmp, "sock_send_worker finished, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  return NULL;
}
//==============================================================================
void *sock_recv_worker(void *arg)
{
  sock_worker *tmp_worker = (sock_worker*)arg;
  SOCKET sock = tmp_worker->sock;

  char tmp[1024];
  sprintf(tmp, "sock_recv_worker started, socket: %d", sock);
  log_add(tmp, LOG_DEBUG);

  pack_buffer buffer;
  int         size = 0;

  fd_set rfds;
  struct timeval tv;
  int retval;

  tv.tv_sec = 5;
  tv.tv_usec = 0;

  while(1)
  {
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);

    retval = select(1, &rfds, NULL, NULL, &tv);
    if (retval == SOCKET_ERROR)
    {
      char tmp[128];
      sprintf(tmp, "sock_recv_worker, select, Error: %d", sock_get_error());
      log_add(tmp, LOG_ERROR);
      break;
    }
    else if(!retval)
    {
      log_add("sock_recv_worker, select, empty for 5 seconds", LOG_WARNING);
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
        break;
      }
      else if(size == 0)
      {
        log_add("sock_recv_worker, recv, socket closed", LOG_WARNING);
        break;
      }
      else if(size > PACK_BUFFER_SIZE)
      {
        log_add("sock_recv_worker, recv, buffer too big", LOG_WARNING);
        break;
      }

      int res = pack_validate(buffer, (pack_size)size, 0, &tmp_worker->protocol);
      if(res == PACK_OK)
      {
        sock_stream_print(buffer, (pack_size)size, tmp_worker);
      };
    };
  }

  sprintf(tmp, "sock_recv_worker finished, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  return NULL;
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
  //    char tmp[128];
  //    sprintf(tmp, "sock_do_send, send, result: %u", res);
  //    log_add(tmp, LOG_ERROR);
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
  };
#endif

  return SOCK_OK;
}
//==============================================================================
// Почему-то не могу передать параметры корректно,
// поэтому пока не использую
int sock_do_send_cmd(pack_protocol *protocol, int argc, ...)
{
  pack_begin(protocol);

  va_list params;
  va_start(params, argc);

  char *cmd = va_arg(params, char*);
  pack_add_cmd(cmd, protocol);

  for(int i = 0; i < argc; i++)
  {
    char *param = va_arg(params, char*);
    pack_add_param_as_string(param, protocol);
  };

  va_end(params);
  pack_end(protocol);

  return SOCK_OK;
}
//==============================================================================
int sock_send_cmd(int argc, ...)
{
  va_list params;
  va_start(params, argc);

  if(worker.mode == SOCK_MODE_CLIENT)
  {
    pack_protocol *protocol = &worker.protocol;

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

//    sock_do_send_cmd(&worker.protocol, argc, params);
  }
  else
  {
    for(int i = 0; i < clients.index; i++)
    {
      pack_protocol *protocol = &clients.items[i].protocol;

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

//      sock_do_send_cmd(&clients.items[i].protocol, argc, params);
    };
  }

  va_end(params);

  return SOCK_OK;
}
//==============================================================================
