//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: py.c
 */
//==============================================================================
#include "py.h"
#include "ncs_error.h"
#include "ncs_log.h"
//==============================================================================
int py_init()
{
  #ifdef USE_PYTHON
  Py_Initialize();
  #endif

  return ERROR_NONE;
}
//==============================================================================
int py_simple()
{
  #ifdef USE_PYTHON
  // PyRun_SimpleString(code);
  // PyRun_SimpleFile(file);
  #endif

  return ERROR_NONE;
}
//==============================================================================
int py_func()
{
  #ifdef USE_PYTHON
  #endif

  return ERROR_NONE;
}
//==============================================================================
int py_main()
{
  #ifdef USE_PYTHON
  Py_Main(0, 0);
  #endif

  return ERROR_NONE;
}
//==============================================================================
int py_final()
{
  #ifdef USE_PYTHON
  Py_Finalize();
  #endif

  return ERROR_NONE;
}
//==============================================================================
