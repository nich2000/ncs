//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: streamer.h
 */
//==============================================================================
#ifndef STREAMER_H
#define STREAMER_H
//==============================================================================
#include <pthread.h>

#include "defines.h"
#include "globals.h"

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
#define SESSION_SIZE 102400
//==============================================================================
typedef pack_struct_s_t session_items_t[SESSION_SIZE];
//==============================================================================
typedef struct
{
  int             index;
  int             count;
  session_items_t items;
} session_t;
//==============================================================================
typedef struct
{
  float lat;
  float lon;
} coord_t;
//==============================================================================
typedef coord_t coords_items_t[SESSION_SIZE];
//==============================================================================
typedef struct
{
  int            index;
  int            count;
  coords_items_t items;
} coords_t;
//==============================================================================
int cmd_streamer       (sock_state_t state, int interval);
int cmd_streamer_status();
//==============================================================================
int session_load();
int coords_load();
//==============================================================================
#endif //STREAMER_H
