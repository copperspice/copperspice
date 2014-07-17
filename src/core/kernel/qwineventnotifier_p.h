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

#ifndef QWINEVENTNOTIFIER_P_H
#define QWINEVENTNOTIFIER_P_H

#include <QtCore/qobject.h>
#include <QtCore/qt_windows.h>

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QWinEventNotifier : public QObject
{
   CS_OBJECT(QWinEventNotifier)

 public:
   explicit QWinEventNotifier(QObject *parent = 0);
   explicit QWinEventNotifier(HANDLE hEvent, QObject *parent = 0);
   ~QWinEventNotifier();

   void setHandle(HANDLE hEvent);
   HANDLE handle() const;

   bool isEnabled() const;

   CORE_CS_SLOT_1(Public, void setEnabled(bool enable))
   CORE_CS_SLOT_2(setEnabled)

   CORE_CS_SIGNAL_1(Public, void activated(HANDLE hEvent))
   CORE_CS_SIGNAL_2(activated, hEvent)

 protected:
   bool event(QEvent *e);

 private:
   Q_DISABLE_COPY(QWinEventNotifier)

   HANDLE handleToEvent;
   bool enabled;
};

QT_END_NAMESPACE

#endif // QWINEVENTNOTIFIER_P_H
