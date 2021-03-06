//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: socket.c
 */
//==============================================================================
// strcpy
#include <string.h>

#include "socket.h"

#include "ncs_log.h"
#include "ncs_error.h"
#include "protocol_types.h"
//==============================================================================
const char *_sock_version()
{
  return SOCK_VERSION;
}
//==============================================================================
int sock_error()
{
  #ifdef __linux__
    return errno;
  #elif _WIN32
    return WSAGetLastError();
  #endif
}
//==============================================================================
int sock_init()
{
#ifdef __linux__
  // linux
#elif _WIN32
  WSADATA wsaData;

  if (WSAStartup(WINSOCK_VERSION, &wsaData))
  {
    WSACleanup();

    log_add_fmt(LOG_INFO, "[SOCK] sock_init, WSAStartup, error: %d",
                sock_error());
    return make_last_error_fmt(ERROR_NORMAL, errno, "sock_init, WSAStartup, error: %d", sock_error());
  }
  else
    log_add(LOG_DEBUG, "[SOCK] sock_init, WSAStartup OK");
#else
  // other
#endif

  return ERROR_NONE;
}
//==============================================================================
int sock_deinit()
{
#ifdef __linux__
  // linux
#elif _WIN32
  if (WSACleanup())
  {
    log_add_fmt(LOG_INFO, "[SOCK] sock_deinit, WSACleanup, error: %d",
                sock_error());
    return make_last_error_fmt(ERROR_NORMAL, errno, "sock_deinit, WSACleanup, error: %d", sock_error());
  }
  else
    log_add(LOG_DEBUG, "[SOCK] sock_deinit, WSACleanup OK");
#else
  // other
#endif

  return ERROR_NONE;
}
//==============================================================================
// http://www.gnu.org/software/libc/manual/html_node/index.html#SEC_Contents
// http://www.gnu.org/software/libc/manual/html_node/Server-Example.html
//==============================================================================
int sock_accept(SOCKET sock, SOCKET *remote_sock, sock_host_t host, sock_port_t *port)
{
  struct timeval tv;
  fd_set active_fd_set, read_fd_set;

  FD_ZERO(&active_fd_set);
  FD_SET(sock, &active_fd_set);

  tv.tv_sec  = SOCK_WAIT_SELECT;
  tv.tv_usec = 0;

  read_fd_set = active_fd_set;

  int res = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &tv);
  if(res == SOCKET_ERROR)
  {
    char tmp[256];
    int error = sock_error();
    sprintf(tmp, "[SOCK] sock_accept, select, socket: %d, error: %d", sock, error);
    log_add(LOG_ERROR, tmp);
    return make_last_error(ERROR_NORMAL, error, tmp);
  }
  else if(res == 0)
  {
    log_add_fmt(LOG_WAIT, "[SOCK] sock_accept, select, socket: %d, empty for %d seconds",
                sock, SOCK_WAIT_SELECT);

    return ERROR_WAIT;
  }
  else
  {
//    for(int i = 0; i < FD_SETSIZE; i++)
//    {
//      if(FD_ISSET(i, &read_fd_set))
//      {
//        if(i == sock)
//        {
          struct sockaddr_in addr;
          socklen_t addrlen = sizeof(struct sockaddr_in);
          *remote_sock = accept(sock, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
          if(*remote_sock == INVALID_SOCKET)
          {
            char tmp[256];
            int error = sock_error();
            sprintf(tmp, "[SOCK] sock_accept, select, socket: %d, error: %d", sock, error);
            log_add(LOG_ERROR, tmp);
            return make_last_error(ERROR_NORMAL, error, tmp);
          }
          else
          {
            struct sockaddr_in tmp_addr;
            socklen_t tmp_len = sizeof(tmp_addr);
            getsockname(*remote_sock, (struct sockaddr*)&tmp_addr, (socklen_t*)&tmp_len);
            strcpy((char*)host, inet_ntoa(tmp_addr.sin_addr));
            *port = ntohs(tmp_addr.sin_port);

//            FD_SET(*remote_sock, &active_fd_set);
            return ERROR_NONE;
          }
//        }
//      }
//    }
  }

  return ERROR_NORMAL;
}
//==============================================================================
int sock_connect(SOCKET sock, sock_port_t port, sock_host_t host)
{
  struct sockaddr_in addr;
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port);
  addr.sin_addr.s_addr = inet_addr((char*)host);

  if(connect(sock, (struct sockaddr *)&addr , sizeof(addr)) == SOCKET_ERROR)
    return ERROR_NORMAL;
  else
    return ERROR_NONE;
}
//==============================================================================
int sock_recv(SOCKET sock, char *buffer, int *size)
{
  struct timeval tv;
  fd_set active_fd_set, read_fd_set;

  FD_ZERO(&active_fd_set);
  FD_SET(sock, &active_fd_set);

  tv.tv_sec  = SOCK_WAIT_SELECT;
  tv.tv_usec = 0;

  read_fd_set = active_fd_set;

  int res = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &tv);

  if (res == SOCKET_ERROR)
  {
    int error = sock_error();
    return make_last_error_fmt(ERROR_NORMAL, errno, "[SOCK] sock_recv, select, socket: %d, error: %d",
                               sock, error);
  }
  else if(res == 0)
  {
    return make_last_error_fmt(ERROR_WAIT, errno, "[SOCK] sock_recv, select, socket: %d, empty for %d seconds",
                           sock, SOCK_WAIT_SELECT);
  }
  else
  {
    *size = recv(sock, buffer, SOCK_BUFFER_SIZE, 0);

    if(*size == SOCKET_ERROR)
    {
      int error = sock_error();
      return make_last_error_fmt(ERROR_NORMAL, errno, "[SOCK] sock_recv, recv, socket: %d, error: %d",
                                 sock, error);
    }
    else if(*size == 0)
    {
      return make_last_error_fmt(ERROR_WARNING, errno, "[SOCK] sock_recv, recv, socket: %d, socket closed",
                                 sock);
    }
    else
    {
//      pack_buffer_t tmp;
//      bytes_to_hex((unsigned char*)buffer, *size, (unsigned char*)tmp);
//      log_add_fmt(LOG_DEBUG, "sock_recv, socket: %d,\n%s",
//                  sock, (char*)tmp);

      return ERROR_NONE;
    }
  }

  return make_last_error(ERROR_NORMAL, errno, "[SOCK] sock_recv, unhandled result");
}
//==============================================================================
int sock_send(SOCKET sock, char *buffer, int size)
{
  #ifdef SOCK_RANDOM_BUFFER
  int tmp_index = 0;
  int real_buffer_size = rand() % SOCK_PART_SIZE + 1;
  char tmp_buffer[real_buffer_size];

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
      sprintf(tmp, "[SOCK] sock_send, send, error: %u", sock_get_error());
      log_add(tmp, LOG_ERROR);
    }
    else
    {
      #ifdef SOCK_EXTRA_LOGS
      log_add_fmt(LOG_EXTRA, "[SOCK] sock_send, socket: %d, send size: %d",
                  sock, size);
      log_add_fmt(LOG_EXTRA, "[SOCK] sock_send, buffer:\n%s",
                  buffer);
      bytes_to_hex(buffer, (pack_size_t)size, tmp);
      log_add_fmt(LOG_EXTRA, "[SOCK] sock_send, hex buffer:\n%s",
                  buffer);
      #endif
    }
  }
  #else
  int res = send(sock, buffer, size, 0);
  if(res == SOCKET_ERROR)
  {
    char tmp[128];
    sprintf(tmp, "[SOCK] sock_send, send, socket: %d, error: %u", sock, sock_error());
    log_add(LOG_ERROR, tmp);
    return make_last_error(ERROR_NORMAL, errno, tmp);
  }
  else
  {
//    pack_buffer_t tmp;
//    bytes_to_hex((unsigned char*)buffer, size, (unsigned char*)tmp);
//    log_add_fmt(LOG_DEBUG, "sock_send, socket: %d,\n%s",
//                sock, (char*)tmp);
  }
  #endif

  return ERROR_NONE;
}
//==============================================================================
