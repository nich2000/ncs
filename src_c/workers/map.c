//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: map.c
 */
//==============================================================================
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "map.h"
#include "ncs_error.h"
#include "ncs_log.h"
#include "utils.h"
//==============================================================================
BOOL map_enable    = FALSE;
char map_path[256] = DEFAULT_MAP_PATH;
char map_file[64]  = DEFAULT_MAP_NAME;
//==============================================================================
static map_t _map;
//==============================================================================
map_t *map()
{
  return &_map;
}
//==============================================================================
int load_map()
{
  if(!map_enable)
    return make_last_error_fmt(ERROR_IGNORE, errno, "load_map, map not enabled");


  char full_file_name[256];
  sprintf(full_file_name, "%s/%s", map_path, map_file);

  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  _map.count = 0;

  FILE *f = fopen(full_file_name, "r");
  if(f == NULL)
    return make_last_error_fmt(ERROR_NORMAL, errno, "load_map, can not open file %s", full_file_name);

  while ((read = getline(&line, &len, f)) != -1)
  {
    _map.count++;

    map_item_t *tmp_item = &_map.items[_map.count-1];

    char *token = strtok(line, ";");
    memset(tmp_item->kind, 0, MAP_ITEM_SIZE);
    strcpy(tmp_item->kind, token);

    token = strtok(NULL, ";");
    memset(tmp_item->number, 0, MAP_ITEM_SIZE);
    strcpy(tmp_item->number, token);

    token = strtok(NULL, ";");
    memset(tmp_item->index, 0, MAP_ITEM_SIZE);
    strcpy(tmp_item->index, token);

    token = strtok(NULL, ";");
    memset(tmp_item->lat_f, 0, MAP_ITEM_SIZE);
    strcpy(tmp_item->lat_f, token);

    token = strtok(NULL, ";");
    memset(tmp_item->lon_f, 0, MAP_ITEM_SIZE);
    strcpy(tmp_item->lon_f, token);

    token = strtok(NULL, ";");
    memset(tmp_item->lat, 0, MAP_ITEM_SIZE);
    strcpy(tmp_item->lat, token);

    token = strtok(NULL, "=");
    if(token[strlen(token)-1] == '\n')
      token[strlen(token)-1] = '\0';
    memset(tmp_item->lon, 0, MAP_ITEM_SIZE);
    strcpy(tmp_item->lon, token);
  }

  log_add_fmt(LOG_INFO, "[MAP] load_map, file: %s, count: %d",
              full_file_name, _map.count);

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
