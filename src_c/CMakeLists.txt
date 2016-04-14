#-------------------------------------------------------------------------------
# NIch CLient Server Project
# Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
#
# <filename>
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)
#-------------------------------------------------------------------------------
project(ncs C)
#-------------------------------------------------------------------------------
IF (WIN32)
  # set(CMAKE_CC_COMPILER "c:/MinGW/bin/gcc.exe")
ELSE()
  # set stuff for other systems
ENDIF()
#-------------------------------------------------------------------------------
add_definitions(-DTURN_ON_STDCALL)
#-------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#-------------------------------------------------------------------------------
# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
set(CMAKE_C_FLAGS "-Wall")
#-------------------------------------------------------------------------------
#set(CMAKE_EXE_LINKER_FLAGS  "-static")
#-------------------------------------------------------------------------------
include_directories(../libs/iniparser/src)
include_directories(../libs/jansson/src)
include_directories(../libs/AVRCryptoLib)
include_directories(./utilites)
include_directories(./workers)
include_directories(./)
#-------------------------------------------------------------------------------
set(_INI      ../libs/iniparser/src/iniparser.c
              ../libs/iniparser/src/dictionary.c)
add_library(ini SHARED ${_INI})
set_property(TARGET ini PROPERTY C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_JANSSON  ../libs/jansson/src/dump.c
              ../libs/jansson/src/error.c
              ../libs/jansson/src/hashtable.c
              ../libs/jansson/src/hashtable_seed.c
              ../libs/jansson/src/load.c
              ../libs/jansson/src/memory.c
              ../libs/jansson/src/pack_unpack.c
              ../libs/jansson/src/strbuffer.c
              ../libs/jansson/src/strconv.c
              ../libs/jansson/src/utf.c
              ../libs/jansson/src/value.c)
add_library(jansson SHARED ${_JANSSON})
set_property(TARGET jansson PROPERTY C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_CRYPTO   ../libs/AVRCryptoLib/sha1.c
              ../libs/AVRCryptoLib/base64.c)
add_library(crypto SHARED ${_CRYPTO})
set_property(TARGET crypto PROPERTY C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_LOG      defines.h
              globals.h
              utilites/ncs_log.c)
add_library(log SHARED ${_LOG})
set_property(TARGET log PROPERTY C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_ERROR    defines.h
              globals.h
              utilites/ncs_error.c)
add_library(error SHARED ${_ERROR})
set_property(TARGET error PROPERTY C_STANDARD 99)
target_link_libraries(error log)
#-------------------------------------------------------------------------------
set(_PACK     defines.h
              globals.h
              utilites/ncs_pack_types.h
              utilites/ncs_pack.c
              utilites/ncs_pack_utils.c
              utilites/ncs_error.c)
add_library(pack SHARED ${_PACK})
set_property(TARGET pack PROPERTY C_STANDARD 99)
target_link_libraries(pack log error)
#-------------------------------------------------------------------------------
set(_PROTOCOL defines.h
              globals.h
              globals.c
              utilites/protocol_types.h
              utilites/protocol.c
              utilites/protocol_utils.c)
add_library(protocol SHARED ${_PROTOCOL})
set_property(TARGET protocol PROPERTY C_STANDARD 99)
target_link_libraries(protocol log error pack)
#-------------------------------------------------------------------------------
set(_SOCKET   defines.h
              globals.h
              globals.c
              utils.c
              test.c
              workers/exec.c
              workers/socket_types.h
              workers/socket.c
              workers/socket_utils.c
              workers/streamer.c
              workers/worker_types.h
              workers/customworker.c
              workers/cmdworker.c
              workers/webworker.c
              workers/wsworker.c)
add_library(socket SHARED ${_SOCKET})
set_property(TARGET socket PROPERTY C_STANDARD 99)
target_link_libraries(socket protocol log error jansson crypto)
IF (WIN32)
  target_link_libraries(socket wsock32 ws2_32)
ENDIF()
#-------------------------------------------------------------------------------
set(_PROG     defines.h
              globals.h
              main.c
              utils.c
              workers/socket_utils.c
              workers/exec.c
              workers/streamer.c
              utilites/protocol_utils.c)
#-------------------------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${_PROG})
target_link_libraries(${PROJECT_NAME} ini protocol log socket error Threads::Threads)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C C_STANDARD 99)
#-------------------------------------------------------------------------------
