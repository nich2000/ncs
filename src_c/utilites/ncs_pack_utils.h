#ifndef PACK_UTILS_H
#define PACK_UTILS_H
//==============================================================================
typedef union
{
  int  i;
  char buff[sizeof(int)];
} intUnion;
//==============================================================================
typedef union
{
  float f;
  char  buff[sizeof(float)];
} floatUnion;
//==============================================================================
unsigned short getCRC16(char *data, unsigned short length);
//==============================================================================
int bytes_to_int    (char *bytes, int   *value);
int bytes_to_float  (char *bytes, float *value);
//==============================================================================
int bytes_from_int  (char *bytes, int   value);
int bytes_from_float(char *bytes, float value);
//==============================================================================
#endif //PACK_UTILS_H
