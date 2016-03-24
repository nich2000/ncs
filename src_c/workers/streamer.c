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
int cmd_streamer_start (int id, streamer_worker *worker, pack_protocol_t *protocol);
int cmd_streamer_work  (streamer_worker *worker);
int cmd_streamer_stop  (streamer_worker *worker);
int cmd_streamer_pause (streamer_worker *worker);
int cmd_streamer_resume(streamer_worker *worker);
//==============================================================================
void *cmd_streamer_worker_func(void *arg);
int cmd_streamer_make_random(int id, pack_protocol_t *protocol);
int cmd_streamer_make(int id, pack_protocol_t *protocol);
//==============================================================================
int counter = 0;
int             _cmd_streamer_count;
streamer_worker _cmd_streamer[SOCK_WORKERS_COUNT];
//==============================================================================
extern int          _cmd_client_count;
extern cmd_client_t _cmd_client[SOCK_WORKERS_COUNT];
extern char *pack_struct_keys[];
//==============================================================================
int cmd_streamer(sock_state_t state)
{
  _cmd_streamer_count = _cmd_client_count;

  for(int i = 0; i < _cmd_streamer_count; i++)
  {
    switch(state)
    {
      case STATE_NONE:
      {
        break;
      }
      case STATE_START:
      {
        cmd_streamer_start(i, &_cmd_streamer[i], &_cmd_client[i].custom_client.custom_remote_client.protocol);
        break;
      }
      case STATE_STOP:
      {
        cmd_streamer_stop(&_cmd_streamer[i]);
        break;
      }
      case STATE_PAUSE:
      {
        cmd_streamer_pause(&_cmd_streamer[i]);
        break;
      }
      case STATE_RESUME:
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
  worker->id          = -1;
  worker->is_test     =  0;
  worker->is_work     =  FALSE;
  worker->is_pause    =  TRUE;
  worker->last_number = -1;
  worker->protocol    =  protocol;
  worker->work_thread =  0;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_start(int id, streamer_worker *worker, pack_protocol_t *protocol)
{
  log_add_fmt(LOG_INFO, "%s", "cmd_streamer_start");

  cmd_streamer_init(worker, protocol);

  worker->id       = id;
  worker->is_work  = TRUE;
  worker->is_pause = FALSE;

  pthread_create(&worker->work_thread, NULL, cmd_streamer_worker_func, (void*)worker);

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_stop(streamer_worker *worker)
{
  log_add_fmt(LOG_INFO, "%s", "cmd_streamer_stop");

  worker->is_work = FALSE;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_pause(streamer_worker *worker)
{
  log_add_fmt(LOG_INFO, "%s", "cmd_streamer_pause");

  worker->is_pause = TRUE;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_resume(streamer_worker *worker)
{
  log_add_fmt(LOG_INFO, "%s", "cmd_streamer_resume");

  worker->is_pause = FALSE;

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

//    cmd_streamer_make_random(tmp_worker->id, tmp_protocol);
    cmd_streamer_make(tmp_worker->id, tmp_protocol);

    usleep(100 * 1000);
  }

  return NULL;
}
//==============================================================================
int cmd_streamer_make(int id, pack_protocol_t *protocol)
{
  pack_struct_t tmp_pack;
  char tmp[32];
  sprintf(tmp, "STREAMER_%d", id);
  strcpy(tmp_pack._ID, tmp);
  tmp_pack.GPStime         = rand();
  tmp_pack.GPStime_s       = rand();
  tmp_pack.TickCount       = rand();
  tmp_pack.GPSspeed        = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.GPSheading      = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.GPSlat          = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.GPSlon          = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.int_par1        = rand();
  tmp_pack.int_par2        = rand();
  tmp_pack.Gyro1AngleZ     = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.Gyro2AngleZ     = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.MPU1temp        = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.MPU2temp        = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.BatteryVoltage  = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.fl_par1         = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.fl_par2         = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.ExtVoltage      = (float)rand()/(float)(RAND_MAX/1000);
  tmp_pack.USBConnected    = '1';

  protocol_begin(protocol);
  protocol_add_as_string((unsigned char*)pack_struct_keys[0],  tmp_pack._ID,            protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[1],  tmp_pack.GPStime,        protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[2],  tmp_pack.GPStime_s,      protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[3],  tmp_pack.TickCount,      protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[4],  tmp_pack.GPSspeed,       protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[5],  tmp_pack.GPSheading,     protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[6],  tmp_pack.GPSlat,         protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[7],  tmp_pack.GPSlon,         protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[8],  tmp_pack.int_par1,       protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[9],  tmp_pack.int_par2,       protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[10], tmp_pack.Gyro1AngleZ,    protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[11], tmp_pack.Gyro2AngleZ,    protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[12], tmp_pack.MPU1temp,       protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[13], tmp_pack.MPU2temp,       protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[14], tmp_pack.BatteryVoltage, protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[15], tmp_pack.fl_par1,        protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[16], tmp_pack.fl_par2,        protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[17], tmp_pack.ExtVoltage,     protocol);
//  protocol_add_as_string((unsigned char*)pack_struct_keys[18], tmp_pack.USBConnected,   protocol);
  protocol_end(protocol);

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_make_random(int id, pack_protocol_t *protocol)
{
//  log_add_fmt(LOG_DEBUG, "%s", "[BEGIN] cmd_streamer_make");

  #define TEST_SEND_COUNT  1
  #define TEST_PACK_COUNT  1
  #define TEST_WORD_COUNT  5
  #define TEST_STRING_SIZE 5

  pack_key_t tmp_key;
  pack_value_t tmp_value;

  for(pack_size_t i = 0; i < TEST_PACK_COUNT; i++)
  {
    protocol_begin(protocol);

    char tmp[32];
    sprintf(tmp, "STREAMER_%d", id);
    protocol_add_as_string((unsigned char*)"NAM", (unsigned char*)tmp, protocol);

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

//  log_add_fmt(LOG_DEBUG, "%s", "[END] cmd_streamer_make");

  return ERROR_NONE;
}
//==============================================================================
