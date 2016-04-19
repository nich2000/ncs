//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "streamer.h"

#include "ncs_log.h"
#include "protocol.h"
#include "utils.h"
//==============================================================================
int cmd_streamer_init  (streamer_worker *worker, custom_remote_client_t *client);
int cmd_streamer_start (streamer_worker *worker, custom_remote_client_t *client);
int cmd_streamer_work  (streamer_worker *worker);
int cmd_streamer_stop  (streamer_worker *worker);
int cmd_streamer_pause (streamer_worker *worker);
int cmd_streamer_resume(streamer_worker *worker);
//==============================================================================
void *cmd_streamer_worker_func(void *arg);
int cmd_streamer_make_random(custom_remote_client_t *client);
int cmd_streamer_make       (custom_remote_client_t *client);
//==============================================================================
static int             _counter = 0;
static int             _cmd_streamer_count;
static streamer_worker _cmd_streamer[SOCK_WORKERS_COUNT];
//==============================================================================
extern char *pack_struct_keys[];
extern cmd_clients_t _cmd_clients;
//==============================================================================
char session_path[256] = DEFAULT_SESSION_PATH;
char session_file[64]  = DEFAULT_SESSION_NAME;
//==============================================================================
session_t _session;
//==============================================================================
session_t *session()
{
  return &_session;
}
//==============================================================================
int load_session()
{
  char full_file_name[256];
  sprintf(full_file_name, "%s/%s", session_path, session_file);

  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  _session.count = 0;

  FILE *f = fopen(full_file_name, "r");
  if(f == NULL)
    return make_last_error_fmt(ERROR_NORMAL, errno, "load_session, can not open file %s", full_file_name);

  while ((read = getline(&line, &len, f)) != -1)
  {
    _session.count++;

//    pack_struct_s_t *tmp_item = &_session.items[_session.count-1];

//    char *token = strtok(line, ";");
//    memset(map_item->kind, 0, MAP_ITEM_SIZE);
//    strcpy(map_item->kind, token);

//    token = strtok(NULL, ";");
//    memset(map_item->number, 0, MAP_ITEM_SIZE);
//    strcpy(map_item->number, token);

//    token = strtok(NULL, ";");
//    memset(map_item->index, 0, MAP_ITEM_SIZE);
//    strcpy(map_item->index, token);

//    token = strtok(NULL, ";");
//    memset(map_item->lat_f, 0, MAP_ITEM_SIZE);
//    strcpy(map_item->lat_f, token);

//    token = strtok(NULL, ";");
//    memset(map_item->lon_f, 0, MAP_ITEM_SIZE);
//    strcpy(map_item->lon_f, token);

//    token = strtok(NULL, ";");
//    memset(map_item->lat, 0, MAP_ITEM_SIZE);
//    strcpy(map_item->lat, token);

//    token = strtok(NULL, "=");
//    if(token[strlen(token)-1] == '\n')
//      token[strlen(token)-1] = '\0';
//    memset(map_item->lon, 0, MAP_ITEM_SIZE);
//    strcpy(map_item->lon, token);
  }

  log_add_fmt(LOG_INFO, "[SRTEAMER] load_session, file: %s, count: %d",
              full_file_name, _session.count);

//  for(int i = 0; i < _map.count; i++)
//    printf("%s  %s  %s  %s  %s  %s  %s\n",
//           _map.items[i].kind,
//           _map.items[i].number,
//           _map.items[i].index,
//           _map.items[i].lat_f,
//           _map.items[i].lon_f,
//           _map.items[i].lat,
//           _map.items[i].lon);

  fclose(f);
  if(line)
    free(line);

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer(sock_state_t state)
{
  if(state == STATE_STEP)
  {
    cmd_streamer_step(&_cmd_clients[0].custom_client.custom_remote_client);
  }
  else
  {
    _cmd_streamer_count = cmd_client_count();

    for(int i = 0; i < _cmd_streamer_count; i++)
    {
      switch(state)
      {
        case STATE_NONE:
          break;
        case STATE_START:
        {
          cmd_streamer_start(&_cmd_streamer[i], &_cmd_clients[i].custom_client.custom_remote_client);
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
        default:
          break;
      }
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
int cmd_streamer_init(streamer_worker *worker, custom_remote_client_t *client)
{
  worker->id          =  client->custom_worker.id;
  worker->is_test     =  0;
  worker->is_work     =  FALSE;
  worker->is_pause    =  TRUE;
  worker->last_number = -1;
  worker->client      =  client;
  worker->work_thread =  0;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_start(streamer_worker *worker, custom_remote_client_t *client)
{
  log_add_fmt(LOG_INFO, "[SRTEAMER] cmd_streamer_start, worker id: %d",
              worker->id);

  cmd_streamer_init(worker, client);

  worker->is_work  = TRUE;
  worker->is_pause = FALSE;

  pthread_create(&worker->work_thread, NULL, cmd_streamer_worker_func, (void*)worker);

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_stop(streamer_worker *worker)
{
  log_add_fmt(LOG_INFO, "[SRTEAMER] cmd_streamer_stop, worker id: %d",
              worker->id);

  worker->is_work = FALSE;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_pause(streamer_worker *worker)
{
  log_add_fmt(LOG_INFO, "[SRTEAMER] cmd_streamer_pause, worker id: %d",
              worker->id);

  worker->is_pause = TRUE;

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_resume(streamer_worker *worker)
{
  log_add_fmt(LOG_INFO, "[SRTEAMER] cmd_streamer_resume, worker id: %d",
              worker->id);

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
  streamer_worker *tmp_worker = (streamer_worker*)arg;

  log_add_fmt(LOG_DEBUG, "[SRTEAMER] cmd_streamer_worker_func, worker id: %d",
              tmp_worker->id);

  while(tmp_worker->is_work)
  {
    if(tmp_worker->is_pause)
    {
      sleep(1);
      continue;
    }

    cmd_streamer_step(tmp_worker->client);

    usleep(100 * 1000);
  }

  return NULL;
}
//==============================================================================
int cmd_streamer_step(custom_remote_client_t *client)
{
  #ifdef STREAM_RANDOM_PACK
  cmd_streamer_make_random(client);
  #else
  cmd_streamer_make(client);
  #endif
}
//==============================================================================
//calc_xor
//==============================================================================
//check_xor
//==============================================================================
void fill_pack_struct(pack_struct_t *pack)
{
  strcpy(tmp_pack._ID, (char*)client->custom_worker.session_id);   // 1
  tmp_pack.GPStime         = rand();                               // 2
  tmp_pack.GPStime_s       = rand();                               // 3
  tmp_pack.TickCount       = rand();                               // 4
  tmp_pack.GPSspeed        = (float)rand()/(float)(RAND_MAX/1000); // 5
  tmp_pack.GPSheading      = (float)rand()/(float)(RAND_MAX/1000); // 6
  tmp_pack.GPSlat          = (float)rand()/(float)(RAND_MAX/1000); // 7
  tmp_pack.GPSlon          = (float)rand()/(float)(RAND_MAX/1000); // 8
  tmp_pack.int_par1        = rand();                               // 9
  tmp_pack.int_par2        = rand();                               // 10
  tmp_pack.Gyro1AngleZ     = (float)rand()/(float)(RAND_MAX/1000); // 11
  tmp_pack.Gyro2AngleZ     = (float)rand()/(float)(RAND_MAX/1000); // 12
  tmp_pack.MPU1temp        = (float)rand()/(float)(RAND_MAX/1000); // 13
  tmp_pack.MPU2temp        = (float)rand()/(float)(RAND_MAX/1000); // 14
  tmp_pack.BatteryVoltage  = (float)rand()/(float)(RAND_MAX/1000); // 15
  tmp_pack.fl_par1         = (float)rand()/(float)(RAND_MAX/1000); // 16
  tmp_pack.fl_par2         = (float)rand()/(float)(RAND_MAX/1000); // 17
  tmp_pack.ExtVoltage      = (float)rand()/(float)(RAND_MAX/1000); // 18
  tmp_pack.USBConnected    = '1';                                  // 19
//  tmp_pack._xor            = calc_xor(&tmp_pack);                // 20
}
//==============================================================================
int cmd_streamer_make(custom_remote_client_t *client)
{
  pack_struct_t tmp_pack;
  fill_pack_struct(&tmp_pack);

//  if(!check_xor(&tmp_pack, tmp_pack._xor))
//    return;

  protocol_begin(&client->protocol);

  protocol_add_as_string((unsigned char*)pack_struct_keys[0],  (unsigned char*)tmp_pack._ID, &client->protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[1],  tmp_pack.GPStime,             &client->protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[2],  tmp_pack.GPStime_s,           &client->protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[3],  tmp_pack.TickCount,           &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[4],  tmp_pack.GPSspeed,            &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[5],  tmp_pack.GPSheading,          &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[6],  tmp_pack.GPSlat,              &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[7],  tmp_pack.GPSlon,              &client->protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[8],  tmp_pack.int_par1,            &client->protocol);
  protocol_add_as_int   ((unsigned char*)pack_struct_keys[9],  tmp_pack.int_par2,            &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[10], tmp_pack.Gyro1AngleZ,         &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[11], tmp_pack.Gyro2AngleZ,         &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[12], tmp_pack.MPU1temp,            &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[13], tmp_pack.MPU2temp,            &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[14], tmp_pack.BatteryVoltage,      &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[15], tmp_pack.fl_par1,             &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[16], tmp_pack.fl_par2,             &client->protocol);
  protocol_add_as_float ((unsigned char*)pack_struct_keys[17], tmp_pack.ExtVoltage,          &client->protocol);
  protocol_add_as_char  ((unsigned char*)pack_struct_keys[18], tmp_pack.USBConnected,        &client->protocol);
  protocol_add_as_char  ((unsigned char*)pack_struct_keys[19], tmp_pack._xor,                &client->protocol);

  protocol_end(&client->protocol);

  return ERROR_NONE;
}
//==============================================================================
int cmd_streamer_make_random(custom_remote_client_t *client)
{
  #define TEST_SEND_COUNT  1
  #define TEST_PACK_COUNT  1
  #define TEST_WORD_COUNT  5
  #define TEST_STRING_SIZE 5

  pack_key_t tmp_key;
  pack_value_t tmp_value;

  for(pack_size_t i = 0; i < TEST_PACK_COUNT; i++)
  {
    protocol_begin(&client->protocol);

    protocol_add_as_string((unsigned char*)"NAM", (unsigned char*)client->custom_worker.session_id, &client->protocol);

    protocol_add_as_int((unsigned char*)"CNT", _counter++, &client->protocol);

    for(pack_size_t i = 0; i < TEST_WORD_COUNT; i++)
    {
      if(i > 9)
        sprintf((char*)tmp_key, "I%d", i);
      else
        sprintf((char*)tmp_key, "IN%d", i);

      protocol_add_as_int(tmp_key, rand(), &client->protocol);
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

      protocol_add_as_string(tmp_key, tmp_value, &client->protocol);
    }
    for(pack_size_t i = 0; i < TEST_WORD_COUNT; i++)
    {
      if(i > 9)
        sprintf((char*)tmp_key, "F%d", i);
      else
        sprintf((char*)tmp_key, "FL%d", i);

      float rnd = (float)rand()/(float)(RAND_MAX/1000);
      protocol_add_as_float(tmp_key, rnd, &client->protocol);
    }
    protocol_end(&client->protocol);
  }

  return ERROR_NONE;
}
//==============================================================================
