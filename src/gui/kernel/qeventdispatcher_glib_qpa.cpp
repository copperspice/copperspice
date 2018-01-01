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

#include <qeventdispatcher_glib_qpa_p.h>
#include <qapplication.h>
#include <qplatformdefs.h>
#include <qapplication.h>

#include <glib.h>
#include <qapplication_p.h>

#include <qdebug.h>

QT_BEGIN_NAMESPACE

struct GUserEventSource {
   GSource source;
   QPAEventDispatcherGlib *q;
};

static gboolean userEventSourcePrepare(GSource *s, gint *timeout)
{
   Q_UNUSED(s)
   Q_UNUSED(timeout)

   return QWindowSystemInterfacePrivate::windowSystemEventsQueued() > 0;
}

static gboolean userEventSourceCheck(GSource *source)
{
   return userEventSourcePrepare(source, 0);
}

static gboolean userEventSourceDispatch(GSource *s, GSourceFunc, gpointer)
{
   GUserEventSource *source = reinterpret_cast<GUserEventSource *>(s);

   QWindowSystemInterfacePrivate::WindowSystemEvent *event;
   while (QWindowSystemInterfacePrivate::windowSystemEventsQueued()) {
      event = QWindowSystemInterfacePrivate::getWindowSystemEvent();
      if (!event) {
         break;
      }

      // send through event filter
      if (source->q->filterEvent(event)) {
         delete event;
         continue;
      }
      QApplicationPrivate::processWindowSystemEvent(event);
      delete event;
   }

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

QPAEventDispatcherGlibPrivate::QPAEventDispatcherGlibPrivate(GMainContext *context)
   : QEventDispatcherGlibPrivate(context)
{
   userEventSource = reinterpret_cast<GUserEventSource *>(g_source_new(&userEventSourceFuncs,
                     sizeof(GUserEventSource)));
   userEventSource->q = 0;
   g_source_set_can_recurse(&userEventSource->source, true);
   g_source_attach(&userEventSource->source, mainContext);
}


QPAEventDispatcherGlib::QPAEventDispatcherGlib(QObject *parent)
   : QEventDispatcherGlib(*new QPAEventDispatcherGlibPrivate, parent)
{
   Q_D(QPAEventDispatcherGlib);
   d->userEventSource->q = this;
}

QPAEventDispatcherGlib::~QPAEventDispatcherGlib()
{
   Q_D(QPAEventDispatcherGlib);

   g_source_destroy(&d->userEventSource->source);
   g_source_unref(&d->userEventSource->source);
   d->userEventSource = 0;
}

bool QPAEventDispatcherGlib::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   static bool init = false;
   if (!init) {
      if (QApplicationPrivate::platformIntegration()->createEventLoopIntegration()) {
         qWarning("Eventloop integration is not supported by the glib event dispatcher");
         qWarning("Use the UNIX event dispatcher by defining environment variable QT_NO_GLIB=1");
      }
      init = true;
   }
   return QEventDispatcherGlib::processEvents(flags);
}

QT_END_NAMESPACE
