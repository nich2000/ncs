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
#define STATE_NONE          0
#define STATE_START         1
#define STATE_STARTING      2
#define STATE_STOP          3
#define STATE_STOPPING      4
#define STATE_PAUSE         5
#define STATE_PAUSING       6
#define STATE_RESUME        7
#define STATE_RESUMING      8
//==============================================================================
typedef int BOOL;
//==============================================================================
#define PACK_FORMAT         "<%s|%u|%u|%u|%.1f|%.1f|%ld|%ld|%d|%d|%.2f|%.2f|%.1f|%.1f|%.2f|%f|%f|%.2f|%c>"
#define PACK_FORMAT_COUNT   19
//==============================================================================
//<Car_001|0|0|81774|0.0|0.0|0|0|1|0|-221.44|5.32|39.9|38.9|4.12|1.000000|2.000000|0.01|1>
//==============================================================================
/*
typedef struct
{
  Float_t MPU1temp;       //(float градусы) temp1 = (accelgyro1.getTemperature() + 12412.0) / 340.0;
  Float_t MPU2temp;       //(float градусы) обычно датчик при комнатной температуре нагревается до 35 градусов
  Float_t MPU1magX;
  Float_t MPU1magY;
  Float_t MPU1magZ;
  Float_t MPU2magX;
  Float_t MPU2magY;
  Float_t MPU2magZ;
  Float_t Gyro1AngleZ;
  Float_t Gyro2AngleZ;
  Float_t GPSspeed;       // (float km/h) - NIch RMC строка выдает в узлах
  Float_t GPSheading;     // (float градусы)
  U32_t   GPStime;        // (float как в строке RMC) - NIch ИМХО в RMC не float, а строка
  U32_t   GPStime_s;
  Bool_t  GPSValid;       // (char) Валидные ли данные с GPS
  S32_t   GPSlat;         // (int градусы с десятичными долями градуса с 5(можно оставить 6, как прилетает с ГПС приёмника) знаками после запятой, умноженные на 100000)
  S32_t   GPSlon;         // (int градусы с десятичными долями градуса с 5(можно оставить 6, как прилетает с ГПС приёмника) знаками после запятой, умноженные на 100000)
  U8_t    ChargingState;  // (int 0-нет зарядки, 1-малый ток от USB, 2-большой ток от USB, 3-от 12В)
  Float_t BatteryVoltage; // (float Вольты с десятичными долями)
  Float_t ExtVoltage;     // (float входное напряжение)
  Bool_t  USBConnected;
} GlobalVar_t;
*/
//==============================================================================
#endif //GLOBALS_H
