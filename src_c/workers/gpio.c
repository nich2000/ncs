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
#define PIN_SPEED_1  RPI_GPIO_P1_21
#define PIN_DIR_1    RPI_GPIO_P1_22
#define PIN_SPEED_2  RPI_GPIO_P1_23
#define PIN_DIR_2    RPI_GPIO_P1_24
//==============================================================================
#define DIR_STOP     "stop"
#define DIR_FORWARD  "forward"
#define DIR_BACKWARD "backward"
#define DIR_LEFT     "left"
#define DIR_RIGHT    "right"
//==============================================================================
static int _speed_1 = 0;
static int _dir_1   = 0;
static int _speed_2 = 0;
static int _dir_2   = 0;
//==============================================================================
int gpio(char *direction)
{
  log_add_fmt(LOG_INFO, "gpio, direction: %s", direction);

  if(strcmp(direction, DIR_STOP) == 0)
  {
    _speed_1 = LOW;
    _dir_1   = LOW;
    _speed_2 = LOW;
    _dir_2   = LOW;
  }
  else if(strcmp(direction, DIR_FORWARD) == 0)
  {
    _speed_1 = HIGH;
    _dir_1   = HIGH;
    _speed_2 = HIGH;
    _dir_2   = HIGH;
  }
  else if(strcmp(direction, DIR_BACKWARD) == 0)
  {
    _speed_1 = HIGH;
    _dir_1   = LOW;
    _speed_2 = HIGH;
    _dir_2   = LOW;
  }
  else if(strcmp(direction, DIR_LEFT) == 0)
  {
    _speed_1 = HIGH;
    _dir_1   = HIGH;
    _speed_2 = LOW;
    _dir_2   = LOW;
  }
  else if(strcmp(direction, DIR_RIGHT) == 0)
  {
    _speed_1 = LOW;
    _dir_1   = LOW;
    _speed_2 = HIGH;
    _dir_2   = HIGH;
  }

  #ifdef PI_DEVICE
  if(!bcm2835_init())
    return make_last_error(ERROR_NORMAL, errno, "gpio, bcm2835_init fails");

  bcm2835_gpio_fsel(PIN_SPEED_1, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(PIN_DIR_1,   BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(PIN_SPEED_2, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(PIN_DIR_2,   BCM2835_GPIO_FSEL_OUTP);

  bcm2835_gpio_write(PIN_SPEED_1, _speed_1);
  bcm2835_gpio_write(PIN_DIR_1,   _dir_1);
  bcm2835_gpio_write(PIN_SPEED_2, _speed_2);
  bcm2835_gpio_write(PIN_DIR_2,   _dir_2);

  bcm2835_close();
  #endif

  return ERROR_NONE;
}
//==============================================================================
