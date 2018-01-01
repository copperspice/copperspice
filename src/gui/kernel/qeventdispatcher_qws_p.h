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

#ifndef QEVENTDISPATCHER_QWS_P_H
#define QEVENTDISPATCHER_QWS_P_H

#include <qeventdispatcher_unix_p.h>

QT_BEGIN_NAMESPACE

class QEventDispatcherQWSPrivate;

class QEventDispatcherQWS : public QEventDispatcherUNIX
{
   GUI_CS_OBJECT(QEventDispatcherQWS)
   Q_DECLARE_PRIVATE(QEventDispatcherQWS)

 public:
   explicit QEventDispatcherQWS(QObject *parent = nullptr);
   ~QEventDispatcherQWS();

   bool processEvents(QEventLoop::ProcessEventsFlags flags);
   bool hasPendingEvents();

   void flush();

   void startingUp();
   void closingDown();

 protected:
   int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, timeval *timeout);
};

QT_END_NAMESPACE

#endif // QEVENTDISPATCHER_QWS_P_H
