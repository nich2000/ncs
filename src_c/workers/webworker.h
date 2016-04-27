//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: webworker.h
 */
//==============================================================================
#ifndef WEBWORKER_H
#define WEBWORKER_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "customworker.h"
//==============================================================================
typedef struct
{
  custom_server_t custom_server;
}web_server_t;
//==============================================================================
int web_server(sock_state_t state, sock_port_t port);
int web_server_status();
//==============================================================================
#endif //WEBWORKER_H
