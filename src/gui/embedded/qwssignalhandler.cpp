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

#include <qwssignalhandler_p.h>

#ifndef QT_NO_QWS_SIGNALHANDLER

#include <qlock_p.h>
#include <qwslock_p.h>
#include <sys/types.h>
#include <signal.h>

QT_BEGIN_NAMESPACE

class QWSSignalHandlerPrivate : public QWSSignalHandler
{
 public:
   QWSSignalHandlerPrivate() : QWSSignalHandler() {}
};


Q_GLOBAL_STATIC(QWSSignalHandlerPrivate, signalHandlerInstance);


QWSSignalHandler *QWSSignalHandler::instance()
{
   return signalHandlerInstance();
}

QWSSignalHandler::QWSSignalHandler()
{
   const int signums[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE,
                           SIGSEGV, SIGTERM, SIGBUS
                         };
   const int n = sizeof(signums) / sizeof(int);

   for (int i = 0; i < n; ++i) {
      const int signum = signums[i];
      qt_sighandler_t old = signal(signum, handleSignal);
      if (old == SIG_IGN) { // don't remove shm and semaphores when ignored
         signal(signum, old);
      } else {
         oldHandlers[signum] = (old == SIG_ERR ? SIG_DFL : old);
      }
   }
}

QWSSignalHandler::~QWSSignalHandler()
{
   clear();
}

void QWSSignalHandler::clear()
{
#if !defined(QT_NO_QWS_MULTIPROCESS)
   // it is safe to call d-tors directly here since, on normal exit,
   // lists should be empty; otherwise, we don't care about semi-alive objects
   // and the only important thing here is to unregister the system semaphores.
   while (!locks.isEmpty()) {
      locks.takeLast()->~QLock();
   }
   while (!wslocks.isEmpty()) {
      wslocks.takeLast()->~QWSLock();
   }
#endif
   objects.clear();
}

void QWSSignalHandler::handleSignal(int signum)
{
   QWSSignalHandler *h = instance();
   if (h) {
      signal(signum, h->oldHandlers[signum]);
      h->clear();
   }
   raise(signum);
}

QT_END_NAMESPACE

#endif // QT_NO_QWS_SIGNALHANDLER
