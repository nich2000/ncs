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
#define CMD_HELP                "help"           // 0 - 1(all, specifically)
#define CMD_VERSION             "version"        // 0
#define CMD_TEST                "test"           // 0
#define CMD_CLEAR               "clear"          // 0
#define CMD_EXIT                "exit"           // 0
#define CMD_ALL                 "all"            // 0 start all default servers
#define CMD_SERVER              "server"         // 1 - 2(state, port)
#define CMD_WEB_SERVER          "webserver"      // 1 - 2(state, port)
#define CMD_WS_SERVER           "wsserver"       // 1 - 2(state, port)
#define CMD_CLIENT              "client"         // 1 - 3(state, host, port)
#define CMD_SND_TO_SERVER       "sndtosr"        // 1 - n
#define CMD_SND_TO_WSSERVER     "sndtows"        // 1 - n
#define CMD_SND_TO_CLIENT       "sndtocl"        // 1 - n
#define CMD_STREAM              "stream"         // 1(on off pause resume)
#define CMD_TYPES_INFO          "typesinfo"      // 0
#define CMD_DEFINES_INFO        "definesinfo"    // 0
#define CMD_SERVER_INFO         "serverinfo"     // 0
#define CMD_WEB_SERVER_INFO     "webserverinfo"  // 0
#define CMD_WS_SERVER_INFO      "wsserverinfo"   // 0
#define CMD_CLIENT_INFO         "clientinfo"     // 0
#define CMD_CMD_REGISTER        "register"       //
#define CMD_CMD_ACTIVATE        "activate"       //
#define CMD_WS_REGISTER         "ws_register"    //
#define CMD_WS_ACTIVATE         "ws_activate"    //
#define CMD_CONFIG              "config"         // 0
#define CMD_RECONFIG            "reconfig"       // 0
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
#define EXEC_UNKNOWN            -1
#define EXEC_NONE                0
#define EXEC_DONE                1
//==============================================================================
#define MAX_COMMAND_SIZE         128
//==============================================================================
typedef int (*exec_func)(int, ...);
//==============================================================================
int  read_config();
//==============================================================================
int handle_command_str    (void *sender, char *command);
int handle_command_str_fmt(void *sender, char *command, ...);
int handle_command_pack   (void *sender, pack_packet_t *packet);
//==============================================================================
#endif //EXEC_H
