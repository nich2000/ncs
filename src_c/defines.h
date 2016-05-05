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
#define USE_BINARY_PROTOCOL
#define DEBUG_MODE

#if defined(__linux__) || defined(_WIN32)
#else
  #define DEMS_DEVICE
#endif

#define PI_DEVICE

#ifdef DEMS_DEVICE
  #define PACK_USE_OWN_BUFFER
#else
  // Flag block or allow writes to file
//  #define WRITE_STAT
  #define WRITE_SESSION
//  #define WRITE_REPORT

  // send stream data to ws
  #define STREAM_TO_WS

  // create random pack on test stream
//  #define STREAM_RANDOM_PACK

  // logs manage
//  #define USE_EXTRA_LOGS
//  #define PRINT_RESV_PACK
//  #define PRINT_SND_PACK
//  #define PRINT_ALL_INFO
//  #define SILENT_MODE

  // own queue or not
  #define PACK_USE_OWN_QUEUE

  // debug and test
//  #define SOCK_RANDOM_BUFFER

//  #define SERVER_MODE
//  #define CLIENT_MODE
#endif
//==============================================================================
#endif //DEFINES_H
