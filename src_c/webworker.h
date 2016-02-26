#ifndef WEBWORKER_H
#define WEBWORKER_H

#include "customworker.h"

#define WEB_LINE_SIZE      256
#define WEB_INITIAL_PATH   "../www"

typedef struct
{
  custom_worker_t custom;
}web_worker_t;

int web_server_start(web_worker_t *worker, sock_port_t port);
int web_server_stop(web_worker_t *worker);

#endif //WEBWORKER_H
