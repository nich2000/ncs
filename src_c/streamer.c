//==============================================================================
//==============================================================================
#include <pthread.h>
#include <unistd.h>

#include "log.h"
#include "streamer.h"
//==============================================================================
void *streamer_worker_func(void *arg);
//==============================================================================
int streamer_init(streamer_worker *worker, pack_protocol *protocol)
{
  worker->is_test     =  0;
  worker->is_work     =  1;
  worker->is_pause    =  1;
  worker->last_number = -1;
  worker->protocol    =  protocol;
  worker->worker      =  0;

  pthread_create(&worker->worker, NULL, streamer_worker_func, (void*)worker);
}
//==============================================================================
int streamer_start(streamer_worker *worker)
{
  log_add("streamer_start", LOG_INFO);

  worker->is_pause = 0;
}
//==============================================================================
int streamer_work(streamer_worker *worker)
{
}
//==============================================================================
int streamer_stop(streamer_worker *worker)
{
  log_add("streamer_stop", LOG_INFO);

  worker->is_pause = 1;
}
//==============================================================================
void *streamer_worker_func(void *arg)
{
  streamer_worker *tmp_worker = (streamer_worker*)arg;

  pack_protocol *tmp_protocol = tmp_worker->protocol;

  while(tmp_worker->is_work)
  {
    if(tmp_worker->is_pause)
    {
      sleep(1);
      continue;
    }

    pack_begin(tmp_protocol);
    pack_add_as_int("TST", 100, tmp_protocol);
    pack_end(tmp_protocol);

    if(tmp_worker->is_test)
    {
    }
    else
    {
    }

    sleep(1);
//    usleep(100000);
  }
}
//==============================================================================
