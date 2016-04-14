//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#ifndef MAP_H
#define MAP_H
//==============================================================================
#include "defines.h"
#include "globals.h"
//==============================================================================
// map item example
// bs1;-1;1;52,130010000000000;23,771370000000000;5796596,462;1622429,742
//==============================================================================
#define MAP_SIZE 102400
#define MAP_ITEM_SIZE 32
typedef struct
{
  char kind[MAP_ITEM_SIZE];
  char number[MAP_ITEM_SIZE];
  char index[MAP_ITEM_SIZE];
  char lat_f[MAP_ITEM_SIZE];
  char lon_f[MAP_ITEM_SIZE];
  char lat[MAP_ITEM_SIZE];
  char lon[MAP_ITEM_SIZE];
} map_item_t;
//==============================================================================
typedef map_item_t map_items_t[MAP_SIZE];
//==============================================================================
typedef struct
{
  int         count;
  map_items_t items;
} map_t;
//==============================================================================
int load_map();
//==============================================================================
#endif //MAP_H
