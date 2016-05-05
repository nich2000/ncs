//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: gpio.c
 */
//==============================================================================
#include <string.h>

#include "gpio.h"

#include "bcm2835.h"

#include "ncs_error.h"
#include "ncs_log.h"
//==============================================================================
#define PIN_SPEED_L  RPI_GPIO_P1_21
#define PIN_DIR_L    RPI_GPIO_P1_22
#define PIN_SPEED_R  RPI_GPIO_P1_23
#define PIN_DIR_R    RPI_GPIO_P1_24
//==============================================================================
#define DIR_STOP     "stop"
#define DIR_FORWARD  "forward"
#define DIR_BACKWARD "backward"
#define DIR_LEFT     "left"
#define DIR_RIGHT    "right"
//==============================================================================
static int _speed_l = 0;
static int _dir_l   = 0;
static int _speed_r = 0;
static int _dir_r   = 0;
//==============================================================================
int gpio_init()
{
  #ifdef PI_DEVICE
  if(!bcm2835_init())
    return make_last_error(ERROR_NORMAL, errno, "gpio, bcm2835_init fails");

  bcm2835_gpio_fsel(PIN_SPEED_L, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(PIN_DIR_L,   BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(PIN_SPEED_R, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(PIN_DIR_R,   BCM2835_GPIO_FSEL_OUTP);
  #endif

  return ERROR_NONE;
}
//==============================================================================
int gpio(char *direction)
{
  log_add_fmt(LOG_INFO, "gpio, direction: %s", direction);

  if(strcmp(direction, DIR_STOP) == 0)
  {
    _speed_l = LOW;
    _dir_l   = LOW;
    _speed_r = LOW;
    _dir_r   = LOW;
  }
  else if(strcmp(direction, DIR_FORWARD) == 0)
  {
    _speed_l = HIGH;
    _dir_l   = HIGH;
    _speed_r = HIGH;
    _dir_r   = HIGH;
  }
  else if(strcmp(direction, DIR_BACKWARD) == 0)
  {
    _speed_l = HIGH;
    _dir_l   = LOW;
    _speed_r = HIGH;
    _dir_r   = LOW;
  }
  else if(strcmp(direction, DIR_LEFT) == 0)
  {
    _speed_l = LOW;
    _dir_l   = LOW;
    _speed_r = HIGH;
    _dir_r   = HIGH;
  }
  else if(strcmp(direction, DIR_RIGHT) == 0)
  {
    _speed_l = HIGH;
    _dir_l   = HIGH;
    _speed_r = LOW;
    _dir_r   = LOW;
  }

  log_add_fmt(LOG_INFO, "gpio, SL:%d-%d   DL:%d-%d   SR:%d-%d   DR:%d-%d",
              PIN_SPEED_L, _speed_l,
              PIN_DIR_L,   _dir_l,
              PIN_SPEED_R, _speed_r,
              PIN_DIR_R,   _dir_r);

  #ifdef PI_DEVICE
  bcm2835_gpio_write(PIN_SPEED_L, _speed_l);
  bcm2835_gpio_write(PIN_DIR_L,   _dir_l);
  bcm2835_gpio_write(PIN_SPEED_R, _speed_r);
  bcm2835_gpio_write(PIN_DIR_R,   _dir_r);
  #endif

  return ERROR_NONE;
}
//==============================================================================
int gpio_close()
{
  #ifdef PI_DEVICE
  bcm2835_close();
  #endif

  return ERROR_NONE;
}
//==============================================================================
