#ifndef DEFINES_H
#define DEFINES_H

#define DEBUG_MODE

//#define SERVER_MODE
//#define CLIENT_MODE

#define SOCK_EXTRA_LOGS
#define SOCK_PACK_MODE
//#define SOCK_RANDOM_BUFFER

// if do not use own queue, sender will send last packet allways
//#define PACK_USE_OWN_QUEUE

#if defined(__linux__) || defined(_WIN32)
#else
  #define PACK_USE_OWN_BUFFER
#endif

#endif // DEFINES
