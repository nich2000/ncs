//==============================================================================
//==============================================================================
#include <unistd.h>
#include <pthread.h>

#include "log.h"
#include "protocol.h"
#include "protocol_utils.h"

#include "socket.h"
#include "test.h"
//==============================================================================
#define MODE_SERVER          0
#define MODE_CLIENT          1
#define SOCK_PACK_MODE
#define SOCK_SERVER_STREAMER 0
#define SOCK_BUFFER_SIZE     100
//==============================================================================
int work_mode;
SOCKET work_socket;
sock_worker_list clients;
//==============================================================================
void do_send   (SOCKET sock, pack_buffer buffer, pack_size  size);
void do_receive(SOCKET sock, pack_buffer buffer, pack_size *size);
//==============================================================================
int getSocketError()
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
    sprintf(tmp, "sock_init, WSAStartup, Error: %d", getSocketError());
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
    sprintf(tmp, "sock_deinit, WSACleanup, Error: %d", getSocketError());
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
  clients.last_id = 0;
  clients.index = 0;
}
//==============================================================================
int sock_server_start(int port)
{
  char tmp[128];
  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_start_server(SERVER_STREAMER), Port: %d", port);
  else
    sprintf(tmp, "sock_start_server(CLIENT_STREAMER), Port: %d", port);
  log_add(tmp, LOG_INFO);

  pack_init();
  pack_version(tmp);
  log_add(tmp, LOG_INFO);

  work_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (work_socket == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_start_server, socket, Error: %d", getSocketError());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
    log_add("sock_start_server, Create socket", LOG_INFO);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(work_socket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_server, bind, Error: %d", getSocketError());
    log_add(tmp, LOG_ERROR);
    return 2;
  }
  else
    log_add("sock_start_server, Bind socket", LOG_INFO);

  if (listen(work_socket, SOMAXCONN) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_server, listen, Error: %d", getSocketError());
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

  work_mode = MODE_SERVER;

  char tmp[128];
  struct sockaddr_in addr;
  int addrlen = sizeof(struct sockaddr_in);

  while(1)
  {
    SOCKET tmp_client;

    tmp_client = accept(work_socket, (struct sockaddr *)&addr, (int *)&addrlen);
    if(tmp_client == INVALID_SOCKET)
    {
      sprintf(tmp, "sock_work, accept, Error: %d", getSocketError());
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

      pthread_t tmp_sender;
      pthread_create(&tmp_sender, &tmp_attr, sock_work_send, (void*)&tmp_client);

      pthread_t tmp_receiver;
      pthread_create(&tmp_receiver, &tmp_attr, sock_work_recv, (void*)&tmp_client);

      clients.items[clients.index]._socket   = tmp_client;
      clients.items[clients.index]._sender   = tmp_sender;
      clients.items[clients.index]._receiver = tmp_receiver;
      clients.items[clients.index]._id       = clients.last_id;
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
  closesocket(work_socket);
  return 0;
}
//==============================================================================
int sock_client_init()
{
}
//==============================================================================
int sock_client_start(int port, const char *host)
{
  char tmp[128];

  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_start_client(SERVER_STREAMER), Port: %d, Host: %s", port, host);
  else
    sprintf(tmp, "sock_start_client(CLIENT_STREAMER), Port: %d, Host: %s", port, host);
  log_add(tmp, LOG_INFO);

  pack_init();
  pack_version(tmp);
  log_add(tmp, LOG_INFO);

  work_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (work_socket == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_start_client, socket, Error: %d", getSocketError());
    log_add(tmp, LOG_ERROR);
    return 1;
  }
  else
  {
    sprintf(tmp, "sock_start_client, socket: %d", work_socket);
    log_add(tmp, LOG_INFO);
  };

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(host);

  if(connect(work_socket, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_client, connect, Error: %d", getSocketError());
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
  work_mode = MODE_CLIENT;

  if(SOCK_SERVER_STREAMER)
    sock_work_recv((void*)&work_socket);
  else
    sock_work_send((void*)&work_socket);
}
//==============================================================================
int sock_client_stop()
{
  log_add("sock_stop_client", LOG_INFO);
  closesocket(work_socket);
  return 0;
}
//==============================================================================
static void *sock_work_send(void *arg)
{
  SOCKET sock = *(SOCKET*)arg;

  char tmp[512];
  sprintf(tmp, "sock_work_send started, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  pack_buffer  buffer;
  pack_size    size;
  int          counter  = 0;
  int          counter1 = 0;
//  pack_key     key;
//  pack_buffer  valueS;

  #define TEST_SEND_COUNT  100
  #define TEST_PACK_COUNT  1
  #define TEST_WORD_COUNT  5
  #define TEST_STRING_SIZE 5

  while(1)
  {
    if(((!SOCK_SERVER_STREAMER) && (work_mode == MODE_SERVER)) ||
       ((SOCK_SERVER_STREAMER)  && (work_mode == MODE_CLIENT)))
    {
      sleep(1);
      continue;
    }

    if(counter >= TEST_SEND_COUNT)
    {
      sleep(1);
      continue;
    }

    counter++;

    pack_key key;
    pack_value valueS;

    for(pack_size i = 0; i < TEST_PACK_COUNT; i++)
    {
      pack_begin();

//      pack_add_as_int("SOC", sock);
//      pack_add_as_int("CCC", counter);
      pack_add_as_int("CN1", counter1++);

      for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
      {
        if(i > 9)
          sprintf(key, "I%d", i);
        else
          sprintf(key, "IN%d", i);

        pack_add_as_int(key, rand());
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

        pack_add_as_string(key, valueS);
      };
      for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
      {
        if(i > 9)
          sprintf(key, "F%d", i);
        else
          sprintf(key, "FL%d", i);

        float rnd = (float)rand()/(float)(RAND_MAX/1000);
        pack_add_as_float(key, rnd);
      };

      pack_end();
    }

    while(!pack_queue_next(buffer, &size))
    {
      if(pack_validate(buffer, size, 0) == 0)
      {
        pack_packet *tmp_pack = _pack_pack_current(PACK_IN);

        pack_size tmp_words_count = pack_words_count(tmp_pack);
        pack_key key;
        pack_value valueS;

//        clrscr();
        for(pack_size i = 0; i < tmp_words_count; i++)
        {
          if(pack_val_by_index_as_string(tmp_pack, i, key, valueS) == 0)
          {
            sprintf(tmp, "%s: %s", key, valueS);
            log_add(tmp, LOG_DATA);
          }
        };

//        pack_buffer csv;
//        pack_values_to_csv(tmp_pack, ';', csv);
//        log_add(csv, LOG_DEBUG);

//        sprintf(tmp, "send, cnt: %d:, sock: %d, data: %s(%d)", counter1, sock, csv, size);
//        log_add(tmp, LOG_DEBUG);
      };

//      #ifdef SOCK_PACK_MODE
//      bytes_to_hex(buffer, (pack_size)size, tmp);
//      log_add(tmp, LOG_DATA);
//      #else
//      add_to_log(buffer, LOG_DATA);
//      continue;
//      #endif

      int tmp_index = 0;
      int real_buffer_size = rand() % SOCK_BUFFER_SIZE + 1;
      unsigned char tmp_buffer[real_buffer_size];
      int counter2 = 0;
      while(tmp_index < size)
      {
        int tmp_cnt;
        if((size - tmp_index) > real_buffer_size)
          tmp_cnt = real_buffer_size;
        else
          tmp_cnt = (size - tmp_index);
        memcpy(tmp_buffer, &buffer[tmp_index], tmp_cnt);
        tmp_index += tmp_cnt;

//        #ifdef SOCK_PACK_MODE
//        bytes_to_hex(tmp_buffer, (pack_size)tmp_cnt, tmp);
//        log_add(tmp, LOG_DATA);
//        #else
//        add_to_log(buffer, LOG_DATA);
//        continue;
//        #endif

        do_send(sock, tmp_buffer, tmp_cnt);

        counter2 += tmp_cnt;
        sprintf(tmp, "Bytes: %d of %d", counter2, size);
        log_add(tmp, LOG_DEBUG);
//        sleep(1);
        usleep(5000);
      }

      sleep(1);
//      usleep(1000);
    }
  }
  return NULL;
}
//==============================================================================
static void *sock_work_recv(void *arg)
{
  SOCKET sock = *(SOCKET*)arg;

  char tmp[512];
  sprintf(tmp, "sock_work_recv started, socket: %d", sock);
  log_add(tmp, LOG_INFO);

  pack_buffer buffer;
  int valread;
  int counter = 0;

  while(1)
  {
    valread = recv(sock, buffer, PACK_BUFFER_SIZE, 0);

    if(valread == SOCKET_ERROR)
    {
      sprintf(tmp, "sock_work_recv, recv, Error: %d", getSocketError());
      log_add(tmp, LOG_ERROR);
      continue;
    }

    if(valread == 0)
    {
      log_add("sock_work_recv, recv, socket closed", LOG_WARNING);
      return NULL;
    }

//    clrscr();

    sprintf(tmp, "Bytes: %d", valread);
    log_add(tmp, LOG_DEBUG);

    if(valread > PACK_BUFFER_SIZE)
      continue;

//    #ifdef SOCK_PACK_MODE
//    bytes_to_hex(buffer, (pack_size)valread, tmp);
//    log_add(tmp, LOG_DATA);
//    #else
//    add_to_log(buffer, LOG_DATA);
//    continue;
//    #endif

    counter += valread;
    if(counter >= 21)
      sleep(1);

    int res = pack_validate(buffer, valread, 0);
//    sprintf(tmp, "Validate: %d", res);
//    log_ausleep(1000);dd(tmp, LOG_DEBUG);
    if(res == 0)
    {
      counter = 0;
      pack_packet *tmp_pack = _pack_pack_current(PACK_IN);

//      pack_buffer csv;
//      pack_values_to_csv(tmp_pack, ';', csv);
//      log_add(csv, LOG_DATA);

      pack_size tmp_words_count = pack_words_count(tmp_pack);

//      sprintf(tmp, "Words: %d", tmp_words_count);
//      log_add(tmp, LOG_DEBUG);

      pack_key key;
      pack_value valueS;

//      clrscr();
      for(pack_size i = 0; i < tmp_words_count; i++)
      {
        if(pack_val_by_index_as_string(tmp_pack, i, key, valueS) == 0)
        {
          sprintf(tmp, "%s: %s", key, valueS);
          log_add(tmp, LOG_DATA);
        }
      };
    }
    else
    {
//      sprintf(tmp, "sock_work_recv, validate: %d", res);
//      log_add(tmp, LOG_DEBUG);
    }
  }

  return NULL;
}
//==============================================================================
void do_send(SOCKET sock, pack_buffer buffer, pack_size size)
{
  if(send(sock, buffer, size, 0) == SOCKET_ERROR)
  {
    char tmp[128];
    sprintf(tmp, "do_send, send, Error: %u", getSocketError());
    log_add(tmp, LOG_ERROR);
  };
}
//==============================================================================
void do_receive(SOCKET sock, pack_buffer buffer, pack_size *size)
{
}
//==============================================================================
