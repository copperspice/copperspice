/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qfunctions_nacl.h>
#include <pthread.h>
#include <qglobal.h>

/*
    The purpose of this file is to stub out certain functions
    that are not provided by the Native Client SDK. This is
    done as an alterative to sprinkling the Qt sources with
    NACL ifdefs.

    There are two main classes of functions:

    - Functions that are called but can have no effect:
    For these we simply give an empty implementation

    - Functions that are referenced in the source code, but
    is not/must not be called at run-time:
    These we either leave undefined or implement with a
    qFatal.

    This is a work in progress.
*/

extern "C" {

   void pthread_cleanup_push(void (*)(void *), void *)
   {

   }

   void pthread_cleanup_pop(int)
   {

   }

   int pthread_setcancelstate(int, int *)
   {
      return 0;
   }

   int pthread_setcanceltype(int, int *)
   {
      return 0;
   }

   void pthread_testcancel(void)
   {

   }


   int pthread_cancel(pthread_t)
   {
      return 0;
   }

   int pthread_attr_setinheritsched(pthread_attr_t *, int)
   {
      return 0;
   }


   int pthread_attr_getinheritsched(const pthread_attr_t *, int *)
   {
      return 0;
   }

   // event dispatcher, select
   //struct fd_set;
   //struct timeval;

   int fcntl(int, int, ...)
   {
      return 0;
   }

   int sigaction(int, const struct sigaction *, struct sigaction *)
   {
      return 0;
   }

   int open(const char *, int, ...)
   {
      return 0;
   }

   int open64(const char *, int, ...)
   {
      return 0;
   }

   int access(const char *, int)
   {
      return 0;
   }

   typedef long off64_t;
   off64_t ftello64(void *)
   {
      qFatal("ftello64 called");
      return 0;
   }

   off64_t lseek64(int, off_t, int)
   {
      qFatal("lseek64 called");
      return 0;
   }

} // Extern C

int select(int, fd_set *, fd_set *, fd_set *, struct timespec *)
{
   return 0;
}
