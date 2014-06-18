/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qcore_unix_p.h"
#include "qelapsedtimer.h"

#ifdef Q_OS_NACL

#else
# if !defined(Q_OS_HPUX) || defined(__ia64)
#  include <sys/select.h>
# endif
# include <sys/time.h>
#endif

#include <stdlib.h>

#ifdef Q_OS_MAC
#include <mach/mach_time.h>
#endif

QT_BEGIN_NAMESPACE

static inline bool time_update(struct timeval *tv, const struct timeval &start,
                               const struct timeval &timeout)
{
   // clock source is (hopefully) monotonic, so we can recalculate how much timeout is left;
   // if it isn't monotonic, we'll simply hope that it hasn't jumped, because we have no alternative
   struct timeval now = qt_gettime();
   *tv = timeout + start - now;
   return tv->tv_sec >= 0;
}

int qt_safe_select(int nfds, fd_set *fdread, fd_set *fdwrite, fd_set *fdexcept,
                   const struct timeval *orig_timeout)
{
   if (!orig_timeout) {
      // no timeout -> block forever
      int ret;
      EINTR_LOOP(ret, select(nfds, fdread, fdwrite, fdexcept, 0));
      return ret;
   }

   timeval start = qt_gettime();
   timeval timeout = *orig_timeout;

   // loop and recalculate the timeout as needed
   int ret;
   forever {
      ret = ::select(nfds, fdread, fdwrite, fdexcept, &timeout);
      if (ret != -1 || errno != EINTR)
      {
         return ret;
      }

      // recalculate the timeout
      if (!time_update(&timeout, start, *orig_timeout))
      {
         // timeout during update
         // or clock reset, fake timeout error
         return 0;
      }
   }
}

QT_END_NAMESPACE
