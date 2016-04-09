//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "ncs_log.h"
#include "protocol.h"

#include "test.h"
//==============================================================================
#define TEST_PACK_COUNT 1
#define TEST_WORD_COUNT 5
#define TEST_STRING_SIZE 5
#define TEST_LOG (TEST_PACK_COUNT <= 5)
//==============================================================================
//char           tmp[128];
//clock_t        start_c, end_c;
//double         total_c;
//pack_buffer_t  buffer;
//pack_size_t    size;
//pack_index_t   ind;
//pack_key_t     key;
//int            valueI;
//pack_buffer_t  valueS;
//float          valueF;
//==============================================================================
int test_pack();
int test_gps();
int test_create_pack();
int test_validate_pack();
int test_parse_pack();
//==============================================================================
int test()
{
  return ERROR_NONE;
}
//==============================================================================
int test_gps()
{
  // Почему то ошибка
  // undefined reference to `gps_init'
  // undefined reference to `gps_parse_str'
  // undefined reference to `gps_data'
  // Файл gps_parse.h подключен
  // Если этот файл подключить в main, то все ок, эти функции видны
  // NIch 06.02.2016
  /*
  gps_init();
  gps_parse_str(GPS_TEST_STR);
  GPRMC_t *tmp_data = gps_data();
  sprintf(tmp, "%s", tmp_data->time_gps);
  log_add(tmp, LOG_DEBUG);
  */

  return ERROR_NONE;
}
//==============================================================================
int test_pack()
{
/*
  pack_begin();
  pack_add_cmd("reset");
  pack_add_param_as_int(123);
  pack_add_param_as_int(321);
  pack_end();

  while(pack_queue_next(buffer, &size))
  {
    int res = pack_validate(buffer, size, 0);
    if(res != 0)
    {
      sprintf(tmp, "pack_validate, Error: %u", res);
      log_add(tmp, LOG_ERROR);
      return 1;
    }

//    test_parse_pack();
  }

  return 0;


  start_c = clock();
  pack_init();
  for(pack_size i = 0; i < TEST_PACK_COUNT; i++)
  {
    test_create_pack();
    test_validate_pack();
    test_parse_pack();
  }
  end_c = clock();
  total_c = (double)(end_c - start_c) / CLOCKS_PER_SEC;
  sprintf(tmp, "Total time: %f", total_c);
  log_add(tmp, LOG_INFO);

  return 0;
*/

  return ERROR_NONE;
}
//==============================================================================
int test_create_pack()
{
/*
  pack_begin();

  pack_add_as_int("SND", 123);

  for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
  {
    if(i > 9)
      sprintf(key, "I%d", i);
    else
      sprintf(key, "IN%d", i);

    pack_add_as_int(key, rand());
  }


  for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
  {
    if(i > 9)
      sprintf(key, "S%d", i);
    else
      sprintf(key, "ST%d", i);

    pack_size j = 0;
    for(j = 0; j < TEST_STRING_SIZE; j++)
      valueS[j] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 26];
    valueS[j] = '\0';

    pack_add_as_string(key, valueS);
  }


  for(pack_size i = 0; i < TEST_WORD_COUNT; i++)
  {
    if(i > 9)
      sprintf(key, "F%d", i);
    else
      sprintf(key, "FL%d", i);

    float rnd = (float)rand()/(float)(RAND_MAX/1000);
    pack_add_as_float(key, rnd);
  }

  pack_end();
*/

  return ERROR_NONE;
}
//==============================================================================
int test_validate_pack()
{
/*
  int res;
  res = pack_queue_next(buffer, &size);
  if(res != 0)
  {
    sprintf(tmp, "pack_queue_next, Error: %u", res);
    log_add(tmp, LOG_ERROR);
    return 1;
  }

  res = pack_validate(buffer, size, 0);
  if(res != 0)
  {
    sprintf(tmp, "pack_validate, Error: %u", res);
    log_add(tmp, LOG_ERROR);
    return 2;
  }

  return 0;
*/

  return ERROR_NONE;
}
//==============================================================================
int test_parse_pack()
{
/*
  log_add("-----------------------------", LOG_DEBUG);

  pack_packet *tmp_pack = _pack_pack_current(PACK_IN);
  if(tmp_pack != 0)
  {
    if(TEST_LOG)
    {
      sprintf(tmp, "test_parse_pack, words_count: %d", tmp_pack->words_count);
      log_add(tmp, LOG_DEBUG);
    }
    //--------------------------------------------------------------------------
    if(TEST_LOG)
      log_add("Test CSV out", LOG_DEBUG);

    pack_keys_to_csv(tmp_pack, ';', tmp);
    if(TEST_LOG)
      log_add(tmp, LOG_DEBUG);
    else
      report_add(tmp);

    pack_values_to_csv(tmp_pack, ';', tmp);
    if(TEST_LOG)
      log_add(tmp, LOG_DEBUG);
    else
      report_add(tmp);
    //--------------------------------------------------------------------------
    if(TEST_LOG)
      log_add("Test words out", LOG_DEBUG);

    pack_size tmp_words_count = _pack_words_count(tmp_pack);
    for(pack_size i = 0; i < tmp_words_count; i++)
    {
      if(pack_val_by_index_as_string(tmp_pack, i, key, valueS) == PACK_OK)
        if(TEST_LOG)
        {
          sprintf(tmp, "%s(%d): %s", key, i, valueS);
          log_add(tmp, LOG_DEBUG);
        }
    }
    //--------------------------------------------------------------------------
    if(TEST_LOG)
      log_add("Test by key out", LOG_DEBUG);

    memcpy(key, "IN0", PACK_KEY_SIZE);
    if(pack_val_by_key_as_int(tmp_pack, key, &ind, &valueI) == PACK_OK)
      if(TEST_LOG)
      {
        sprintf(tmp, "%s(%d): %d", key, ind, valueI);
        log_add(tmp, LOG_DEBUG);
      }

    memcpy(key, "ST0", PACK_KEY_SIZE);
    if(pack_val_by_key_as_string(tmp_pack, key, &ind, valueS) == PACK_OK)
      if(TEST_LOG)
      {
        sprintf(tmp, "%s(%d): %s", key, ind, valueS);
        log_add(tmp, LOG_DEBUG);
      }

    memcpy(key, "FL0", PACK_KEY_SIZE);
    if(pack_val_by_key_as_float(tmp_pack, key, &ind, &valueF) == PACK_OK)
      if(TEST_LOG)
      {
        sprintf(tmp, "%s(%d): %f", key, ind, valueF);
        log_add(tmp, LOG_DEBUG);
      }
    //--------------------------------------------------------------------------
    if(TEST_LOG)
      log_add("Test by index out", LOG_DEBUG);

    ind = 1;
    if(pack_val_by_index_as_int(tmp_pack, ind, key, &valueI) == PACK_OK)
      if(TEST_LOG)
      {
        sprintf(tmp, "%s(%d): %d", key, ind, valueI);
        log_add(tmp, LOG_DEBUG);
      }

    ind = 7;
    if(pack_val_by_index_as_string(tmp_pack, ind, key, valueS) == PACK_OK)
      if(TEST_LOG)
      {
        sprintf(tmp, "%s(%d): %s", key, ind, valueS);
        log_add(tmp, LOG_DEBUG);
      }

    ind = 11;
    if(pack_val_by_index_as_float(tmp_pack, ind, key, &valueF) == PACK_OK)
      if(TEST_LOG)
      {
        sprintf(tmp, "%s(%d): %f", key, ind, valueF);
        log_add(tmp, LOG_DEBUG);
      }
    //--------------------------------------------------------------------------
  }
*/

  return ERROR_NONE;
}
//==============================================================================
