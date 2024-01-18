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

#ifndef QXCB_EVENTDISPATCHER_GLIB_P_H
#define QXCB_EVENTDISPATCHER_GLIB_P_H

#include <qeventdispatcher_glib_p.h>

using GMainContext = struct _GMainContext;

class QXcbEventDispatcherGlibPrivate;
struct GUserEventSource;

class QXcbEventDispatcherGlib : public QEventDispatcherGlib
{
   CS_OBJECT(QXcbEventDispatcherGlib)

 public:
   explicit QXcbEventDispatcherGlib(QObject *parent = nullptr);
   ~QXcbEventDispatcherGlib();

   bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
   QEventLoop::ProcessEventsFlags m_flags;

 private:
   Q_DECLARE_PRIVATE(QXcbEventDispatcherGlib)
};

class QXcbEventDispatcherGlibPrivate : public QEventDispatcherGlibPrivate
{
   Q_DECLARE_PUBLIC(QXcbEventDispatcherGlib)

 public:
   QXcbEventDispatcherGlibPrivate(GMainContext *context = nullptr);
   GUserEventSource *userEventSource;
};

#endif