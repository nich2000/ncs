//==============================================================================
//==============================================================================
#include <unistd.h>
#include <stdlib.h>

#include "log.h"
#include "streamer.h"
//==============================================================================
void *streamer_worker_func(void *arg);
int streamer_prepare(pack_protocol_t *protocol);
//==============================================================================
int streamer_init(streamer_worker *worker, pack_protocol_t *protocol)
{
  worker->is_test     =  0;
  worker->is_work     =  1;
  worker->is_pause    =  1;
  worker->last_number = -1;
  worker->protocol    =  protocol;
  worker->work_thread =  0;

  pthread_create(&worker->work_thread, NULL, streamer_worker_func, (void*)worker);
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

  pack_protocol_t *tmp_protocol = tmp_worker->protocol;

  while(tmp_worker->is_work)
  {
    if(tmp_worker->is_pause)
    {
      sleep(1);
      continue;
    }

//    streamer_prepare(tmp_protocol);

//    sock_server_send_to_ws(sock_server_t *server, int argc, ...);

    if(tmp_worker->is_test)
    {
    }
    else
    {
    }

    usleep(100000);
  }
}
//==============================================================================
int streamer_prepare(pack_protocol_t *protocol)
{
  #define TEST_SEND_COUNT  1
  #define TEST_PACK_COUNT  1
  #define TEST_WORD_COUNT  5
  #define TEST_STRING_SIZE 5

  pack_key_t tmp_key;
  pack_value_t tmp_value;

  for(pack_size_t i = 0; i < TEST_PACK_COUNT; i++)
  {
    pack_begin(protocol);
    for(pack_size_t i = 0; i < TEST_WORD_COUNT; i++)
    {
      if(i > 9)
        sprintf(tmp_key, "I%d", i);
      else
        sprintf(tmp_key, "IN%d", i);

      pack_add_as_int(tmp_key, rand(), protocol);
    };
    for(pack_size_t i = 0; i < TEST_WORD_COUNT; i++)
    {
      if(i > 9)
        sprintf(tmp_key, "S%d", i);
      else
        sprintf(tmp_key, "ST%d", i);

      pack_size_t j = 0;
      for(j = 0; j < TEST_STRING_SIZE; j++)
        tmp_value[j] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 26];
      tmp_value[j] = '\0';

      pack_add_as_string(tmp_key, tmp_value, protocol);
    };
    for(pack_size_t i = 0; i < TEST_WORD_COUNT; i++)
    {
      if(i > 9)
        sprintf(tmp_key, "F%d", i);
      else
        sprintf(tmp_key, "FL%d", i);

      float rnd = (float)rand()/(float)(RAND_MAX/1000);
      pack_add_as_float(tmp_key, rnd, protocol);
    };
    pack_end(protocol);
  }
}
//==============================================================================
