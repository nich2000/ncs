//==============================================================================
//==============================================================================
#include <unistd.h>
#include <pthread.h>

#include "log.h"
#include "protocol.h"
#include "socket.h"
#include "test.h"
//==============================================================================
#define SOCK_SERVER_STREAMER 0
//==============================================================================
SOCKET work_socket;
sock_worker_list clients;
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
    add_to_log(tmp, LOG_ERROR);

    WSACleanup();

    return 1;
  }
  else
    add_to_log("sock_init, WSAStartup OK", LOG_INFO);
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
    add_to_log(tmp, LOG_ERROR);
    return 1;
  }
  else
    add_to_log("sock_deinit, WSACleanup OK", LOG_INFO);
#else
#endif

  return 0;
}
//==============================================================================
int sock_server_start(int port)
{
  char tmp[128];
  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_start_server(SERVER_STREAMER), Port: %d", port);
  else
    sprintf(tmp, "sock_start_server(CLIENT_STREAMER), Port: %d", port);
  add_to_log(tmp, LOG_INFO);

  pack_init();
  pack_version(tmp);
  add_to_log(tmp, LOG_INFO);

  work_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (work_socket == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_start_server, socket, Error: %d", getSocketError());
    add_to_log(tmp, LOG_ERROR);
    return 1;
  }
  else
    add_to_log("sock_start_server, Create socket", LOG_INFO);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(work_socket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_server, bind, Error: %d", getSocketError());
    add_to_log(tmp, LOG_ERROR);
    return 2;
  }
  else
    add_to_log("sock_start_server, Bind socket", LOG_INFO);

  if (listen(work_socket, SOMAXCONN) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_server, listen, Error: %d", getSocketError());
    add_to_log(tmp, LOG_ERROR);
    return 3;
  }
  else
    add_to_log("sock_start_server, Listen socket", LOG_INFO);

  return 0;
}
//==============================================================================
int sock_server_work()
{
  add_to_log("sock_work", LOG_INFO);

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
      add_to_log(tmp, LOG_ERROR);
      return 1;
    }
    else
    {
      sprintf(tmp, "sock_work, accept, socket: %d, ip: %s, port: %d",
              tmp_client, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
      add_to_log(tmp, LOG_INFO);
    };

    if(clients.index < SOCK_WORKERS_COUNT)
    {
      clients.items[clients.index]._socket = tmp_client;

      pthread_attr_t tmp_attr;
      pthread_attr_init(&tmp_attr);
      pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

      pthread_t tmp_thread;

      void *func;
      if(SOCK_SERVER_STREAMER)
        func = sock_work_send;
      else
        func = sock_work_recv;

      if(pthread_create(&tmp_thread, &tmp_attr, func, (void*)&tmp_client))
      {
        if(pthread_join(tmp_thread, NULL) == 0)
        {
          clients.items[clients.index]._thread = tmp_thread;
          clients.index++;
        }
      }
    }
    else
      return 3;
  };

  return 0;
}
//==============================================================================
int sock_server_stop()
{
  add_to_log("sock_stop_server", LOG_INFO);
  closesocket(work_socket);
  return 0;
}
//==============================================================================
int sock_client_start(int port, const char *host)
{
  char tmp[128];

  if(SOCK_SERVER_STREAMER)
    sprintf(tmp, "sock_start_client(SERVER_STREAMER), Port: %d, Host: %s", port, host);
  else
    sprintf(tmp, "sock_start_client(CLIENT_STREAMER), Port: %d, Host: %s", port, host);
  add_to_log(tmp, LOG_INFO);

  pack_init();
  pack_version(tmp);
  add_to_log(tmp, LOG_INFO);

  work_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (work_socket == INVALID_SOCKET)
  {
    sprintf(tmp, "sock_start_client, socket, Error: %d", getSocketError());
    add_to_log(tmp, LOG_ERROR);
    return 1;
  }
  else
  {
    sprintf(tmp, "sock_start_client, socket: %d", work_socket);
    add_to_log(tmp, LOG_INFO);
  };

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(host);

  if(connect(work_socket, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
  {
    sprintf(tmp, "sock_start_client, connect, Error: %d", getSocketError());
    add_to_log(tmp, LOG_ERROR);
    return 1;
  }

  sprintf(tmp, "sock_start_client, connect");
  add_to_log(tmp, LOG_INFO);

  return 0;
}
//==============================================================================
int sock_client_work()
{
  if(SOCK_SERVER_STREAMER)
    sock_work_recv((void*)&work_socket);
  else
    sock_work_send((void*)&work_socket);
}
//==============================================================================
int sock_client_stop()
{
  add_to_log("sock_stop_client", LOG_INFO);
  closesocket(work_socket);
  return 0;
}
//==============================================================================
static void *sock_work_send(void *arg)
{
  SOCKET sock = *(SOCKET*)arg;

  char         tmp[128];

  sprintf(tmp, "sock_work_send, socket: %d", sock);
  add_to_log(tmp, LOG_INFO);

  pack_buffer  buffer;
  pack_size    size;
  int          counter  = 0;
  int          counter1 = 0;
//  pack_key     key;
//  pack_buffer  valueS;

  #define TEST_PACK_COUNT  5
  #define TEST_WORD_COUNT  3
  #define TEST_STRING_SIZE 3

  while(1)
  {
    counter++;

    for(pack_size i = 0; i < TEST_PACK_COUNT; i++)
    {
      pack_begin();
      pack_add_as_int("SOC", sock);
      pack_add_as_int("СNT", counter);
      pack_add_as_int("CN1", counter1++);

//      for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
//      {
//        if(i > 9)
//          sprintf(key, "I%d", i);
//        else
//          sprintf(key, "IN%d", i);

//        pack_add_as_int(key, rand());
//      };
//      for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
//      {
//        if(i > 9)
//          sprintf(key, "S%d", i);
//        else
//          sprintf(key, "ST%d", i);

//        pack_size j = 0;
//        for(j = 0; j < TEST_STRING_SIZE; j++)
//          valueS[j] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 26];
//        valueS[j] = '\0';

//        pack_add_as_string(key, valueS, strlen(valueS));
//      };
//      for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
//      {
//        if(i > 9)
//          sprintf(key, "F%d", i);
//        else
//          sprintf(key, "FL%d", i);

//        float rnd = (float)rand()/(float)(RAND_MAX/1000);
//        pack_add_as_float(key, rnd);
//      };

      pack_end();
    }

    while(!pack_queue_next(buffer, &size))
    {
      if(pack_validate(buffer, 0) == 0)
      {
        pack_packet *tmp_pack = _pack_pack_current(PACK_IN);
        pack_buffer csv;
        pack_values_to_csv(tmp_pack, ';', csv);
        sprintf(tmp, "send, cnt: %d:, sock: %d, data: %s", counter1, sock, csv);
        add_to_log(tmp, LOG_DEBUG);
      };

      if(send(sock, buffer, size, 0) == SOCKET_ERROR)
      {
        sprintf(tmp, "sock_send_data, send, Error: %u", getSocketError());
        add_to_log(tmp, LOG_ERROR);
//        return NULL;
      };

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

  char         tmp[128];

  sprintf(tmp, "sock_work_recv, socket: %d", sock);
  add_to_log(tmp, LOG_INFO);

  pack_buffer buffer;
  int valread;

  int counter = 0;

  while(1)
  {
    valread = recv(sock, buffer, PACK_BUFFER_SIZE, 0);

    counter++;

    if(valread == SOCKET_ERROR)
    {
      sprintf(tmp, "sock_handle_connection, recv, Error: %d", getSocketError());
      add_to_log(tmp, LOG_ERROR);
      continue;
//      return NULL;
    }

    if(valread == 0)
    {
      add_to_log("sock_handle_connection, recv, socket closed", LOG_WARNING);
      return NULL;
    }

    if(pack_validate(buffer, 0) == 0)
    {
      pack_packet *tmp_pack = _pack_pack_current(PACK_IN);
      pack_buffer csv;
      pack_values_to_csv(tmp_pack, ';', csv);
      sprintf(tmp, "recv, cnt: %d, sock: %d, data: %s", counter, sock, csv);
      add_to_log(tmp, LOG_DEBUG);
    };
  }

  return NULL;
}
//==============================================================================
