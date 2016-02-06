#ifndef UTILS_H
#define UTILS_H
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
int bytes_to_int  (unsigned char *bytes, int size, int   *value);
int bytes_to_float(unsigned char *bytes, int size, float *value);
//==============================================================================
//int bytes_from_int  (unsigned char *bytes, size_t size, int   *value);
//int bytes_from_float(unsigned char *bytes, size_t size, float *value);
//==============================================================================
unsigned short getCRC16(char *data_p, unsigned short length);
//==============================================================================
#endif //UTILS_H
