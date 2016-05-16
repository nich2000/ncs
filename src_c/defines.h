//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: defines.h
 */
//==============================================================================
#ifndef DEFINES_H
#define DEFINES_H
//==============================================================================
// minimal configuration(sizes and counts)
#define SAFE_MODE
// maximum information about working process
#define DEBUG_MODE

#if defined(__linux__)
//  #define PI_DEVICE
#elif defined(_WIN32)
#else
  #define DEMS_DEVICE
#endif

#ifdef DEMS_DEVICE
  #define PACK_USE_OWN_BUFFER
#else
  // flag block or allow writes to file
//  #define WRITE_STAT
  #define WRITE_SESSION
//  #define WRITE_REPORT

  // logs manage
//  #define USE_EXTRA_LOGS
//  #define PRINT_RESV_PACK
//  #define PRINT_SND_PACK
//  #define PRINT_ALL_INFO
//  #define SILENT_MODE

  // own queue or not
  #define PACK_USE_OWN_QUEUE

  // send with random buffer
//  #define SOCK_RANDOM_BUFFER
//  #define SOCK_PART_SIZE 100

  // make random pack
//  #define STREAM_RANDOM_PACK

  // send stream data to ws
//  #define STREAM_TO_WS // deprecated
#endif

//#define USE_BINARY_PROTOCOL // deprecated
//==============================================================================
#endif //DEFINES_H
