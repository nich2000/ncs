/*
 * gps_parse.c
 *
 *  Created on: 29-01-2012
 *  Author: andy
 *  Edit: NIch 08-10-2015
 */

#include <string.h>
#include <stdlib.h>

#include "gps_parse.h"
#include "protocol_utils.h"

uint8_t       *gprmc_token[13];
unsigned char  gprmc_str[128];
GPRMC_t        gprms_data;

void gps_init(void)
{
  gprmc_token[0]  = &gprms_data.time_gps[0];
  gprmc_token[1]  = &gprms_data.data_valid[0];
  gprmc_token[2]  = &gprms_data.lat_gps[0];
  gprmc_token[3]  = &gprms_data.N_S[0];
  gprmc_token[4]  = &gprms_data.long_gps[0];
  gprmc_token[5]  = &gprms_data.E_W[0];
  gprmc_token[6]  = &gprms_data.speed_knots[0];
  gprmc_token[7]  = &gprms_data.true_course[0];
  gprmc_token[8]  = &gprms_data.date_gps[0];
  gprmc_token[9]  = &gprms_data.magnetic[0];
  gprmc_token[10] = &gprms_data.E_W_magnetic[0];
  gprmc_token[11] = &gprms_data.mode[0];
  gprmc_token[12] = &gprms_data.check_summ[0];
}

int gps_parse_str(unsigned char *str)
{
  strcpy(gprmc_str, str);

  uint8_t new_gps_string    = 0;
  uint8_t count             = 0;
  uint8_t gprmc_ready_flag  = 0;
  uint8_t gprmc_parse_allow = 1;
  uint8_t gprmc_done_flag   = 0;
  uint8_t* ptr_temp         = NULL;

  // printf("gps_parse_str\n", str);
  while(*str)
  {
    // Search first simbol $
    // printf("%c", ch);
    unsigned char ch = *str;
    *str++;
    if(ch == '$' && !gprmc_ready_flag)
    {
      new_gps_string = 1;
      // printf(" new_gps_string\n");
      continue;
    }

    // If $ found = new string receive
    if(new_gps_string)
    {
      // Get and skip 5 simbols(GPRMC)
      if(!gprmc_ready_flag)
      {
        count++; // simbol counter
        // Maybe simbol is C(last from GPRMC)
        if(count == 5)
        {
          switch(ch)
          {
            case 'C':
              if(!gprmc_parse_allow)
                continue;
              // GPS string is GPRMC or GGGGC :-)
              gprmc_ready_flag = 1;
              count = 0;
              continue;
            default:
              // GPS string is not GPRMC, let`s search again
              new_gps_string = 0;
              count = 0;
              continue;
          }
        }
        else
          continue;
      }

      // GPS string is GPRMC
      if(gprmc_ready_flag)
      {
        // Delimeter or pre last token
        if(ch == ',' || (ch == '*' && count == 11))
        {
          count++;// token counter
          ptr_temp = gprmc_token[count - 1];
          continue;
        }

        if(count <= 9)
        {
          *ptr_temp = ch;
          ptr_temp++;
          continue;
        }
        else
        {
          new_gps_string = 0;
          gprmc_ready_flag = 0;
          gprmc_done_flag = 1;
          *ptr_temp = '\0';
          continue;
        }
      }
    }
  }
  // printf("\n");

  return gprmc_done_flag;
}

GPRMC_t *gps_data()
{
  gprms_data.time   = atof(gprms_data.time_gps);
  gprms_data.lat    = atof(gprms_data.lat_gps);
  gprms_data.lon    = atof(gprms_data.long_gps);
  gprms_data.speed  = atof(gprms_data.speed_knots);
  gprms_data.course = atof(gprms_data.true_course);

  return &gprms_data;
}
