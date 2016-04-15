//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
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

    map_item_t *map_item = &_map.items[_map.count-1];

    char *token = strtok(line, ";");
    memset(map_item->kind, 0, MAP_ITEM_SIZE);
    strcpy(map_item->kind, token);

    token = strtok(NULL, ";");
    memset(map_item->number, 0, MAP_ITEM_SIZE);
    strcpy(map_item->number, token);

    token = strtok(NULL, ";");
    memset(map_item->index, 0, MAP_ITEM_SIZE);
    strcpy(map_item->index, token);

    token = strtok(NULL, ";");
    memset(map_item->lat_f, 0, MAP_ITEM_SIZE);
    strcpy(map_item->lat_f, token);

    token = strtok(NULL, ";");
    memset(map_item->lon_f, 0, MAP_ITEM_SIZE);
    strcpy(map_item->lon_f, token);

    token = strtok(NULL, ";");
    memset(map_item->lat, 0, MAP_ITEM_SIZE);
    strcpy(map_item->lat, token);

    token = strtok(NULL, "=");
    if(token[strlen(token)-1] == '\n')
      token[strlen(token)-1] = '\0';
    memset(map_item->lon, 0, MAP_ITEM_SIZE);
    strcpy(map_item->lon, token);
  }

  log_add_fmt(LOG_INFO, "load_map, file: %s, points count: %d",
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
