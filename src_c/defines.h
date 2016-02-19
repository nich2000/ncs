#ifndef DEFINES_H
#define DEFINES_H

#define DEBUG_MODE
//#DEFINE SILENT_MODE

//#define SERVER_MODE
//#define CLIENT_MODE

//#define SOCK_EXTRA_LOGS
#define SOCK_PACK_MODE
//#define SOCK_RANDOM_BUFFER

#ifdef DEMS_DEVICE
  #define PACK_USE_OWN_BUFFER
#else
  #define PACK_USE_OWN_QUEUE
#endif

#endif // DEFINES
