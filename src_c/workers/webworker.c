//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: webworker.c
 */
//==============================================================================
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "webworker.h"

#include "utils.h"
#include "socket.h"
#include "socket_types.h"
#include "socket_utils.h"
#include "ncs_log.h"
#include "exec.h"
//==============================================================================
/**
* GET /index.html HTTP/1.0 \r\n
* Host: www.paulgriffiths.net \r\n
* User-Agent: Lynx/2.8.1rel.2 libwww-FM/2.14 \r\n
* Accept-Encoding: gzip, compress \r\n
* Accept-Language: en \r\n\r\n
 */
/**
* HTTP/1.0 200 OK \r\n
* Server: PGWebServ v0.1 \r\n
* Content-Type: text/html \r\n\r\n
* <DATA> \r\n
 */
//==============================================================================
// http://csapp.cs.cmu.edu/2e/ics2/code/netp/tiny/tiny.c
// http://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
//==============================================================================
#define WEB_LINE_SIZE 256
//==============================================================================
char web_path[256] = DEFAULT_WEB_PATH;
//==============================================================================
int web_server_init(web_server_t *server);
int web_server_start(web_server_t *server, sock_port_t port);
int web_server_work(web_server_t *server);
int web_server_stop(web_server_t *server);
int web_server_pause(web_server_t *server);
int web_server_resume(web_server_t *server);
//==============================================================================
void *web_server_worker(void *arg);
//==============================================================================
int on_web_accept(void *sender, SOCKET socket, sock_host_t host);
//==============================================================================
void *web_handle_connection(void *arg);
int web_get_response(char *request, char *response, int *size);
//==============================================================================
static web_server_t _web_server;
//==============================================================================
int web_server(sock_state_t state, sock_port_t port)
{
  sock_print_server_header(SOCK_MODE_WEB_SERVER, port);

  switch(state)
  {
    case STATE_NONE:
    {
      break;
    }
    case STATE_START:
    {
      web_server_start(&_web_server, port);
      break;
    }
    case STATE_STOP:
    {
      web_server_stop(&_web_server);
      break;
    }
    case STATE_PAUSE:
    {
      web_server_pause(&_web_server);
      break;
    }
    case STATE_RESUME:
    {
      web_server_resume(&_web_server);
      break;
    }
    default:;
  }

  return ERROR_NONE;
}
//==============================================================================
int web_server_init(web_server_t *server)
{
  custom_server_init(STATIC_WEB_SERVER_ID, &server->custom_server);

  strcpy((char*)server->custom_server.custom_worker.session_id, STATIC_WEB_SERVER_NAME);
  server->custom_server.custom_worker.type = SOCK_TYPE_SERVER;
  server->custom_server.custom_worker.mode = SOCK_MODE_WEB_SERVER;

  server->custom_server.on_accept          = &on_web_accept;

  return ERROR_NONE;
}
//==============================================================================
int web_server_start(web_server_t *server, sock_port_t port)
{
  if(server->custom_server.custom_worker.state == STATE_START)
    return make_last_error(ERROR_NORMAL, errno, "web_server_start, server already started");

  web_server_init(server);

  server->custom_server.custom_worker.port  = port;
  server->custom_server.custom_worker.state = STATE_STARTING;

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_DETACHED);

  return pthread_create(&server->custom_server.work_thread, &tmp_attr, web_server_worker, (void*)server);
}
//==============================================================================
int web_server_stop(web_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_STOPPING;

  return ERROR_NONE;
}
//==============================================================================
int web_server_pause(web_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_PAUSING;

  return ERROR_NONE;
}
//==============================================================================
int web_server_resume(web_server_t *server)
{
  server->custom_server.custom_worker.state = STATE_RESUMING;

  return ERROR_NONE;
}
//==============================================================================
void *web_server_worker(void *arg)
{
  web_server_t *tmp_server = (web_server_t*)arg;

  log_add_fmt(LOG_DEBUG, "[WEB] [BEGIN] web_server_worker, server id: %d",
              tmp_server->custom_server.custom_worker.id);

  if(custom_server_start(&tmp_server->custom_server.custom_worker) >= ERROR_NORMAL)
  {
    log_add_fmt(LOG_ERROR_CRITICAL, "[WEB] web_server_worker,\nmessage: %s",
                last_error()->message);
    goto exit;
  }

  custom_server_work (&tmp_server->custom_server);
  custom_worker_stop (&tmp_server->custom_server.custom_worker);

  exit:
  log_add_fmt(LOG_DEBUG, "[WEB] [END] web_server_worker, server id: %d",
              tmp_server->custom_server.custom_worker.id);
  return NULL;
}
//==============================================================================
int on_web_accept(void *sender, SOCKET socket, sock_host_t host)
{
  custom_server_t *tmp_server = (custom_server_t*)sender;

  log_add_fmt(LOG_DEBUG, "[WEB] web_accept, server id: %d, host: %s",
              tmp_server->custom_worker.id, (char*)host);

  SOCKET *s = malloc(sizeof(SOCKET));
  if(memcpy(s, &socket, sizeof(SOCKET)) == NULL)
    return make_last_error_fmt(ERROR_NORMAL, errno, "web_accept, memcpy == NULL, server id: %d",
                               tmp_server->custom_worker.id);

  pthread_attr_t tmp_attr;
  pthread_attr_init(&tmp_attr);
  pthread_attr_setdetachstate(&tmp_attr, PTHREAD_CREATE_DETACHED);

  pthread_t tmp_pthread;
  if(pthread_create(&tmp_pthread, &tmp_attr, web_handle_connection, (void*)s) != 0)
    return make_last_error_fmt(ERROR_NORMAL, errno, "web_accept, pthread_create != 0, server id: %d",
                               tmp_server->custom_worker.id);

  return ERROR_NONE;
}
//==============================================================================
void *web_handle_connection(void *arg)
{
  SOCKET tmp_sock = *(SOCKET*)arg;
  free(arg);

  // TODO: fix web_handle_connection
//  custom_remote_client_t *tmp_client = (custom_remote_client_t*)arg;
//  SOCKET tmp_sock = tmp_client->custom_worker.sock;

  log_add_fmt(LOG_DEBUG, "[WEB] [BEGIN] web_handle_connection, server id: %d",
              _web_server.custom_server.custom_worker.id);

  char *request  = (char *)malloc(SOCK_WEB_REQUEST_SIZE);
  char *response = (char *)malloc(SOCK_WEB_RESPONSE_SIZE);
  int   size     = 0;
  int   errors   = 0;

  while(TRUE)
  {
    int res = sock_recv(tmp_sock, request, &size);

    if(res == ERROR_NONE)
    {
      if(web_get_response(request, response, &size) == ERROR_NONE)
        sock_send(tmp_sock, response, size);
      else
        log_add_fmt(LOG_ERROR, "[WEB] web_handle_connection,\nmessage: %s",
                    last_error()->message);
      break;
    }
    else if(res == ERROR_WARNING)
    {
      if(size == 0)
      {
//        if(tmp_client->on_disconnect != 0)
//          tmp_client->on_disconnect((void*)tmp_client);
        break;
      }
    }
    else if(res >= ERROR_NORMAL)
    {
//      if(tmp_client->on_error != 0)
//        tmp_client->on_error((void*)tmp_client, last_error());
      if(errors++ > SOCK_ERRORS_COUNT)
        break;
    }

    usleep(10000);
  }

  free(request);
  free(response);

  log_add_fmt(LOG_DEBUG, "[WEB] [END] web_handle_connection, server id: %d",
              _web_server.custom_server.custom_worker.id);

  return NULL;
}
//==============================================================================
#define HEADER_200 "HTTP/1.0 200 OK\r\n"
#define HEADER_400 "HTTP/1.0 400 Bad Request\r\n"
#define HEADER_401 "HTTP/1.0 401 Unauthorized\r\n"
#define HEADER_403 "HTTP/1.0 403 Forbidden\r\n"
#define HEADER_404 "HTTP/1.0 404 Not Found\r\n"
#define HEADER_500 "HTTP/1.0 500 Internal Server Error\r\n"
//==============================================================================
const char NOT_FOUND[256] =
"<!DOCTYPE html>" \
"<html>" \
"  <body>" \
"     <h1>Not Found</h1>" \
"  </body>" \
"</html>\0";
//==============================================================================
int web_get_response(char *request, char *response, int *size)
{
  char    tmp[256];

  char   *tmp_header;
  char    tmp_method[WEB_LINE_SIZE];
  char    tmp_uri[WEB_LINE_SIZE];
  char    tmp_version[WEB_LINE_SIZE];

  char    tmp_response_header[WEB_LINE_SIZE];

  char    tmp_work_dir[256];
  char    tmp_full_name[WEB_LINE_SIZE];

  char   *tmp_buffer;
  size_t  tmp_file_size = 0;

  *size = 0;

  tmp_header = strtok(request, "\r\n");
  if(tmp_header == NULL)
    return make_last_error_fmt(ERROR_NORMAL, errno, "web_get_response, empty request");

  strcpy(tmp_work_dir, web_path);

  if(sscanf(tmp_header, "%s %s %s", tmp_method, tmp_uri, tmp_version) >= 3)
  {
     log_add_fmt(LOG_DEBUG, "[WEB] request header: %s %s %s",
                 tmp_method, tmp_uri, tmp_version);

    if(strstr(tmp_method, "GET") != NULL)
    {
     // log_add(LOG_INFO, "GET request");

      if(strstr(tmp_uri, "command") != NULL)
      {
       // log_add(LOG_INFO, "command found");

        char *tmp_command = strstr(tmp_uri, "?");
        if(tmp_command != NULL)
        {
          // log_add(LOG_INFO, &tmp_command[1]);
          handle_command_ajax(NULL, &tmp_command[1]);

          strcpy(response, HEADER_200);
          strcat(response, "Content-Type: application/json\r\n");
          sprintf(tmp, "Content-Length: %u\r\n\r\n", 2);
          strcat(response, tmp);
          strcat(response, "{}");
          *size = strlen(response);
        }
      }
      else
      {
        if(strcmp("/", tmp_uri) == 0)
          strcpy(tmp_uri, "html/index.html");

        sprintf(tmp_full_name, "%s%s", tmp_work_dir, tmp_uri);
        log_add_fmt(LOG_DEBUG, "[WEB] request file: %s",
                    tmp_full_name);

        FILE *f = fopen(tmp_full_name, "rb");
        if(f == NULL)
        {
          sprintf(tmp_full_name, "%s%s", web_path, "html/utility/404.html");

          // log_add_fmt(LOG_INFO, "[WEB] file(%s) not found, responce file: %s",
                      // tmp_uri, tmp_full_name);
          log_add_fmt(LOG_WARNING, "[WEB] requested file(%s) not found",
                      tmp_uri);

          strcpy(tmp_response_header, HEADER_404);
          f = fopen(tmp_full_name, "rb");
        }
        else
        {
          strcpy(tmp_response_header, HEADER_200);
        }

        if(f != NULL)
        {
          fseek(f, 0, SEEK_END);
          tmp_file_size = ftell(f);
          tmp_buffer = (char *)malloc(tmp_file_size);
          fseek(f, 0, SEEK_SET);
          fread(tmp_buffer, tmp_file_size, 1, f);
          fclose(f);

          strcpy(response, tmp_response_header);

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
          }

          sprintf(tmp, "Content-Length: %u\r\n\r\n", (unsigned int)tmp_file_size);
          strcat(response, tmp);

          *size = strlen(response);

          memcpy(&response[*size], tmp_buffer, tmp_file_size);
          *size += tmp_file_size;

          free(tmp_buffer);
        }
        else
        {
          strcpy(response, HEADER_404);
          strcat(response, "Content-Type: text/html\r\n");
          sprintf(tmp, "Content-Length: %li\r\n\r\n", strlen(NOT_FOUND));
          strcat(response, tmp);
          strcat(response, NOT_FOUND);
          *size = strlen(response);
          log_add_fmt(LOG_WARNING, "[WEB] requested file(%s) not found",
                      tmp_full_name);
        }
      }
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int web_server_status()
{
  clr_scr();

  print_custom_worker_info(&_web_server.custom_server.custom_worker, "web_server");

  return ERROR_NONE;
}
//==============================================================================
