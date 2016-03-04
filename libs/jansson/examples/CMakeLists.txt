#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)
#-------------------------------------------------------------------------------
project(JanssonTestC C)
#-------------------------------------------------------------------------------
IF (WIN32)
  set(CMAKE_CC_COMPILER "c:/MinGW/bin/gcc.exe")
ELSE()
  # set stuff for other systems
ENDIF()
#-------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
#-------------------------------------------------------------------------------
include_directories(../src)
include_directories(/)
#-------------------------------------------------------------------------------
set(_PROG simple_parse.c ../src/dump.c ../src/error.c ../src/hashtable.c ../src/hashtable_seed.c ../src/load.c ../src/memory.c ../src/pack_unpack.c ../src/strbuffer.c ../src/strconv.c ../src/utf.c ../src/value.c)
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${_PROG})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C C_STANDARD 99)
#-------------------------------------------------------------------------------
