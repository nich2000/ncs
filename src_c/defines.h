#ifndef DEFINES_H
#define DEFINES_H

//#define DEMS_DEVICE
#define USE_BINARY_PROTOCOL

#if defined(__linux__) || defined(_WIN32)
#else
  #define DEMS_DEVICE
#endif

#ifdef DEMS_DEVICE
  #define PACK_USE_OWN_BUFFER
#else
  #define PRINT_STREAMER_COUNTER
//  #define PRINT_RESV_PACK
//  #define PRINT_SND_PACK
//  #define PRINT_ALL_INFO

  #define PACK_USE_OWN_QUEUE

  //#define USE_EXTRA_LOGS
  #define SOCK_PACK_MODE
  //#define SOCK_RANDOM_BUFFER

  #define DEBUG_MODE

  //#define SILENT_MODE

  //#define SERVER_MODE
  //#define CLIENT_MODE
#endif

#endif //DEFINES_H
