#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)
#-------------------------------------------------------------------------------
project(SocketTestC C)
#-------------------------------------------------------------------------------
IF (WIN32)
  set(CMAKE_CC_COMPILER "c:/MinGW/bin/gcc.exe")
ELSE()
  # set stuff for other systems
ENDIF()
#-------------------------------------------------------------------------------
add_definitions(-DTURN_ON_STDCALL)
#-------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#-------------------------------------------------------------------------------
# Warnings settings
# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
#set(CMAKE_C_FLAGS "-pedantic")
#-------------------------------------------------------------------------------
set(CMAKE_EXE_LINKER_FLAGS  "-static")
#-------------------------------------------------------------------------------
include_directories(../libs/jansson/src)
include_directories(./utilites)
include_directories(./workers)
include_directories(./)
#-------------------------------------------------------------------------------
set(_LOG defines.h globals.h utilites/ncs_log.c)
add_library(log SHARED ${_LOG})
set_property(TARGET log PROPERTY C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_ERROR defines.h globals.h utilites/ncs_error.c)
add_library(error SHARED ${_ERROR})
set_property(TARGET error PROPERTY C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_JANSSON ../libs/jansson/src/dump.c ../libs/jansson/src/error.c ../libs/jansson/src/hashtable.c ../libs/jansson/src/hashtable_seed.c ../libs/jansson/src/load.c ../libs/jansson/src/memory.c ../libs/jansson/src/pack_unpack.c ../libs/jansson/src/strbuffer.c ../libs/jansson/src/strconv.c ../libs/jansson/src/utf.c ../libs/jansson/src/value.c)
add_library(jansson SHARED ${_JANSSON})
set_property(TARGET jansson PROPERTY C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_PROTOCOL defines.h globals.h utilites/protocol_types.h utilites/protocol.c utilites/protocol_utils.c test.c)
add_library(protocol SHARED ${_PROTOCOL})
set_property(TARGET protocol PROPERTY C_STANDARD 99)
target_link_libraries(protocol log error)
#-------------------------------------------------------------------------------
set(_SOCKET defines.h globals.h utils.c workers/socket_types.h workers/socket.c workers/socket_utils.c workers/streamer.c workers/customworker.c workers/cmdworker.c workers/webworker.c workers/wsworker.c workers/base64.c workers/sha1.c)
add_library(socket SHARED ${_SOCKET})
set_property(TARGET socket PROPERTY C_STANDARD 99)
target_link_libraries(socket protocol jansson log error)
IF (WIN32)
  target_link_libraries(socket wsock32 ws2_32)
ENDIF()
#-------------------------------------------------------------------------------
set(_PROG main.c defines.h globals.h utils.c workers/socket_utils.c workers/exec.c workers/streamer.c utilites/protocol_utils.c utilites/gps_parse.c)
#-------------------------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${_PROG})
target_link_libraries(${PROJECT_NAME} protocol log socket error jansson Threads::Threads)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C C_STANDARD 99)
#-------------------------------------------------------------------------------
