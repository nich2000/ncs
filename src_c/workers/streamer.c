//==============================================================================
//==============================================================================
#include <unistd.h>
#include <stdlib.h>

#include "streamer.h"
#include "ncs_log.h"
#include "protocol.h"
#include "cmdworker.h"
//==============================================================================
int cmd_streamer_init  (streamer_worker *worker, pack_protocol_t *protocol);
int cmd_streamer_start (streamer_worker *worker, pack_protocol_t *protocol);
int cmd_streamer_work  (streamer_worker *worker);
int cmd_streamer_stop  (streamer_worker *worker);
int cmd_streamer_pause (streamer_worker *worker);
int cmd_streamer_resume(streamer_worker *worker);
//==============================================================================
void *cmd_streamer_worker_func(void *arg);
int cmd_streamer_make(pack_protocol_t *protocol);
//==============================================================================
int counter = 0;
int             _cmd_streamer_count;
streamer_worker _cmd_streamer[SOCK_WORKERS_COUNT];
//==============================================================================
extern int          _cmd_client_count;
extern cmd_client_t _cmd_client[SOCK_WORKERS_COUNT];
//==============================================================================
int cmd_streamer(sock_state_t state)
{
  _cmd_streamer_count = _cmd_client_count;

  for(int i = 0; i < _cmd_streamer_count; i++)
  {
    switch(state)
    {
      case SOCK_STATE_NONE:
      {
        break;
      }
      case SOCK_STATE_START:
      {
        cmd_streamer_start(&_cmd_streamer[i], &_cmd_client[i].custom_client.custom_remote_client.protocol);
        break;
      }
      case SOCK_STATE_STOP:
      {
        cmd_streamer_stop(&_cmd_streamer[i]);
        break;
      }
      case SOCK_STATE_PAUSE:
      {
        cmd_streamer_pause(&_cmd_streamer[i]);
        break;
      }
      case SOCK_STATE_RESUME:
      {
        cmd_streamer_resume(&_cmd_streamer[i]);
        break;
      }
      default:;
    }
  }

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_status()
{
  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_init(streamer_worker *worker, pack_protocol_t *protocol)
{
  worker->is_test     =  0;
  worker->is_work     =  SOCK_FALSE;
  worker->is_pause    =  SOCK_TRUE;
  worker->last_number = -1;
  worker->protocol    =  protocol;
  worker->work_thread =  0;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_start(streamer_worker *worker, pack_protocol_t *protocol)
{
  log_add_fmt(LOG_INFO, "%s", "cmd_streamer_start");

  cmd_streamer_init(worker, protocol);

  worker->is_work  = SOCK_TRUE;
  worker->is_pause = SOCK_FALSE;

  pthread_create(&worker->work_thread, NULL, cmd_streamer_worker_func, (void*)worker);

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_stop(streamer_worker *worker)
{
  log_add_fmt(LOG_INFO, "%s", "cmd_streamer_stop");

  worker->is_work = SOCK_FALSE;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_pause(streamer_worker *worker)
{
  log_add_fmt(LOG_INFO, "%s", "cmd_streamer_pause");

  worker->is_pause = SOCK_TRUE;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_resume(streamer_worker *worker)
{
  log_add_fmt(LOG_INFO, "%s", "cmd_streamer_resume");

  worker->is_pause = SOCK_FALSE;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_work(streamer_worker *worker)
{
  return ERROR_NONE;
}
//==============================================================================
void *cmd_streamer_worker_func(void *arg)
{
  log_add_fmt(LOG_DEBUG, "%s", "cmd_streamer_worker_func");

  streamer_worker *tmp_worker = (streamer_worker*)arg;

  pack_protocol_t *tmp_protocol = tmp_worker->protocol;

  while(tmp_worker->is_work)
  {
    if(tmp_worker->is_pause)
    {
      sleep(1);
      continue;
    }

    cmd_streamer_make(tmp_protocol);

    usleep(100000);
  }

  return NULL;
}
//==============================================================================
int cmd_streamer_make(pack_protocol_t *protocol)
{
//  log_add_fmt(LOG_DEBUG, "%s", "BEGIN cmd_streamer_make");

  #define TEST_SEND_COUNT  1
  #define TEST_PACK_COUNT  1
  #define TEST_WORD_COUNT  5
  #define TEST_STRING_SIZE 5

  pack_key_t tmp_key;
  pack_value_t tmp_value;

  for(pack_size_t i = 0; i < TEST_PACK_COUNT; i++)
  {
    protocol_begin(protocol);

    protocol_add_as_int((unsigned char*)"CNT", counter++, protocol);

    for(pack_size_t i = 0; i < TEST_WORD_COUNT; i++)
    {
      if(i > 9)
        sprintf((char*)tmp_key, "I%d", i);
      else
        sprintf((char*)tmp_key, "IN%d", i);

      protocol_add_as_int(tmp_key, rand(), protocol);
    }
    for(pack_size_t i = 0; i < TEST_WORD_COUNT; i++)
    {
      if(i > 9)
        sprintf((char*)tmp_key, "S%d", i);
      else
        sprintf((char*)tmp_key, "ST%d", i);

      pack_size_t j = 0;
      for(j = 0; j < TEST_STRING_SIZE; j++)
        tmp_value[j] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 26];
      tmp_value[j] = '\0';

      protocol_add_as_string(tmp_key, tmp_value, protocol);
    }
    for(pack_size_t i = 0; i < TEST_WORD_COUNT; i++)
    {
      if(i > 9)
        sprintf((char*)tmp_key, "F%d", i);
      else
        sprintf((char*)tmp_key, "FL%d", i);

      float rnd = (float)rand()/(float)(RAND_MAX/1000);
      protocol_add_as_float(tmp_key, rnd, protocol);
    }
    protocol_end(protocol);
  }

//  log_add_fmt(LOG_DEBUG, "%s", "END cmd_streamer_make");

  return ERROR_NONE;
}
//==============================================================================
