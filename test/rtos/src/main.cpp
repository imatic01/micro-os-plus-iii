/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2016 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <cmsis-plus/rtos/os.h>

#include <cstdio>

#include <test-cpp-api.h>
#include <test-c-api.h>
#include <test-iso-api.h>

#include <test-cpp-mem.h>

int
os_main (int argc __attribute__((unused)), char* argv[] __attribute__((unused)))
{
  printf ("\nµOS++ RTOS simple APIs test.\n");
#if defined(__clang__)
  printf ("Built with clang " __VERSION__ ".\n");
#else
  printf ("Built with GCC " __VERSION__ ".\n");
#endif

  int ret = 0;

#if 0
  if (ret == 0)
    {
      ret = test_cpp_mem ();
    }
#endif

#if 1
  if (ret == 0)
    {
      ret = test_cpp_api ();
    }
#endif

#if 1
  if (ret == 0)
    {
      ret = test_c_api ();
    }
#endif

#if 1
  if (ret == 0)
    {
      ret = test_iso_api (false);
    }
#endif

  printf ("errno=%d\n", errno);

  return ret;
}

// ----------------------------------------------------------------------------
