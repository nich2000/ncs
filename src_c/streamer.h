#ifndef STREAMER_H
#define STREAMER_H
//==============================================================================
#include <pthread.h>

#include "protocol.h"
//==============================================================================
typedef struct
{
  int            is_test;
  int            is_work;
  int            is_pause;
  int            last_number;
  pack_protocol_t *protocol;
  pthread_t      work_thread;
}streamer_worker;
//==============================================================================
int streamer_init (streamer_worker *worker, pack_protocol_t *protocol);
int streamer_start(streamer_worker *worker);
int streamer_work (streamer_worker *worker);
int streamer_stop (streamer_worker *worker);
//==============================================================================
#endif //STREAMER_H
