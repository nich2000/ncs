//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: globals.c
 */
//==============================================================================
#include "globals.h"
//==============================================================================
char *pack_struct_keys[PACK_STRUCT_VAL_COUNT]=
{
  "_ID",                 // 1
  "TIM",                 // 2
  "T_S",                 // 3
  "CNT",                 // 4
  "SPD",                 // 5
  "HEA",                 // 6
  "LAT",                 // 7
  "LON",                 // 8
  "UN1",                 // 9
  "UN2",                 // 10
  "AZ1",                 // 11
  "AZ2",                 // 12
  "MT1",                 // 13
  "MT2",                 // 14
  "BVL",                 // 15
  "UN3",                 // 16
  "UN4",                 // 17
  "EVL",                 // 18
  "USB",                 // 19
  "XOR"                  // 20
};
//==============================================================================
char *pack_struct_captions[PACK_STRUCT_VAL_COUNT]=
{
  "ID",                  // 1
  "GPStime",             // 2
  "GPStime_s",           // 3
  "TickCount",           // 4
  "GPSspeed",            // 5
  "GPSheading",          // 6
  "GPSlat",              // 7
  "GPSlon",              // 8
  "int_par1",            // 9
  "int_par2",            // 10
  "Gyro1AngleZ",         // 11
  "Gyro2AngleZ",         // 12
  "MPU1temp",            // 13
  "MPU2temp",            // 14
  "BatteryVoltage",      // 15
  "fl_par1",             // 16
  "fl_par2",             // 17
  "ExtVoltage",          // 18
  "USBConnected",        // 19
  "xor"                  // 20
};
//==============================================================================
BOOL binary_protocol = TRUE;
//==============================================================================
