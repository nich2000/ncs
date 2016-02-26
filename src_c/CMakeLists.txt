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
include_directories(utilites)
include_directories(/)
#-------------------------------------------------------------------------------
set(_LOG defines.h utilites/log.c)
add_library(log SHARED ${_LOG})
set_property(TARGET log PROPERTY C_STANDARD 99)
#-------------------------------------------------------------------------------
set(_PROTOCOL defines.h utilites/protocol_types.h utilites/protocol.c utilites/protocol_utils.c test.c)
add_library(protocol SHARED ${_PROTOCOL})
set_property(TARGET protocol PROPERTY C_STANDARD 99)
target_link_libraries(protocol log)
#-------------------------------------------------------------------------------
set(_SOCKET defines.h utils.c socket_types.h socket.c socket_utils.c streamer.c customworker.c cmdworker.c webworker.c wsworker.c base64.c sha1.c)
add_library(socket SHARED ${_SOCKET})
set_property(TARGET socket PROPERTY C_STANDARD 99)
target_link_libraries(socket protocol log)
IF (WIN32)
  target_link_libraries(socket wsock32 ws2_32)
ENDIF()
#-------------------------------------------------------------------------------
set(_PROG main.c defines.h utils.c socket_utils.c exec.c streamer.c utilites/protocol_utils.c utilites/gps_parse.c)
#-------------------------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${_PROG})
target_link_libraries(${PROJECT_NAME} protocol log socket Threads::Threads)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C C_STANDARD 99)
#-------------------------------------------------------------------------------
