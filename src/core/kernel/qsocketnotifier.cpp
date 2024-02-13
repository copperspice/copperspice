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

#include <qsocketnotifier.h>

#include <qabstracteventdispatcher.h>
#include <qcoreapplication.h>
#include <qplatformdefs.h>

#include <qthread_p.h>

QSocketNotifier::QSocketNotifier(qintptr socket, Type type, QObject *parent)
   : QObject(parent)
{
   sockfd    = socket;
   sntype    = type;
   snenabled = true;

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);
   auto tmp = threadData->eventDispatcher.load();

   if (socket < 0) {
      qWarning("QSocketNotifier() Invalid socket specified");

   } else if (! tmp) {
      qWarning("QSocketNotifier() Can only be used with threads started with QThread");

   } else {
      tmp->registerSocketNotifier(this);
   }
}

QSocketNotifier::~QSocketNotifier()
{
   setEnabled(false);
}

qintptr QSocketNotifier::socket() const
{

   return sockfd;
}

QSocketNotifier::Type QSocketNotifier::type() const
{

   return sntype;
}

bool QSocketNotifier::isEnabled() const
{

   return snenabled;
}

void QSocketNotifier::setEnabled(bool enable)
{
   if (sockfd < 0) {
      return;
   }

   if (snenabled == enable) {                      // no change
      return;
   }

   snenabled = enable;

   //
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);
   auto tmp = threadData->eventDispatcher.load();

   if (! tmp) {
      // perhaps application/thread is shutting down
      return;
   }

   if (thread() != QThread::currentThread()) {
      qWarning("QSocketNotifier::setEnabled() Socket notifiers can not be enabled or disabled from another thread");
      return;
   }

   if (snenabled) {
      tmp->registerSocketNotifier(this);

   } else {
      tmp->unregisterSocketNotifier(this);
   }
}

bool QSocketNotifier::event(QEvent *e)
{
   // Emits the activated() signal when a QEvent::SockAct is received.

   if (e->type() == QEvent::ThreadChange) {
      if (snenabled) {
         QMetaObject::invokeMethod(this, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, snenabled));
         setEnabled(false);
      }
   }

   QObject::event(e);                        // will activate filters

   if ((e->type() == QEvent::SockAct) || (e->type() == QEvent::SockClose)) {
      emit activated(sockfd);
      return true;
   }

   return false;
}
