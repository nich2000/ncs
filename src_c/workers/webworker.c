//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "webworker.h"
#include "socket.h"
#include "socket_types.h"
#include "socket_utils.h"
#include "ncs_log.h"
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
int web_server_init(web_server_t *server);
int web_server_start(web_server_t *server, sock_port_t port);
int web_server_work(web_server_t *server);
int web_server_stop(web_server_t *server);
int web_server_pause(web_server_t *server);
//==============================================================================
void *web_server_worker(void *arg);
//==============================================================================
int web_accept(void *sender, SOCKET socket, sock_host_t host);
//==============================================================================
void *web_handle_connection(void *arg);
int web_get_response(char *request, char *response, int *size);
//==============================================================================
int          _web_server_id = 0;
web_server_t _web_server;
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

  return ERROR_NONE;
}
//==============================================================================
int web_server_init(web_server_t *server)
{
  custom_worker_init(&server->custom_server.custom_worker);

  server->custom_server.custom_worker.id   = _web_server_id++;
  server->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  server->custom_server.custom_worker.mode = SOCK_MODE_WEB_SERVER;

  server->custom_server.on_accept          = &web_accept;
}
//==============================================================================
int web_server_start(web_server_t *server, sock_port_t port)
{
  web_server_init(server);

  server->custom_server.custom_worker.port  = port;
  server->custom_server.custom_worker.state = SOCK_STATE_START;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  return pthread_create(&server->custom_server.work_thread, &tmp_attr, web_server_worker, (void*)server);
}
//==============================================================================
int web_server_stop(web_server_t *server)
{
  server->custom_server.custom_worker.state = SOCK_STATE_STOP;
}
//==============================================================================
int web_server_pause(web_server_t *server)
{
  server->custom_server.custom_worker.state = SOCK_STATE_PAUSE;
}
//==============================================================================
void *web_server_worker(void *arg)
{
  log_add("BEGIN web_server_worker", LOG_DEBUG);

  web_server_t *tmp_server = (web_server_t*)arg;

  custom_server_start(&tmp_server->custom_server.custom_worker);
  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  log_add("END web_server_worker", LOG_DEBUG);
}
//==============================================================================
int web_accept(void *sender, SOCKET socket, sock_host_t host)
{
  char tmp[256];
  sprintf(tmp, "web_accept, socket: %d", socket);
  log_add(tmp, LOG_DEBUG);

  SOCKET *s = malloc(sizeof(SOCKET));
  if(memcpy(s, &socket, sizeof(SOCKET)) == NULL)
  {
    make_error(ERROR_NORMAL, 0, "web_accept, memcpy == NULL");
    return ERROR_NORMAL;
  };

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_JOINABLE);

  if(pthread_create(NULL, &tmp_attr, web_handle_connection, (void*)s) != 0)
  {
    make_error(ERROR_NORMAL, 0, "web_accept, pthread_create != 0");
    return ERROR_NORMAL;
  };

  return ERROR_NONE;
}
//==============================================================================
void *web_handle_connection(void *arg)
{
  SOCKET tmp_sock = *(SOCKET*)arg;
  free(arg);

  char tmp[256];
  sprintf(tmp, "BEGIN web_handle_connection, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  char *request  = (char *)malloc(2048);
  char *response = (char *)malloc(1024*1024);
  int  size      = 0;

  while(1)
  {
    if(sock_recv(tmp_sock, request, &size) == ERROR_NONE)
    {
      web_get_response(request, response, &size);

      sock_send(tmp_sock, response, size);

      break;
    }

    usleep(1000);
  }

  free(request);
  free(response);

  sprintf(tmp, "END web_handle_connection, socket: %d", tmp_sock);
  log_add(tmp, LOG_DEBUG);

  return NULL;
}
//==============================================================================
int web_get_response(char *request, char *response, int *size)
{
  char    tmp[256];
  char   *tmp_header;
  char    tmp_method[WEB_LINE_SIZE];
  char    tmp_uri[WEB_LINE_SIZE];
  char    tmp_version[WEB_LINE_SIZE];
  char    tmp_full_name[256];
  char   *tmp_buffer;
  size_t  tmp_file_size = 0;

  *size = 0;

  tmp_header = strtok(request, "\r\n");

  if(sscanf(tmp_header, "%s %s %s", tmp_method, tmp_uri, tmp_version) >= 3)
  {
    if(strcmp("/", tmp_uri) == 0)
      strcpy(tmp_uri, "/index.html");

    sprintf(tmp_full_name, "%s%s", WEB_INITIAL_PATH, tmp_uri);
    log_add(tmp_full_name, LOG_DEBUG);

    FILE *f = fopen(tmp_full_name, "rb");
    if(f != NULL)
    {
      fseek(f, 0, SEEK_END);
      tmp_file_size = ftell(f);
      tmp_buffer = (char *)malloc(tmp_file_size);
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
  clr_scr();

  sock_print_custom_worker_info(&_web_server.custom_server.custom_worker, "web_server");
}
//==============================================================================
