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
int sock_do_recv(SOCKET sock, pack_buffer buffer, pack_size *size);
int sock_do_send(SOCKET sock, pack_buffer buffer, pack_size  size);
//==============================================================================
void sock_print_all_data(sock_worker *worker)
{
//pack_packet *tmp_pack = _pack_pack_current(PACK_IN, &tmp_worker->protocol);
}
//==============================================================================
//==============================================================================
int sock_server(int port)
{
  char tmp[128];
  log_set_name("server_log.txt");
  sprintf(tmp, "Server mode, port: %d", port);
  log_add(tmp, LOG_INFO);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  worker.port = port;
  memset(worker.host,'0', sizeof(worker.host));

  int res = pthread_create(&worker.worker, &tmp_attr, sock_server_worker, (void*)&worker);

//  int status;
//  pthread_join(worker.worker, (void*)&status);

  return res;
}
//==============================================================================
void *sock_server_worker(void *arg)
{
  log_add("sock_server_worker", LOG_INFO);

  sock_init();
  sock_server_init();
  sock_server_start();
  sock_server_work();
  sock_server_stop();
  sock_deinit();
}
//==============================================================================
int sock_client(int port, char *host)
{
  char tmp[128];
  log_set_name("client_log.txt");
  sprintf(tmp, "Client mode, port: %d, host: %s", port, host);
  log_add(tmp, LOG_INFO);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  worker.port = port;
  strcpy(worker.host, host);

  int res = pthread_create(&worker.worker, &tmp_attr, sock_client_worker, (void*)&worker);

//  int status;
//  pthread_join(worker.worker, (void*)&status);

  return res;
}
//==============================================================================
void *sock_client_worker(void *arg)
{
  log_add("sock_client_worker", LOG_INFO);

  sock_init();
  sock_client_init();
  sock_client_start();
  sock_client_work();
  sock_client_stop();
  sock_deinit();
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
    log_add("sock_init, WSAStartup OK", LOG_INFO);
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
    log_add("sock_deinit, WSACleanup OK", LOG_INFO);
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
    sprintf(tmp, "sock_start_server(SERVER_STREAMER), Port: %d", worker.port);
  else
    sprintf(tmp, "sock_start_server(CLIENT_STREAMER), Port: %d", worker.port);
  log_add(tmp, LOG_INFO);

  pack_version(tmp);
  log_add(tmp, LOG_INFO);

  worker.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker.sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_start_server, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
    log_add("sock_start_server, Create socket", LOG_INFO);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker.port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(worker.sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_server, bind, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 2;
  }
  else
    log_add("sock_start_server, Bind socket", LOG_INFO);

  if (listen(worker.sock, SOMAXCONN) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_server, listen, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 3;
  }
  else
    log_add("sock_start_server, Listen socket", LOG_INFO);

  return 0;
}
//==============================================================================
int sock_server_work()
{
  log_add("sock_work", LOG_INFO);

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);

  while(1)
  {
    SOCKET tmp_client;

    tmp_client = accept(worker.sock, (struct sockaddr *)&addr, (int *)&addrlen);
    if(tmp_client == INVALID_SOCKET)
    {
      sprintf(tmp, "sock_work, accept, Error: %d", sock_get_error());
      log_add(tmp, LOG_ERROR);
      return 1;
    }
    else
    {
      struct sockaddr_in sin;
      int len = sizeof(sin);
      getsockname(tmp_client, (struct sockaddr *)&sin, &len);
      sprintf(tmp, "sock_work, accept, socket: %d, ip: %s, port: %d",
              tmp_client, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
      log_add(tmp, LOG_INFO);
    };

    if(clients.index < SOCK_WORKERS_COUNT)
    {
      pthread_attr_t tmp_attr;
      pthread_attr_init(&tmp_attr);
      pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

      sock_worker *tmp_worker = &clients.items[clients.index];

      pack_init(&tmp_worker->protocol);

      tmp_worker->sock = tmp_client;
      tmp_worker->id   = clients.last_id;

      pthread_t tmp_sender;
      pthread_create(&tmp_sender, &tmp_attr, sock_send_worker, (void*)tmp_worker);
      tmp_worker->sender   = tmp_sender;

      pthread_t tmp_receiver;
      pthread_create(&tmp_receiver, &tmp_attr, sock_recv_worker, (void*)tmp_worker);
      tmp_worker->receiver = tmp_receiver;

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
  log_add("sock_stop_server", LOG_INFO);
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
    sprintf(tmp, "sock_start_client(SERVER_STREAMER), Port: %d, Host: %s", worker.port, worker.host);
  else
    sprintf(tmp, "sock_start_client(CLIENT_STREAMER), Port: %d, Host: %s", worker.port, worker.host);
  log_add(tmp, LOG_INFO);

  pack_version(tmp);
  log_add(tmp, LOG_INFO);

  worker.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (worker.sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_start_client, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
  {
    sprintf(tmp, "sock_start_client, socket: %d", worker.sock);
    log_add(tmp, LOG_INFO);
  };

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(worker.port);
  addr.sin_addr.s_addr = inet_addr(worker.host);

  if(connect(worker.sock, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_client, connect, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }

  sprintf(tmp, "sock_start_client, connect");
  log_add(tmp, LOG_INFO);

  return 0;
}
//==============================================================================
int sock_client_work()
{
  pack_init(&worker.protocol);

  if(SOCK_SERVER_STREAMER)
    sock_recv_worker((void*)&worker);
  else
    sock_send_worker((void*)&worker);
}
//==============================================================================
int sock_client_stop()
{
  log_add("sock_stop_client", LOG_INFO);
  closesocket(worker.sock);
  return 0;
}
//==============================================================================
void *sock_send_worker(void *arg)
{
  sock_worker *tmp_worker = (sock_worker*)arg;
  SOCKET sock = tmp_worker->sock;

  char tmp[1024];
  sprintf(tmp, "sock_send_worker started, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  pack_buffer  buffer;
  pack_size    size;
  pack_key     key;
  pack_value   valueS;

  int          counter  = 0;
  int          counter1 = 0;

  #define TEST_SEND_COUNT  1
  #define TEST_PACK_COUNT  1
  #define TEST_WORD_COUNT  5
  #define TEST_STRING_SIZE 5

  int tmp_errors = 0;

  while(1)
  {
    sleep(1);
//    usleep(10000000);

    if(((!SOCK_SERVER_STREAMER) && (tmp_worker->mode == SOCK_MODE_SERVER)) ||
       ((SOCK_SERVER_STREAMER)  && (tmp_worker->mode == SOCK_MODE_CLIENT)))
      continue;

    if(counter >= TEST_SEND_COUNT)
      continue;

    counter++;

    for(pack_size i = 0; i < TEST_PACK_COUNT; i++)
    {
      pack_begin(&tmp_worker->protocol);
      pack_add_as_int("SOC", sock, &tmp_worker->protocol);
      pack_add_as_int("CCC", counter, &tmp_worker->protocol);
      pack_add_as_int("CN1", counter1++, &tmp_worker->protocol);
      for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
      {
        if(i > 9)
          sprintf(key, "I%d", i);
        else
          sprintf(key, "IN%d", i);

        pack_add_as_int(key, rand(), &tmp_worker->protocol);
      };
      for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
      {
        if(i > 9)
          sprintf(key, "S%d", i);
        else
          sprintf(key, "ST%d", i);

        pack_size j = 0;
        for(j = 0; j < TEST_STRING_SIZE; j++)
          valueS[j] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 26];
        valueS[j] = '\0';

        pack_add_as_string(key, valueS, &tmp_worker->protocol);
      };
      for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
      {
        if(i > 9)
          sprintf(key, "F%d", i);
        else
          sprintf(key, "FL%d", i);

        float rnd = (float)rand()/(float)(RAND_MAX/1000);
        pack_add_as_float(key, rnd, &tmp_worker->protocol);
      };
      pack_end(&tmp_worker->protocol);
    }

    while(pack_queue_next(buffer, &size, &tmp_worker->protocol))
    {
      int res = pack_validate(buffer, size, 0, &tmp_worker->protocol);
      if(res == PACK_OK)
      {
        clrscr();

        #ifdef SOCK_PACK_MODE
        bytes_to_hex(buffer, (pack_size)size, tmp);
        log_add(tmp, LOG_DATA);
        #else
        add_to_log(buffer, LOG_DATA);
        #endif

        pack_packet *tmp_pack = _pack_pack_current(PACK_IN, &tmp_worker->protocol);

        pack_size tmp_words_count = _pack_words_count(tmp_pack);
        for(pack_size i = 0; i < tmp_words_count; i++)
        {
          if(pack_val_by_index_as_string(tmp_pack, i, key, valueS) == PACK_OK)
          {
            sprintf(tmp, "%s: %s", key, valueS);
            log_add(tmp, LOG_DATA);
          }
        };

//        pack_buffer csv;
//        pack_values_to_csv(tmp_pack, ';', csv);
//        log_add(csv, LOG_DEBUG);

        if(sock_do_send(sock, buffer, size) == SOCK_ERROR)
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
  log_add(tmp, LOG_INFO);

  pack_buffer buffer;
  pack_key    key;
  pack_value  valueS;
  pack_size   size = 0;

  int tmp_errors = 0;

  while(1)
  {
    size = recv(sock, buffer, PACK_BUFFER_SIZE, 0);

    if(size == SOCKET_ERROR)
    {
      sprintf(tmp, "sock_work_recv, recv, Error: %d", sock_get_error());
      log_add(tmp, LOG_ERROR);
      tmp_errors++;
      if(tmp_errors > SOCK_ERRORS_COUNT)
        break;
      else
        continue;
    }

    if(size == 0)
    {
      log_add("sock_work_recv, recv, socket closed", LOG_WARNING);
      return NULL;
    }

    if(size > PACK_BUFFER_SIZE)
    {
      log_add("sock_work_recv, recv, buffer too big", LOG_WARNING);
      tmp_errors++;
      if(tmp_errors > SOCK_ERRORS_COUNT)
        break;
      else
        continue;
    }

    int res = pack_validate(buffer, size, 0, &tmp_worker->protocol);
    if(res == PACK_OK)
    {
      clrscr();

      #ifdef SOCK_PACK_MODE
      bytes_to_hex(buffer, (pack_size)size, tmp);
      log_add(tmp, LOG_DATA);
      #else
      add_to_log(buffer, LOG_DATA);
      #endif

      pack_packet *tmp_pack = _pack_pack_current(PACK_IN, &tmp_worker->protocol);

      pack_size tmp_words_count = _pack_words_count(tmp_pack);
      log_add(tmp, LOG_DATA);
      for(pack_size i = 0; i < tmp_words_count; i++)
      {
        if(pack_val_by_index_as_string(tmp_pack, i, key, valueS) == PACK_OK)
        {
          sprintf(tmp, "%s: %s", key, valueS);
          log_add(tmp, LOG_DATA);
        }
      };

//      pack_buffer csv;
//      pack_values_to_csv(tmp_pack, ';', csv);
//      log_add(csv, LOG_DATA);
    };
  }

  sprintf(tmp, "sock_recv_worker finished, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  return NULL;
}
//==============================================================================
int sock_do_send(SOCKET sock, pack_buffer buffer, pack_size size)
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
int sock_do_recv(SOCKET sock, pack_buffer buffer, pack_size *size)
{
  return SOCK_OK;
}
//==============================================================================
