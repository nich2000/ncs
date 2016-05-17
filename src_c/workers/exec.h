//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: exec.h
 */
//==============================================================================
#ifndef EXEC_H
#define EXEC_H
//==============================================================================
#include "defines.h"
#include "globals.h"

#include "iniparser.h"
#include "dictionary.h"

#include "ncs_pack.h"
//==============================================================================
#define CMD_S_HELP              "-H"
#define CMD_S_CONFIG            "-C"
#define CMD_S_VERSION           "-V"
#define CMD_S_ALL               "-a"
#define CMD_S_SERVER            "-s"
#define CMD_S_WEB_SERVER        "-w"
#define CMD_S_WS_SERVER         "-s"
#define CMD_S_CLIENT            "-c"
#define PARAM_S_PORT            "-p"
#define PARAM_S_WEB_PORT        "-r"
#define PARAM_S_WS_PORT         "-t"
#define PARAM_S_HOST            "-h"
#define PARAM_S_COUNT           "-n"
//==============================================================================
#define CMD_HELP                "--help"
#define CMD_CONFIG              "--config"
#define CMD_VERSION             "--version"
#define CMD_ALL                 "--all"
#define CMD_SERVER              "--server"
#define CMD_WEB_SERVER          "--webserver"
#define CMD_WS_SERVER           "--wsserver"
#define CMD_CLIENT              "--client"
#define PARAM_PORT              "--port"
#define PARAM_WEB_PORT          "--webport"
#define PARAM_WS_PORT           "--wsport"
#define PARAM_HOST              "--host"
#define PARAM_COUNT             "--count"
//==============================================================================
#define CMD_TEST                "test"
#define CMD_CLEAR               "clear"
#define CMD_EXIT                "exit"
#define CMD_SND_TO_SERVER       "sndtosr"
#define CMD_SND_TO_WSSERVER     "sndtows"
#define CMD_SND_TO_CLIENT       "sndtocl"
#define CMD_STREAM              "stream"
#define CMD_TYPES_INFO          "typesinfo"
#define CMD_DEFINES_INFO        "definesinfo"
#define CMD_SERVER_INFO         "serverinfo"
#define CMD_WEB_SERVER_INFO     "webserverinfo"
#define CMD_WS_SERVER_INFO      "wsserverinfo"
#define CMD_CLIENT_INFO         "clientinfo"
#define CMD_CMD_REGISTER        "register"
#define CMD_CMD_ACTIVATE        "activate"
#define CMD_WS_REGISTER         "ws_register"
#define CMD_WS_ACTIVATE         "ws_activate"
#define CMD_RECONFIG            "reconfig"
#define CMD_GPIO                "gpio"
#define CMD_PY                  "py"
//==============================================================================
#define CMD_START               "on"
#define CMD_STOP                "off"
#define CMD_PAUSE               "pause"
#define CMD_RESUME              "resume"
#define CMD_STEP                "step"
//==============================================================================
#define CMD_FIRST               "first"
#define CMD_SECOND              "second"
#define CMD_NEXT                "next"
//==============================================================================
#define CMP_PY_SIMPLE           "simple"
#define CMP_PY_FUNC             "func"
#define CMP_PY_MAIN             "main"
//==============================================================================
#define EXEC_UNKNOWN            -1
#define EXEC_NONE                0
#define EXEC_DONE                1
//==============================================================================
#define COMMAND_SIZE            128
#define HISTORY_COUNT           1024
//==============================================================================
typedef char command_t[COMMAND_SIZE];
//==============================================================================
typedef command_t history_items_t[HISTORY_COUNT];
//==============================================================================
typedef struct
{
  int             count;
  history_items_t items;
} history_t;
//==============================================================================
history_t *history();
//==============================================================================
typedef int (*exec_func)(int, ...);
//==============================================================================
int read_config();
//==============================================================================
int history_load();
void history_add(command_t command);
const char* history_prev();
const char* history_next();
//==============================================================================
int handle_command_str    (void *sender, char *command);
int handle_command_str_fmt(void *sender, char *command, ...);
int handle_command_pack   (void *sender, pack_packet_t *packet);
int handle_command_ajax   (void *sender, char *command);
//==============================================================================
#endif //EXEC_H
