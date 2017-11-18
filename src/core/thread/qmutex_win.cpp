/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <qt_windows.h>
#include <qatomic.h>
#include <qlog.h>
#include <qmutex.h>
#include <qmutex_p.h>

QMutexPrivate::QMutexPrivate()
{
   event = CreateEvent(0, FALSE, FALSE, 0);
   if (!event) {
      qWarning("QMutexData::QMutexData: Can not create event");
   }
}

QMutexPrivate::~QMutexPrivate()
{
   CloseHandle(event);
}

bool QMutexPrivate::wait(int timeout)
{
   return (WaitForSingleObject(event, timeout < 0 ? INFINITE : timeout) ==  WAIT_OBJECT_0);
}

void QMutexPrivate::wakeUp()
{
   SetEvent(event);
}

