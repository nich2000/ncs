#ifndef GLOBALS_H
#define GLOBALS_H
//==============================================================================
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
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
#define STATE_NONE          0
#define STATE_START         1
#define STATE_STARTING      2
#define STATE_STOP          3
#define STATE_STOPPING      4
#define STATE_PAUSE         5
#define STATE_PAUSING       6
#define STATE_RESUME        7
#define STATE_RESUMING      8
#define STATE_STEP          9
//==============================================================================
typedef int BOOL;
//==============================================================================
#define PACK_STRUCT_VAL_COUNT   19
//==============================================================================
//                                1  2  3  4  5    6    7   8   9  10 11   12   13   14   15   16 17 18   19
#define PACK_TXT_FORMAT         "<%s|%u|%u|%u|%.1f|%.1f|%ld|%ld|%d|%d|%.2f|%.2f|%.1f|%.1f|%.2f|%f|%f|%.2f|%c>"
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
}pack_struct_t;
//==============================================================================
#define PACK_TXT_S_FORMAT   "<%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s>"
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
  char sUSBConnected[32];   // 19
} pack_struct_s_t;
//==============================================================================
#endif //GLOBALS_H
