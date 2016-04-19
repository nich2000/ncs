#-------------------------------------------------------------------------------
# NIch CLient Server Project
# Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
#
# File: CMakeLists.txt
#-------------------------------------------------------------------------------
# https://cmake.org/Wiki/CMake_FAQ
# http://www.vtk.org/Wiki/CMake_Useful_Variables
# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
# http://knzsoft.blogspot.ru/2012/04/cmakelisttxt.html
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)
#-------------------------------------------------------------------------------
include_directories(../libs/iniparser/src)
include_directories(../libs/jansson/src)
include_directories(../libs/AVRCryptoLib)
include_directories(./utilites)
include_directories(./workers)
include_directories(./)
#-------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin/lib)
set(CMAKE_C_FLAGS "-Wall")
set(BUILD_SHARED_LIBS OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)
IF (WIN32)
  set(CMAKE_C_COMPILER "c:/MinGW/bin/gcc.exe")
  set(CMAKE_EXE_LINKER_FLAGS "-static")
ELSE()
ENDIF()
#-------------------------------------------------------------------------------
project(ncs C)
#-------------------------------------------------------------------------------
add_definitions(-DTURN_ON_STDCALL)
#-------------------------------------------------------------------------------
find_package(Threads REQUIRED)
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
#...............................................................................
#add_library          (jansson SHARED ${_JANSSON})
add_library          (jansson STATIC ${_JANSSON})
set_target_properties(jansson PROPERTIES C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_CRYPTO   ../libs/AVRCryptoLib/sha1.c
              ../libs/AVRCryptoLib/base64.c)
#...............................................................................
#add_library          (crypto SHARED ${_CRYPTO})
add_library          (crypto STATIC ${_CRYPTO})
set_target_properties(crypto PROPERTIES C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_INI      ../libs/iniparser/src/iniparser.c
              ../libs/iniparser/src/dictionary.c)
#...............................................................................
add_library          (ini SHARED ${_INI})
#add_library          (ini STATIC ${_INI})
set_target_properties(ini PROPERTIES C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_LOG      defines.h
              globals.h
              utilites/ncs_log.h
              utilites/ncs_log.c)
#...............................................................................
add_library          (log SHARED ${_LOG})
#add_library          (log STATIC ${_LOG})
set_target_properties(log PROPERTIES C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_ERROR    defines.h
              globals.h
              utilites/ncs_error.h
              utilites/ncs_error.c)
#...............................................................................
add_library          (error SHARED ${_ERROR})
#add_library          (error STATIC ${_ERROR})
set_target_properties(error PROPERTIES C_STANDARD 99)
target_link_libraries(error log)
#-------------------------------------------------------------------------------
set(_PACK     defines.h
              globals.h
              utilites/ncs_pack_types.h
              utilites/ncs_pack.h
              utilites/ncs_pack.c
              utilites/ncs_pack_utils.h
              utilites/ncs_pack_utils.c)
#...............................................................................
add_library          (pack SHARED ${_PACK})
#add_library          (pack STATIC ${_PACK})
set_target_properties(pack PROPERTIES C_STANDARD 99)
target_link_libraries(pack error log)
#-------------------------------------------------------------------------------
set(_PROTOCOL defines.h
              globals.h
              globals.c
              utilites/protocol_types.h
              utilites/protocol.h
              utilites/protocol.c
              utilites/protocol_utils.h
              utilites/protocol_utils.c)
#...............................................................................
#add_library          (protocol SHARED ${_PROTOCOL})
add_library          (protocol STATIC ${_PROTOCOL})
set_target_properties(protocol PROPERTIES C_STANDARD 99)
target_link_libraries(protocol error log pack)
#-------------------------------------------------------------------------------
set(_UTILS    defines.h
              globals.h
              utils.h
              utils.c
              test.h
              test.c)
#...............................................................................
#add_library          (utils SHARED ${_UTILS})
add_library          (utils STATIC ${_UTILS})
set_target_properties(utils PROPERTIES C_STANDARD 99)
target_link_libraries(utils log)
#-------------------------------------------------------------------------------
#set(_MAP      defines.h
#              globals.h
#              workers/map.h
#              workers/map.c)
##...............................................................................
##add_library          (map SHARED ${_MAP})
#add_library          (map STATIC ${_MAP})
#set_target_properties(map PROPERTIES C_STANDARD 99)
#target_link_libraries(map error log utils)
#-------------------------------------------------------------------------------
set(_SOCKET   defines.h
              globals.h
              globals.c
              workers/socket_types.h
              workers/worker_types.h
              workers/socket.c
              workers/socket_utils.c
              workers/streamer.c
              workers/customworker.c
              workers/cmdworker.c
              workers/webworker.c
              workers/wsworker.c
              workers/exec.c
              workers/map.h
              workers/map.c
              utilites/ncs_pack_types.h
              utilites/ncs_pack.h
              utilites/ncs_pack.c)
#...............................................................................
#add_library          (socket SHARED ${_SOCKET})
add_library          (socket STATIC ${_SOCKET})
set_target_properties(socket PROPERTIES C_STANDARD 99)
target_link_libraries(socket protocol log error utils jansson crypto Threads::Threads)
IF (WIN32)
target_link_libraries(socket wsock32 ws2_32)
ENDIF()
#-------------------------------------------------------------------------------
set(_PROG     defines.h
              globals.h
              main.h
              main.c)
#...............................................................................
add_executable       (${PROJECT_NAME} ${_PROG})
target_link_libraries(${PROJECT_NAME} log error ini socket)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C C_STANDARD 99)
#-------------------------------------------------------------------------------
