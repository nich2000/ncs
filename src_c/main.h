//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: main.h
 */
//==============================================================================
#ifndef MAIN_H
#define MAIN_H
//==============================================================================
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "defines.h"
#include "globals.h"

#ifdef PI_DEVICE
#include "gpio.h"
#endif

#include "socket.h"
#include "ncs_log.h"
#include "ncs_error.h"
#include "exec.h"
#include "streamer.h"
#include "utils.h"
//==============================================================================
#endif //MAIN_H
