//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "webworker.h"
#include "socket.h"
#include "socket_types.h"
#include "socket_utils.h"
#include "log.h"
//==============================================================================
/*
* GET /index.html HTTP/1.0 \r\n
* Host: www.paulgriffiths.net \r\n
* User-Agent: Lynx/2.8.1rel.2 libwww-FM/2.14 \r\n
* Accept-Encoding: gzip, compress \r\n
* Accept-Language: en \r\n\r\n
*/
/*
* HTTP/1.0 200 OK \r\n
* Server: PGWebServ v0.1 \r\n
* Content-Type: text/html \r\n\r\n
* <DATA> \r\n
*/
//==============================================================================
// http://csapp.cs.cmu.edu/2e/ics2/code/netp/tiny/tiny.c
// http://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
//==============================================================================
int web_server_init(web_worker_t *worker);
int web_server_start(web_worker_t *worker, sock_port_t port);
int web_server_work(web_worker_t *worker);
int web_server_stop(web_worker_t *worker);
int web_server_pause(web_worker_t *worker);
//==============================================================================
void *web_server_worker(void *arg);
int web_accept(SOCKET socket);
void *web_handle_connection(void *arg);
int web_get_response(char *request, char *response, int *size);
//==============================================================================
int          _web_server_id = 0;
web_worker_t _web_server;
//==============================================================================
int web_server(sock_state_t state, sock_port_t port)
{
  sock_print_server_header(SOCK_MODE_WEB_SERVER, port);

  switch(state)
  {
    case SOCK_STATE_NONE:
    {
      break;
    }
    case SOCK_STATE_START:
    {
      web_server_start(&_web_server, port);
      break;
    }
    case SOCK_STATE_STOP:
    {
      web_server_stop(&_web_server);
      break;
    }
    case SOCK_STATE_PAUSE:
    {
      web_server_pause(&_web_server);
      break;
    }
    default:;
  };

  return SOCK_OK;
}
//==============================================================================
int web_server_init(web_worker_t *worker)
{
  custom_worker_init(&worker->custom_server.custom_worker);

  worker->custom_server.custom_worker.id   = _web_server_id++;
  worker->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  worker->custom_server.custom_worker.mode = SOCK_MODE_WEB_SERVER;
  worker->custom_server.on_accept          = &web_accept;
}
//==============================================================================
int web_server_start(web_worker_t *worker, sock_port_t port)
{
  web_server_init(worker);

  worker->custom_server.custom_worker.port  = port;
  worker->custom_server.custom_worker.state = SOCK_STATE_START;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  return pthread_create(&worker->custom_server.custom_worker.work_thread, &tmp_attr, web_server_worker, (void*)worker);
}
//==============================================================================
int web_server_stop(web_worker_t *worker)
{
  worker->custom_server.custom_worker.state = SOCK_STATE_STOP;
}
//==============================================================================
int web_server_pause(web_worker_t *worker)
{
  worker->custom_server.custom_worker.state = SOCK_STATE_PAUSE;
}
//==============================================================================
void *web_server_worker(void *arg)
{
  log_add("BEGIN web_server_worker", LOG_DEBUG);

  web_worker_t *tmp_server = (web_worker_t*)arg;

  custom_server_start(&tmp_server->custom_server.custom_worker);
  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  log_add("END web_server_worker", LOG_DEBUG);
}
//==============================================================================
int web_accept(SOCKET socket)
{
  char tmp[1024];
  sprintf(tmp, "web_accept, socket: %d", socket);
  log_add(tmp, LOG_DEBUG);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  SOCKET *s = malloc(sizeof(SOCKET));
  memcpy(s, &socket, sizeof(SOCKET));

  pthread_create(NULL, &tmp_attr, web_handle_connection, (void*)s);
}
//==============================================================================
void *web_handle_connection(void *arg)
{
  SOCKET tmp_sock = *(SOCKET*)arg;
  free(arg);

  char tmp[1024];
  sprintf(tmp, "BEGIN web_handle_connection, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  char request[2048];
  char response[1024*1024];
  int  size = 0;
  int  retval = 0;

  struct timeval tv;
  tv.tv_sec  = SOCK_WAIT_SELECT;
  tv.tv_usec = 0;

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(tmp_sock, &rfds);

  while(1)
  {
    retval = select(1, &rfds, NULL, NULL, &tv);
    if (retval == SOCKET_ERROR)
    {
      sprintf(tmp, "web_handle_connection, select, socket: %d, Error: %d", tmp_sock, sock_get_error());
      log_add(tmp, LOG_ERROR);
      return NULL;
    }
    else if(!retval)
    {
      #ifdef SOCK_EXTRA_LOGS
      sprintf(tmp, "web_handle_connection, select, socket: %d, empty for %d seconds", tmp_sock, SOCK_WAIT_SELECT);
      log_add(tmp, LOG_WARNING);
      #endif
      continue;
    }
    else
    {
      size = recv(tmp_sock, request, 2048, 0);
      if(size == SOCKET_ERROR)
      {
        sprintf(tmp, "web_handle_connection, recv, socket: %d, Error: %d", tmp_sock, sock_get_error());
        log_add(tmp, LOG_ERROR);
        return NULL;
      }
      else if(!size)
      {
        sprintf(tmp, "web_handle_connection, recv, socket: %d, socket closed", tmp_sock);
        log_add(tmp, LOG_WARNING);
        return NULL;
      }

      #ifdef SOCK_EXTRA_LOGS
      sprintf(tmp, "sock_recv_worker, socket: %d, recv size: %d", tmp_sock, size);
      log_add(tmp, LOG_INFO);
      bytes_to_hex(buffer, (pack_size)size, tmp);
      log_add(tmp, LOG_INFO);
      #endif

//      char *buffer = malloc(size);
//      memcpy(buffer, request, size);
//      on_recv(request);

      web_get_response(request, response, &size);

      int res = send(tmp_sock, response, size, 0);
      if(res == SOCKET_ERROR)
      {
        char tmp[128];
        sprintf(tmp, "web_handle_connection, send, Error: %u", sock_get_error());
        log_add(tmp, LOG_ERROR);
      }
      else
      {
        #ifdef SOCK_EXTRA_LOGS
        char tmp[256];
        sprintf(tmp, "web_handle_connection, send size: %d", res);
        log_add(tmp, LOG_INFO);
        bytes_to_hex(buffer, (pack_size)size, tmp);
        log_add(tmp, LOG_INFO);
        #endif
      }

      return NULL;
    }
  }

  return NULL;
}
//==============================================================================
int web_get_response(char *request, char *response, int *size)
{
  char    tmp[128];
  char   *tmp_header;
  char    tmp_method[WEB_LINE_SIZE];
  char    tmp_uri[WEB_LINE_SIZE];
  char    tmp_version[WEB_LINE_SIZE];
  char    tmp_full_name[256];

  unsigned char *tmp_buffer;
  size_t  tmp_file_size = 0;

  *size = 0;

  tmp_header = strtok(request, "\r\n");

  if(sscanf(tmp_header, "%s %s %s", tmp_method, tmp_uri, tmp_version) >= 3)
  {
    if(strcmp("/", tmp_uri) == 0)
      strcpy(tmp_uri, "/index.html");

    sprintf(tmp_full_name, "%s%s", WEB_INITIAL_PATH, tmp_uri);
    log_add(tmp_full_name, LOG_INFO);

    FILE *f = fopen(tmp_full_name, "rb");
    if(f != NULL)
    {
      fseek(f, 0, SEEK_END);
      tmp_file_size = ftell(f);
      tmp_buffer = (unsigned char *)malloc(tmp_file_size);
      fseek(f, 0, SEEK_SET);
      fread(tmp_buffer, tmp_file_size, 1, f);
      fclose(f);

      strcpy(response, "HTTP/1.0 200 OK\r\n");

      if(strstr(tmp_full_name, "html") != NULL)
        strcat(response, "Content-Type: text/html\r\n");
      else if(strstr(tmp_full_name, "js") != NULL)
        strcat(response, "Content-Type: text/javascript\r\n");
      else if(strstr(tmp_full_name, "css") != NULL)
        strcat(response, "Content-Type: text/css\r\n");
      else if(strstr(tmp_full_name, "ico") != NULL)
        strcat(response, "Content-Type: image/x-icon\r\n");
      else if(strstr(tmp_full_name, "png") != NULL)
      {
        strcat(response, "Content-Type: image/png\r\n");
        strcat(response, "Content-Transfer-Encoding: binary\r\n");
      }
      else if(strstr(tmp_full_name, "jpg") != NULL)
      {
        strcat(response, "Content-Type: image/jpg\r\n");
        strcat(response, "Content-Transfer-Encoding: binary\r\n");
      };

      sprintf(tmp, "Content-Length: %d\r\n\r\n", tmp_file_size);
      strcat(response, tmp);

      *size = strlen(response);

      memcpy(&response[*size], tmp_buffer, tmp_file_size);
      *size += tmp_file_size;

      free(tmp_buffer);
    }
    else
    {
      strcpy(response, "HTTP/1.0 404 Not Found\r\n");
      *size = strlen(response);
    }
  }

  return 0;
}
//==============================================================================
int web_server_status()
{
  print_custom_worker_info(&_web_server.custom_server.custom_worker, "web_server");
}
//==============================================================================
