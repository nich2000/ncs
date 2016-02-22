#include <stdio.h>
#include <stdlib.h>

#include "string.h"

#include "webworker.h"
#include "log.h"

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

// example
// http://csapp.cs.cmu.edu/2e/ics2/code/netp/tiny/tiny.c
// http://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer

int web_handle_buffer(char *request, char *response)
{
  char *tmp_request;
  char tmp_method[WEB_LINE_SIZE];
  char tmp_uri[WEB_LINE_SIZE];
  char tmp_version[WEB_LINE_SIZE];

  tmp_request = strtok(request, "\r\n");
  log_add(tmp_request, LOG_INFO);

  if(sscanf(tmp_request, "%s %s %s", tmp_method, tmp_uri, tmp_version) >= 3)
  {
    if(strcmp("/", tmp_uri) == 0)
    {
      strcpy(tmp_uri, "index.html");

      char tmp_full_name[128];
      sprintf(tmp_full_name, "%s/%s", WEB_INITIAL_PATH, tmp_uri);

//      log_add(tmp_full_name, LOG_INFO);

      FILE *f = fopen(tmp_full_name, "r");
      if (f != NULL)
      {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

//        response = (char *)malloc(fsize + 1);
        fread(response, fsize, 1, f);
        fclose(f);

        response[fsize] = '\0';
      }
    };
  }

  return 0;
}
