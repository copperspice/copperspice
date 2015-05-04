/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QFUTUREWATCHER_P_H
#define QFUTUREWATCHER_P_H

#include <qfutureinterface_p.h>
#include <qlist.h>

QT_BEGIN_NAMESPACE

class QFutureWatcherBase;

class QFutureWatcherBasePrivate : public QFutureCallOutInterface
{
   Q_DECLARE_PUBLIC(QFutureWatcherBase)

 public:
   QFutureWatcherBasePrivate();
   virtual ~QFutureWatcherBasePrivate() {}

   void postCallOutEvent(const QFutureCallOutEvent &callOutEvent);
   void callOutInterfaceDisconnected();

   void sendCallOutEvent(QFutureCallOutEvent *event);

   QList<QFutureCallOutEvent *> pendingCallOutEvents;
   QAtomicInt pendingResultsReady;
   int maximumPendingResultsReady;

   QAtomicInt resultAtConnected;
   bool finished;

 protected:
   QFutureWatcherBase *q_ptr;

};

QT_END_NAMESPACE

#endif
