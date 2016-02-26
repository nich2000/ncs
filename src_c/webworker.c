//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "webworker.h"
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
int web_server_start(web_worker_t *worker, sock_port_t port)
{
}
//==============================================================================
int web_server_work()
{
}
//==============================================================================
int web_server_stop(web_worker_t *worker)
{
  worker->custom.is_active = SOCK_FALSE;
}
//==============================================================================
int web_handle_connection(char *request, char *response, int *size)
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
