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

#ifndef QWINEVENTNOTIFIER_H
#define QWINEVENTNOTIFIER_H

#include <qobject.h>
#include <qt_windows.h>

#ifdef Q_OS_WIN

class Q_CORE_EXPORT QWinEventNotifier : public QObject
{
   CORE_CS_OBJECT(QWinEventNotifier)

 public:
   explicit QWinEventNotifier(QObject *parent = nullptr);
   explicit QWinEventNotifier(HANDLE hEvent, QObject *parent = nullptr);
   ~QWinEventNotifier();

   void setHandle(HANDLE hEvent);
   HANDLE handle() const;

   bool isEnabled() const;

   CORE_CS_SIGNAL_1(Public, void activated(HANDLE hEvent))
   CORE_CS_SIGNAL_2(activated, hEvent)

   CORE_CS_SLOT_1(Public, void setEnabled(bool enable))
   CORE_CS_SLOT_2(setEnabled)

 protected:
   bool event(QEvent *event) override;

 private:
   HANDLE handleToEvent;
   bool enabled;
};

#endif // Q_OS_WIN

#endif
