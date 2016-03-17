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
    char tmp[128];
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
    sprintf(tmp, "sock_accept, select, socket: %d, error: %d", sock, error);
    make_last_error(ERROR_NORMAL, error, tmp);
    log_add(tmp, LOG_ERROR);

    return ERROR_NORMAL;
  }
  else if(res == 0)
  {
    log_add_fmt(LOG_WAIT, "sock_accept, select, socket: %d, empty for %d seconds", sock, SOCK_WAIT_SELECT);

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
            sprintf(tmp, "sock_accept, select, accept: %d, error: %d", sock, error);
            make_last_error(ERROR_NORMAL, error, tmp);
            log_add(tmp, LOG_ERROR);

            return ERROR_NORMAL;
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
    char tmp[256];
    int error = sock_error();
    sprintf(tmp, "sock_recv, select, socket: %d, error: %d", sock, error);
    make_last_error(ERROR_NORMAL, error, tmp);
    log_add(tmp, LOG_ERROR);

    return ERROR_NORMAL;
  }
  else if(res == 0)
  {
    log_add_fmt(LOG_WAIT, "sock_recv, select, socket: %d, empty for %d seconds", sock, SOCK_WAIT_SELECT);

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
          *size = recv(sock, buffer, PACK_BUFFER_SIZE, 0);
          if(*size == SOCKET_ERROR)
          {
            char tmp[256];
            int error = sock_error();
            sprintf(tmp, "sock_recv, recv, socket: %d, error: %d", sock, error);
            make_last_error(ERROR_NORMAL, error, tmp);
            log_add(tmp, LOG_ERROR);

            return ERROR_NORMAL;
          }
          else if(*size == 0)
          {
            log_add_fmt(LOG_WARNING,  "sock_recv, recv, socket: %d, socket closed", sock);

//            FD_CLR(sock, &active_fd_set);
            return ERROR_WARNING;
          }
          else
          {
            #ifdef USE_EXTRA_LOGS
            log_add_fmt(LOG_EXTRA, "sock_recv, socket: %d, recv size: %d", sock, size);
            log_add_fmt(LOG_EXTRA, "sock_recv, buffer:\n%s", buffer);
            bytes_to_hex(buffer, (pack_size_t)size, tmp);
            log_add_fmt(LOG_EXTRA, "sock_recv, hex buffer:\n%s", buffer);
            #endif

//            FD_CLR(sock, &active_fd_set);
            return ERROR_NONE;
          };
//        }
//      }
//    }
  }

  return ERROR_NORMAL;
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
