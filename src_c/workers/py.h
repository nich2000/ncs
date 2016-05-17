//==============================================================================
/**
 * NIch CLient Server Project
 * Copyright 2016 NIch(nich2000@mail.ru) All rights reserved
 *
 * File: py.h
 */
//==============================================================================
#ifndef PY_H
#define PY_H
//==============================================================================
// https://github.com/shadowmint/cmake-python-embed-nostdlib
//==============================================================================
#include "defines.h"
#include "globals.h"

#ifdef USE_PYTHON
#include <python2.7/Python.h>
#endif
//==============================================================================
int py_init();
int py_simple();
int py_func();
int py_main();
int py_final();
//==============================================================================
#endif //PY_H
