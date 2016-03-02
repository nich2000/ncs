/*
 * gps_parse.h
 *
 *  Created on: 29-01-2012
 *  Author: andy
 *  Edit: NIch 08-10-2015
 */
//==============================================================================
#ifndef GPS_PARSE_H
#define GPS_PARSE_H
//==============================================================================
/*
NMEA 0183 version 3.00
$GPRMC,hhmmss.sss,A,GGMM.MM   ,P,gggmm.mm   ,J,v.v  ,b.b   ,ddmmyy,x.x,n  ,m*hh<CR><LF>
$GPRMC,065315.080,A,5535.26112,N,03753.79745,E,5.527,264.98,081015,_._,_._,A *61
       1         2 3          4 5           6 7     8      9      10  11  12 13
1  = UTC time of fix
2  = Data status (A=Valid position, V=navigation receiver warning)
3  = Latitude of fix
4  = N or S of longitude
5  = Longitude of fix
6  = E or W of longitude
7  = Speed over ground in knots
8  = Track made good in degrees True
9  = UTC date of fix
10 = Magnetic variation degrees (Easterly var. subtracts from true course)
11 = E or W of magnetic variation
12 = Mode indicator, (A=Autonomous, D=Differential, E=Estimated, N=Data not valid)
13 = Checksum
*/
//==============================================================================
#include "defines.h"
//==============================================================================
typedef struct
{
  // 1
  unsigned char time_gps[10];
  int           time;
  int           time_s;
  // 2
  unsigned char data_valid[1];
  //3
  unsigned char lat_gps[10];
  double        lat;
  // 4
  unsigned char N_S[1];
  // 5
  unsigned char long_gps[11];
  double        lon;
  // 6
  unsigned char E_W[1];
  // 7
  unsigned char speed_knots[6];
  double        speed;
  // 8
  unsigned char true_course[6];
  double        course;
  // 9
  unsigned char date_gps[6];
  // 10
  unsigned char magnetic[6];
  // 11
  unsigned char E_W_magnetic[1];
  // 12
  unsigned char mode[1];
  // 13
  unsigned char check_summ[2];
} GPRMC_t;
//==============================================================================
void     gps_init();
int      gps_parse_str(unsigned char *str);
GPRMC_t *gps_data();
//==============================================================================
#endif /* GPS_PARSE_H */
