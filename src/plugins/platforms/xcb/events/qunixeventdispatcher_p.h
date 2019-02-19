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

#ifndef QUNIXEVENTDISPATCHER_P_H
#define QUNIXEVENTDISPATCHER_P_H

#include <qglobal.h>
#include <qeventdispatcher_unix_p.h>

class QUnixEventDispatcherPrivate;

class QUnixEventDispatcher : public QEventDispatcherUNIX
{
   CS_OBJECT(QUnixEventDispatcher)
   Q_DECLARE_PRIVATE(QUnixEventDispatcher)

 public:
   explicit QUnixEventDispatcher(QObject *parent = nullptr);
   ~QUnixEventDispatcher();

   bool processEvents(QEventLoop::ProcessEventsFlags flags);
   bool hasPendingEvents();

   void flush();
};

#endif
