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

#ifndef QEVENTDISPATCHER_GLIB_QPA_P_H
#define QEVENTDISPATCHER_GLIB_QPA_P_H

#include <qeventdispatcher_glib_p.h>

typedef struct _GMainContext GMainContext;

QT_BEGIN_NAMESPACE
class QPAEventDispatcherGlibPrivate;

class QPAEventDispatcherGlib : public QEventDispatcherGlib
{
   GUI_CS_OBJECT(QPAEventDispatcherGlib)
   Q_DECLARE_PRIVATE(QPAEventDispatcherGlib)

 public:
   explicit QPAEventDispatcherGlib(QObject *parent = nullptr);
   ~QPAEventDispatcherGlib();

   bool processEvents(QEventLoop::ProcessEventsFlags flags);
};

struct GUserEventSource;

class QPAEventDispatcherGlibPrivate : public QEventDispatcherGlibPrivate
{
   Q_DECLARE_PUBLIC(QPAEventDispatcherGlib)

 public:
   QPAEventDispatcherGlibPrivate(GMainContext *context = 0);
   GUserEventSource *userEventSource;
};


QT_END_NAMESPACE

#endif // QEVENTDISPATCHER_GLIB_QPA_P_H
