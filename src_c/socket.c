//==============================================================================
//==============================================================================
#include <unistd.h>
#include <pthread.h>

#include "log.h"
#include "protocol_utils.h"

#include "socket.h"
#include "test.h"
//==============================================================================
sock_worker      _worker;
sock_worker_list _clients; // MODE_SERVER
//==============================================================================
sock_streamer    _streamer;
//==============================================================================
int sock_init();
int sock_deinit();
//==============================================================================
int sock_custom_init();
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
int sock_do_work(sock_worker * worker, int wait);
//==============================================================================
void *sock_recv_worker(void *arg);
void *sock_send_worker(void *arg);
//==============================================================================
int sock_do_send(SOCKET sock, pack_buffer buffer, int  size);
//==============================================================================
int sock_handle_buffer(pack_buffer buffer, pack_size size, sock_worker *worker);
//==============================================================================
int sock_stream_print(sock_worker *worker, pack_type out, int clear, int buffer, int pack, int csv);
int sock_route_datacc(sock_worker *worker);
int sock_exec_cmdcccc(sock_worker *worker);
//==============================================================================
int sock_version(char *version)
{
  strncpy((char *)version, SOCK_VERSION, SOCK_VERSION_SIZE);

  return SOCK_OK;
}
//==============================================================================
int sock_custom_init()
{
  _worker.id                 =  0;
  _worker.mode               =  SOCK_MODE_UNKNOWN;
  _worker.sock               =  INVALID_SOCKET;
  _worker.worker_kill_flag   =  0;
  _worker.sender_kill_flag   =  0;
  _worker.receiver_kill_flag =  0;
  _worker.sender             =  0;
  _worker.receiver           =  0;
  _worker.exec_cmd           =  0;

  _streamer.is_stream        =  0;
  _streamer.last_number      = -1;
}
//==============================================================================
int sock_exit()
{
  _worker.worker_kill_flag = 1;
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

  _worker.port = port;
  memset(_worker.host, '0', sizeof(_worker.host));

  return pthread_create(&_worker.worker, &tmp_attr, sock_server_worker, (void*)&_worker);
}
//==============================================================================
void *sock_server_worker(void *arg)
{
  log_add("BEGIN sock_server_worker", LOG_INFO);

  sock_init();

  sock_server_init();
  sock_server_start();
  sock_server_work();
  sock_server_stop();
  sock_deinit();

  log_add("END sock_server_worker", LOG_INFO);
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

  _worker.port = port;
  strcpy(_worker.host, host);

  return pthread_create(&_worker.worker, &tmp_attr, sock_client_worker, (void*)&_worker);
}
//==============================================================================
void *sock_client_worker(void *arg)
{
  log_add("BEGIN sock_client_worker", LOG_INFO);

  sock_init();

  while(!_worker.worker_kill_flag)
  {
    sock_client_init();
    sock_client_start();
    sock_client_work();
    sock_client_stop();
  };

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
  sock_custom_init();

  _worker.mode      = SOCK_MODE_SERVER;

  _clients.last_id  = 0;
  _clients.index    = 0;
}
//==============================================================================
int sock_server_start()
{
  log_add("BEGIN sock_server_start", LOG_INFO);

  char tmp[128];
  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_server_start(SERVER_STREAMER), Port: %d", _worker.port);
  else
    sprintf(tmp, "sock_server_start(CLIENT_STREAMER), Port: %d", _worker.port);
  log_add(tmp, LOG_DEBUG);

  _worker.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (_worker.sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_server_start, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
    log_add("sock_server_start, socket", LOG_DEBUG);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(_worker.port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(_worker.sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_server_start, bind, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 2;
  }
  else
    log_add("sock_server_start, bind", LOG_DEBUG);

  if (listen(_worker.sock, SOMAXCONN) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_server, listen, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 3;
  }
  else
    log_add("sock_server_start, listen", LOG_DEBUG);

  log_add("END sock_server_start", LOG_INFO);

  return 0;
}
//==============================================================================
int sock_server_work()
{
  log_add("BEGIN sock_server_work", LOG_INFO);

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);

  while(!_worker.worker_kill_flag)
  {
    SOCKET tmp_client;

    log_add("sock_server_work, accept", LOG_INFO);
    tmp_client = accept(_worker.sock, (struct sockaddr *)&addr, (int *)&addrlen);
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

    if(_clients.index < SOCK_WORKERS_COUNT)
    {
      sock_worker *tmp_worker = &_clients.items[_clients.index];

      pack_init(&tmp_worker->protocol);
      tmp_worker->sock = tmp_client;
      tmp_worker->id = _clients.last_id;

      sock_do_work(tmp_worker, 0);

      _clients.index++;
      if(_clients.index > SOCK_WORKERS_COUNT)
        _clients.index++;
      _clients.last_id++;
    }
    else
      return SOCK_ERROR;
  };

  log_add("END sock_server_work", LOG_INFO);

  return SOCK_OK;
}
//==============================================================================
int sock_server_stop()
{
  log_add("sock_server_stop", LOG_INFO);
  closesocket(_worker.sock);
  return 0;
}
//==============================================================================
int sock_client_init()
{
  sock_custom_init();

  _worker.mode = SOCK_MODE_CLIENT;
}
//==============================================================================
int sock_client_start()
{
  log_add("BEGIN sock_client_start", LOG_INFO);

  char tmp[128];

  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_client_start(SERVER_STREAMER), Port: %d, Host: %s", _worker.port, _worker.host);
  else
    sprintf(tmp, "sock_client_start(CLIENT_STREAMER), Port: %d, Host: %s", _worker.port, _worker.host);
  log_add(tmp, LOG_INFO);

  _worker.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (_worker.sock == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_client_start, socket, Error: %d", sock_get_error());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
  {
    sprintf(tmp, "sock_client_start, socket: %d", _worker.sock);
    log_add(tmp, LOG_DEBUG);
  };

  log_add("END sock_client_start", LOG_INFO);
}
//==============================================================================
int sock_client_work()
{
  log_add("BEGIN sock_client_work", LOG_INFO);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(_worker.port);
  addr.sin_addr.s_addr = inet_addr(_worker.host);

  log_add("sock_client_work, connect", LOG_INFO);
  while(!_worker.worker_kill_flag)
  {
    if(connect(_worker.sock, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
    {
      #ifdef SOCK_EXTRA_LOGS
      char tmp[128];
      sprintf(tmp, "sock_client_work, connect, try in %d seconds, Error: %d", sock_get_error(), SOCK_WAIT_CONNECT);
      log_add(tmp, LOG_ERROR);
      #endif
      sleep(SOCK_WAIT_CONNECT);
      continue;
    }
    else
      break;
  };
  log_add("sock_client_work, connected", LOG_INFO);

  pack_init(&_worker.protocol);

  sock_do_work(&_worker, 1);

  log_add("END sock_client_work", LOG_INFO);
}
//==============================================================================
int sock_client_stop()
{
  log_add("sock_client_stop", LOG_INFO);
  closesocket(_worker.sock);
  return 0;
}
//==============================================================================
int sock_do_work(sock_worker *worker, int wait)
{
  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  pthread_create(&worker->sender,   &tmp_attr, sock_send_worker, (void*)worker);
  pthread_create(&worker->receiver, &tmp_attr, sock_recv_worker, (void*)worker);

  if(wait)
  {
    int status;
    pthread_join(worker->sender  , (void**)&status);
    pthread_join(worker->receiver, (void**)&status);
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
int sock_stream_print(sock_worker *worker, pack_type out, int clear, int buffer, int pack, int csv)
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
int sock_route_data(sock_worker *worker)
{
}
//==============================================================================
int sock_exec_cmd(sock_worker *worker)
{
  pack_packet *tmp_pack = _pack_pack_current(PACK_OUT, &worker->protocol);
  pack_value tmp_cmd;

  if(pack_command(tmp_pack, tmp_cmd) == PACK_OK)
  {
    if(strcmp(tmp_cmd, "stream") == 0)
    {
      pack_key   tmp_key;
      pack_value tmp_param;
      if(pack_param_by_index_as_string(tmp_pack, 1, tmp_key, tmp_param) == PACK_OK)
      {
        if(strcmp(tmp_param, "on") == 0)
          _streamer.is_stream =  1;
        else
          _streamer.is_stream = 0;
      };
    };
  };

  return SOCK_ERROR;
}
//==============================================================================
void *sock_send_worker(void *arg)
{
  sock_worker *tmp_worker = (sock_worker*)arg;
  SOCKET sock = tmp_worker->sock;

  char tmp[1024];
  sprintf(tmp, "BEGIN sock_send_worker, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  pack_buffer buffer;
  pack_size   size = 0;

  int tmp_errors = 0;

  while(!tmp_worker->sender_kill_flag)
  {
    while(pack_queue_next(buffer, &size, &tmp_worker->protocol))
    {
      int res = pack_validate(buffer, size, 1, &tmp_worker->protocol);
      if(res == PACK_OK)
      {
        sock_stream_print(tmp_worker, PACK_OUT, 0, 0, 0, 1);

        if(sock_do_send(sock, buffer, (int)size) == SOCK_ERROR)
          tmp_errors++;
      }
    }

    if(tmp_errors > SOCK_ERRORS_COUNT)
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
  sock_worker *tmp_worker = (sock_worker*)arg;
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
        log_add("sock_recv_worker, recv, buffer too big", LOG_WARNING);
        break;
      }

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
int sock_handle_buffer(pack_buffer buffer, pack_size size, sock_worker *worker)
{
  int res = pack_validate(buffer, (pack_size)size, 0, &worker->protocol);
  if(res == PACK_OK)
  {
    sock_stream_print(worker, PACK_IN, 0, 0, 1, 0);

    sock_route_data(worker);

    sock_exec_cmd(worker);
  }
  else
  {
    #ifdef SOCK_EXTRA_LOGS
    char tmp[128];
    sprintf(tmp, "sock_recv_worker, pack_validate, Place: %d", res);
    log_add(tmp, LOG_WARNING);
    #endif
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
// поэтому пока не использую и фактически дублирую код
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

  if(_worker.mode == SOCK_MODE_CLIENT)
  {
    pack_protocol *protocol = &_worker.protocol;

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

//    sock_do_send_cmd(&_worker.protocol, argc, params);
  }
  else
  {
    for(int i = 0; i < _clients.index; i++)
    {
      pack_protocol *protocol = &_clients.items[i].protocol;

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
