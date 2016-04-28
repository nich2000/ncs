//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: globals.h
 */
//==============================================================================
#ifndef GLOBALS_H
#define GLOBALS_H
//==============================================================================
#define APPLICATION_VERSION      "1.0.1.130\0"
#define APPLICATION_VERSION_SIZE 15
#define PACK_VERSION             "V01\0"
#define PACK_VERSION_SIZE        4
#define SOCK_VERSION             "SOCK001\0"
#define SOCK_VERSION_SIZE        8
#define PROTOCOL_VERSION         "PROT001\0"
#define PROTOCOL_VERSION_SIZE    8
//==============================================================================
#define DEFAULT_CMD_SERVER_PORT  5700
#define DEFAULT_WS_SERVER_PORT   5800
#define DEFAULT_WEB_SERVER_PORT  5900
#define DEFAULT_SERVER_HOST      "127.0.0.1"
//==============================================================================
#define DEFAULT_LOG_PATH         "../logs"
#define DEFAULT_LOG_NAME         "log.txt"
//==============================================================================
#define DEFAULT_STAT_PATH        "../stats"
#define DEFAULT_STAT_NAME        "stat.txt"
//==============================================================================
#define DEFAULT_REPORT_PATH      "../reports"
#define DEFAULT_REPORT_NAME      "report.txt"
//==============================================================================
#define DEFAULT_SESSION_PATH     "../sessions"
#define DEFAULT_SESSION_NAME     "session.txt"
//==============================================================================
#define DEFAULT_MAP_PATH         "../tracks"
#define DEFAULT_MAP_NAME         "default.map"
//==============================================================================
#define DEFAULT_SOCK_NAME        "DEVICE\0"
#define DEFAULT_SOCK_NO_NAME     "NONAME\0"
//==============================================================================
#define DEFAULT_WEB_PATH         "../www/"
//==============================================================================
//TODO: разнести все типы по своим хедерам
//==============================================================================
enum type_t
{
  type_unknown,
  type_client,
  type_server,
  type_remote_client
};
//==============================================================================
#define SOCK_TYPE_UNKNOWN        0
#define SOCK_TYPE_CLIENT         1
#define SOCK_TYPE_SERVER         2
#define SOCK_TYPE_REMOTE_CLIENT  3
//==============================================================================
enum mode_t
{
  mode_unknown,
  mode_cmd_client,
  mode_cmd_server,
  mode_ws_server,
  mode_web_server
};
//==============================================================================
#define SOCK_MODE_UNKNOWN        0
#define SOCK_MODE_CMD_CLIENT     1
#define SOCK_MODE_CMD_SERVER     2
#define SOCK_MODE_WS_SERVER      3
#define SOCK_MODE_WEB_SERVER     4
//==============================================================================
enum state_t
{
  state_none,
  state_start,
  state_starting,
  state_stop,
  state_stopping,
  state_pause,
  state_pausing,
  state_resume,
  state_resuming,
  state_step
};
//==============================================================================
#define STATE_NONE              0
#define STATE_START             1
#define STATE_STARTING          2
#define STATE_STOP              3
#define STATE_STOPPING          4
#define STATE_PAUSE             5
#define STATE_PAUSING           6
#define STATE_RESUME            7
#define STATE_RESUMING          8
#define STATE_STEP              9
//==============================================================================
enum active_t
{
  active_none,
  active_first,
  active_second,
  active_next
};
//==============================================================================
#define ACTIVE_NONE             0
#define ACTIVE_FIRST            1
#define ACTIVE_SECOND           2
#define ACTIVE_NEXT             3
//==============================================================================
enum register_t
{
  register_none,
  register_ok
};
//==============================================================================
#define REGISTER_NONE           0
#define REGISTER_OK             1
//==============================================================================
enum connected_t
{
  connected,
  disconnected
};
//==============================================================================
#define CONNECTED               1
#define DISCONNECTED            0
//==============================================================================
typedef int BOOL;
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
//==============================================================================
#define PACK_STRUCT_VAL_COUNT   20
//==============================================================================
//                                1  2  3  4  5    6    7   8   9  10 11   12   13   14   15   16 17 18   19 20
#define PACK_TXT_FORMAT         "<%s|%u|%u|%u|%.1f|%.1f|%ld|%ld|%d|%d|%.2f|%.2f|%.1f|%.1f|%.2f|%f|%f|%.2f|%c|%c>"
//==============================================================================
typedef struct
{
  char  _ID[32];         // 1
  int   GPStime;         // 2
  int   GPStime_s;       // 3
  int   TickCount;       // 4
  float GPSspeed;        // 5
  float GPSheading;      // 6
  float GPSlat;          // 7
  float GPSlon;          // 8
  int   int_par1;        // 9
  int   int_par2;        // 10
  float Gyro1AngleZ;     // 11
  float Gyro2AngleZ;     // 12
  float MPU1temp;        // 13
  float MPU2temp;        // 14
  float BatteryVoltage;  // 15
  float fl_par1;         // 16
  float fl_par2;         // 17
  float ExtVoltage;      // 18
  char  USBConnected;    // 19
  char  _xor;            // 20
}pack_struct_t;
//==============================================================================
#define PACK_TXT_S_FORMAT   "<%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s>"
//==============================================================================
typedef struct
{
  char _ID[32];             // 1
  char sGPStime[32];        // 2
  char sGPStime_s[32];      // 3
  char sTickCount[32];      // 4
  char sGPSspeed[32];       // 5
  char sGPSheading[32];     // 6
  char sGPSlat[32];         // 7
  char sGPSlon[32];         // 8
  char sint_par1[32];       // 9
  char sint_par2[32];       // 10
  char sGyro1AngleZ[32];    // 11
  char sGyro2AngleZ[32];    // 12
  char sMPU1temp[32];       // 13
  char sMPU2temp[32];       // 14
  char sBatteryVoltage[32]; // 15
  char sfl_par1[32];        // 16
  char sfl_par2[32];        // 17
  char sExtVoltage[32];     // 18
  char sUSBConnected[1];    // 19
  char sxor[1];             // 20
} pack_struct_s_t;
//==============================================================================
#endif //GLOBALS_H
