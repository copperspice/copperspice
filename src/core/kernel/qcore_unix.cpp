/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qcore_unix_p.h>

#include <qelapsedtimer.h>

#include <stdlib.h>

#ifdef Q_OS_NACL

#else
# if ! defined(Q_OS_HPUX) || defined(__ia64)
#  include <sys/select.h>
# endif

# include <sys/time.h>
#endif

#ifdef Q_OS_DARWIN
#include <mach/mach_time.h>
#endif

static inline bool time_update(struct timespec *tv, const struct timespec &start,
      const struct timespec &timeout)
{
   // clock source is (hopefully) monotonic, so we can recalculate how much timeout is left;
   // if it isn't monotonic, we'll simply hope that it hasn't jumped, because we have no alternative
   struct timespec now = qt_gettime();
   *tv = timeout + start - now;
   return tv->tv_sec >= 0;
}

int qt_safe_select(int nfds, fd_set *fdread, fd_set *fdwrite, fd_set *fdexcept,
      const struct timespec *orig_timeout)
{
   if (! orig_timeout) {
      // no timeout -> block forever
      int ret;
      EINTR_LOOP(ret, select(nfds, fdread, fdwrite, fdexcept, nullptr));
      return ret;
   }

   timespec start   = qt_gettime();
   timespec timeout = *orig_timeout;

   // loop and recalculate the timeout as needed
   int ret;

   while (true) {

      ret = ::pselect(nfds, fdread, fdwrite, fdexcept, &timeout, nullptr);

      if (ret != -1 || errno != EINTR) {
         return ret;
      }

      // recalculate the timeout
      if (!time_update(&timeout, start, *orig_timeout))  {
         // timeout during update
         // or clock reset, fake timeout error
         return 0;
      }
   }
}

int qt_select_msecs(int nfds, fd_set *fdread, fd_set *fdwrite, int timeout)
{
   if (timeout < 0) {
      return qt_safe_select(nfds, fdread, fdwrite, nullptr, nullptr);
   }

   struct timespec tv;

   tv.tv_sec = timeout / 1000;
   tv.tv_nsec = (timeout % 1000) * 1000 * 1000;
   return qt_safe_select(nfds, fdread, fdwrite, nullptr, &tv);
}
