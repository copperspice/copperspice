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

#ifndef QABSTRACTEVENTDISPATCHER_P_H
#define QABSTRACTEVENTDISPATCHER_P_H

#include <QtCore/qabstracteventdispatcher.h>

QT_BEGIN_NAMESPACE

Q_CORE_EXPORT uint qGlobalPostedEventsCount();

class Q_CORE_EXPORT QAbstractEventDispatcherPrivate
{
   Q_DECLARE_PUBLIC(QAbstractEventDispatcher)

 public:
   inline QAbstractEventDispatcherPrivate() : event_filter(0) { }
   virtual ~QAbstractEventDispatcherPrivate() {}

   void init();
   QAbstractEventDispatcher::EventFilter event_filter;

   static int allocateTimerId();
   static void releaseTimerId(int id);

 protected:
   QAbstractEventDispatcher *q_ptr;

};

QT_END_NAMESPACE

#endif // QABSTRACTEVENTDISPATCHER_P_H
