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

#include <qxcb_eventdispatcher_glib_p.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qplatformdefs.h>

#include <qapplication_p.h>

#include <glib.h>

struct GUserEventSource {
   GSource source;
   QXcbEventDispatcherGlib *q;
};

static gboolean userEventSourcePrepare(GSource *s, gint *timeout)
{
   return QWindowSystemInterface::windowSystemEventsQueued() > 0;
}

static gboolean userEventSourceCheck(GSource *source)
{
   return userEventSourcePrepare(source, 0);
}

static gboolean userEventSourceDispatch(GSource *source, GSourceFunc, gpointer)
{
   GUserEventSource *userEventSource   = reinterpret_cast<GUserEventSource *>(source);
   QXcbEventDispatcherGlib *dispatcher = userEventSource->q;
   QWindowSystemInterface::sendWindowSystemEvents(dispatcher->m_flags);
   return true;
}

static GSourceFuncs userEventSourceFuncs = {
   userEventSourcePrepare,
   userEventSourceCheck,
   userEventSourceDispatch,
   NULL,
   NULL,
   NULL
};

QXcbEventDispatcherGlibPrivate::QXcbEventDispatcherGlibPrivate(GMainContext *context)
   : QEventDispatcherGlibPrivate(context)
{
   Q_Q(QXcbEventDispatcherGlib);
   userEventSource = reinterpret_cast<GUserEventSource *>(g_source_new(&userEventSourceFuncs, sizeof(GUserEventSource)));
   userEventSource->q = q;

   g_source_set_can_recurse(&userEventSource->source, true);
   g_source_attach(&userEventSource->source, mainContext);
}

QXcbEventDispatcherGlib::QXcbEventDispatcherGlib(QObject *parent)
   : QEventDispatcherGlib(*new QXcbEventDispatcherGlibPrivate, parent), m_flags(QEventLoop::AllEvents)
{
   Q_D(QXcbEventDispatcherGlib);
   d->userEventSource->q = this;
}

QXcbEventDispatcherGlib::~QXcbEventDispatcherGlib()
{
   Q_D(QXcbEventDispatcherGlib);

   g_source_destroy(&d->userEventSource->source);
   g_source_unref(&d->userEventSource->source);
   d->userEventSource = 0;
}

bool QXcbEventDispatcherGlib::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   m_flags = flags;
   return QEventDispatcherGlib::processEvents(m_flags);
}

