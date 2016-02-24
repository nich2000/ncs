#ifndef WEBWORKER_H
#define WEBWORKER_H

#define WEB_LINE_SIZE 256

#define WEB_INITIAL_PATH "../www"

typedef struct
{
  char *uri;
  char *host;
}web_header;

int web_handle_buffer(char *request, char *response, int *size);

#endif //WEBWORKER_H
