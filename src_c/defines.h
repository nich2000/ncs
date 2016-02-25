#ifndef DEFINES_H
#define DEFINES_H

#if defined(__linux__) || defined(_WIN32)
#else
  #define DEMS_DEVICE
#endif

#define DEBUG_MODE
//#DEFINE SILENT_MODE

//#define SERVER_MODE
//#define CLIENT_MODE

//#define SOCK_EXTRA_LOGS
#define SOCK_PACK_MODE
//#define SOCK_RANDOM_BUFFER
//#define SOCK_USE_FREE_INDEX

#ifdef DEMS_DEVICE
  #define PACK_USE_OWN_BUFFER
#else
  #define PACK_USE_OWN_QUEUE
#endif

#endif // DEFINES
