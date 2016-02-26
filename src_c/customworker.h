#ifndef CUSTOMWORKER_H
#define CUSTOMWORKER_H

#include "customworker.h"
#include "socket_types.h"

typedef struct
{
  sock_id_t              id;
  sock_type_t            type;
  sock_mode_t            mode;
  sock_port_t            port;
  sock_host_t            host;
  SOCKET                 sock;
  int                    is_active;
  int                    is_locked;
}custom_worker_t;

#endif //CUSTOMWORKER_H
