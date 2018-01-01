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

#ifndef QFunctions_NACL_H
#define QFunctions_NACL_H

#ifdef Q_OS_NACL

#include <sys/types.h>

// pthread
#include <pthread.h>
#define PTHREAD_CANCEL_DISABLE 1
#define PTHREAD_CANCEL_ENABLE  2
#define PTHREAD_INHERIT_SCHED  3

QT_BEGIN_NAMESPACE

extern "C" {

   void pthread_cleanup_push(void (*handler)(void *), void *arg);
   void pthread_cleanup_pop(int execute);

   int pthread_setcancelstate(int state, int *oldstate);
   int pthread_setcanceltype(int type, int *oldtype);
   void pthread_testcancel(void);
   int pthread_cancel(pthread_t thread);

   int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
   int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched);

   // event dispatcher, select
   //struct fd_set;
   //struct timeval;
   int fcntl(int fildes, int cmd, ...);
   int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);

   typedef long off64_t;
   off64_t ftello64(void *stream);
   off64_t lseek64(int fildes, off_t offset, int whence);
   int open64(const char *path, int oflag, ...);

}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timespec *timeout);

QT_END_NAMESPACE

#endif //Q_OS_NACL

#endif //QNACLUNIMPLEMENTED_H
