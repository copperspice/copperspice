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

#ifndef QWSSIGNALHANDLER_P_H
#define QWSSIGNALHANDLER_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_QWS_SIGNALHANDLER

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qobjectcleanuphandler.h>

QT_BEGIN_NAMESPACE

typedef void (*qt_sighandler_t)(int);

class QLock;
class QWSLock;

class QWSSignalHandlerPrivate;

class Q_GUI_EXPORT QWSSignalHandler
{
 public:
   static QWSSignalHandler *instance();

   ~QWSSignalHandler();

#ifndef QT_NO_QWS_MULTIPROCESS
   inline void addLock(QLock *lock) {
      locks.append(lock);
   }
   inline void removeLock(QLock *lock) {
      locks.removeOne(lock);
   }
   inline void addWSLock(QWSLock *wslock) {
      wslocks.append(wslock);
   }
   inline void removeWSLock(QWSLock *wslock) {
      wslocks.removeOne(wslock);
   }

#endif
   inline void addObject(QObject *object) {
      (void)objects.add(object);
   }

 private:
   QWSSignalHandler();

   void clear();

   static void handleSignal(int signal);

   QHash<int, qt_sighandler_t> oldHandlers;

#ifndef QT_NO_QWS_MULTIPROCESS
   QList<QLock *> locks;
   QList<QWSLock *> wslocks;
#endif

   QObjectCleanupHandler objects;

   friend class QWSSignalHandlerPrivate;
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_SIGNALHANDLER

#endif // QWSSIGNALHANDLER_P_H
