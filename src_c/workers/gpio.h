//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: gpio.h
 */
//==============================================================================
#ifndef GPIO_H
#define GPIO_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#ifdef PI_DEVICE
#include "bcm2835.h"
#endif
//==============================================================================
int gpio_init();
int gpio(char *direction);
int gpio_close();
//==============================================================================
#endif //GPIO_H
