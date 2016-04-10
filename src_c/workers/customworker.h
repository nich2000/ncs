//==============================================================================
/*
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * <filename>
*/
//==============================================================================
#ifndef CUSTOMWORKER_H
#define CUSTOMWORKER_H
//==============================================================================
#include <stdio.h>
#include <pthread.h>

#include "defines.h"
#include "globals.h"

#include "protocol_types.h"
#include "worker_types.h"
#include "ncs_error.h"
//==============================================================================
#define ID_GEN_NEW            -1
#define ID_DEFAULT             0
//==============================================================================
#define STATIC_CMD_SERVER_ID   1
#define STATIC_WEB_SERVER_ID   2
#define STATIC_WS_SERVER_ID    3
//==============================================================================
#define STATIC_CMD_SERVER_NAME "CMD_SERVER\0"
#define STATIC_WEB_SERVER_NAME "WEB_SERVER\0"
#define STATIC_WS_SERVER_NAME  "WS_SERVER\0"
//==============================================================================
int custom_worker_init             (int id, custom_worker_t *worker);
int custom_worker_stop             (        custom_worker_t *worker);

int custom_server_init             (int id, custom_server_t *custom_server);
int custom_server_start            (custom_worker_t *worker);
int custom_server_work             (custom_server_t *server);

int custom_remote_client_init      (int id, custom_remote_client_t *custom_remote_client);
int custom_remote_client_deinit    (custom_remote_client_t *custom_remote_client);

int custom_client_init             (custom_client_t *custom_client);
int custom_client_start            (custom_worker_t *worker);
int custom_client_work             (custom_client_t *client);

int                      custom_remote_clients_init (custom_remote_clients_list_t *clients_list);
int                     _custom_remote_clients_count(custom_remote_clients_list_t *clients_list);
custom_remote_client_t *_custom_remote_clients_next (custom_remote_clients_list_t *clients_list);

void *custom_recv_worker(void *arg);
//==============================================================================
#endif //CUSTOMWORKER_H
