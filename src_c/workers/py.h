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
// http://www.linuxjournal.com/article/8497
// http://www.linuxjournal.com/article/3641
// http://www.codeproject.com/Articles/11805/Embedding-Python-in-C-C-Part-I
// https://docs.python.org/2/extending/index.html
// https://docs.python.org/2/c-api/index.html#c-api-index
// https://docs.python.org/2/c-api/veryhigh.html?highlight=py_main
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
