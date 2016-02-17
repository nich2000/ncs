#ifndef DEFINES_H
#define DEFINES_H

//#define SERVER_MODE
//#define CLIENT_MODE

#define SOCK_PACK_MODE
//#define SOCK_RANDOM_BUFFER

//#define PACK_USE_OWN_QUEUE

#if defined(__linux__) || defined(_WIN32)
#else
  #define PACK_USE_OWN_BUFFER
#endif

#endif // DEFINES
