#ifndef STREAMER_H
#define STREAMER_H
//==============================================================================
#include <pthread.h>

#include "defines.h"
#include "protocol_types.h"
#include "socket_types.h"
#include "cmdworker.h"
//==============================================================================
typedef struct
{
  int                     id;
  int                     is_test;
  int                     is_work;
  int                     is_pause;
  int                     last_number;
  custom_remote_client_t *client;
  pthread_t               work_thread;
}streamer_worker;
//==============================================================================
int cmd_streamer(sock_state_t state);
int cmd_streamer_status();
//==============================================================================
#endif //STREAMER_H
