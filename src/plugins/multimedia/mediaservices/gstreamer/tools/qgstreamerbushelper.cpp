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

#include <qmap.h>
#include <qtimer.h>
#include <qmutex.h>
#include <qlist.h>
#include <qabstracteventdispatcher.h>
#include <qcoreapplication.h>

#include <qgstreamerbushelper_p.h>

class QGstreamerBusHelperPrivate : public QObject
{
   CS_OBJECT(QGstreamerBusHelperPrivate)

 public:
   QGstreamerBusHelperPrivate(QGstreamerBusHelper *parent, GstBus *bus) :
      QObject(parent), m_tag(0), m_bus(bus), m_helper(parent), m_intervalTimer(nullptr) {

      // glib event loop can be disabled either by env variable or QT_NO_GLIB define, so check the dispacher
      QAbstractEventDispatcher *dispatcher = QCoreApplication::eventDispatcher();
      const bool hasGlib = dispatcher && dispatcher->inherits("QEventDispatcherGlib");

      if (! hasGlib) {
         m_intervalTimer = new QTimer(this);
         m_intervalTimer->setInterval(250);
         connect(m_intervalTimer, &QTimer::timeout, this, &QGstreamerBusHelperPrivate::interval);
         m_intervalTimer->start();

      } else {
         m_tag = gst_bus_add_watch_full(bus, G_PRIORITY_DEFAULT, busCallback, this, nullptr);
      }
   }

   ~QGstreamerBusHelperPrivate() {
      m_helper = nullptr;
      delete m_intervalTimer;

      if (m_tag) {
         g_source_remove(m_tag);
      }
   }

   GstBus *bus() const {
      return m_bus;
   }

   QMutex filterMutex;
   QList<QGstreamerSyncMessageFilter *> syncFilters;
   QList<QGstreamerBusMessageFilter *> busFilters;

 private:
   void processMessage(GstMessage *message) {
      QGstreamerMessage msg(message);
      doProcessMessage(msg);
   }

   void queueMessage(GstMessage *message) {
      QGstreamerMessage msg(message);
      QMetaObject::invokeMethod(this, "doProcessMessage", Qt::QueuedConnection, Q_ARG(const QGstreamerMessage &, msg));
   }

   static gboolean busCallback(GstBus *bus, GstMessage *message, gpointer data) {
      (void) bus;
      reinterpret_cast<QGstreamerBusHelperPrivate *>(data)->queueMessage(message);

      return TRUE;
   }

   guint m_tag;
   GstBus *m_bus;
   QGstreamerBusHelper *m_helper;
   QTimer *m_intervalTimer;

   CS_SLOT_1(Private, void doProcessMessage(const QGstreamerMessage &msg))
   CS_SLOT_2(doProcessMessage)

   CS_SLOT_1(Private, void interval())
   CS_SLOT_2(interval)
};


void QGstreamerBusHelperPrivate::doProcessMessage(const QGstreamerMessage &msg)
{
   for (QGstreamerBusMessageFilter *filter : busFilters) {
      if (filter->processBusMessage(msg)) {
         break;
      }
   }

   emit m_helper->message(msg);
}

void QGstreamerBusHelperPrivate::interval()
{
   GstMessage *message;

   while ((message = gst_bus_poll(m_bus, GST_MESSAGE_ANY, 0)) != nullptr) {
      processMessage(message);
      gst_message_unref(message);
   }
}

static GstBusSyncReply syncGstBusFilter(GstBus *bus, GstMessage *message, QGstreamerBusHelperPrivate *d)
{
   (void) bus;

   QMutexLocker lock(&d->filterMutex);

   for (QGstreamerSyncMessageFilter *filter : d->syncFilters) {
      if (filter->processSyncMessage(QGstreamerMessage(message))) {
         return GST_BUS_DROP;
      }
   }

   return GST_BUS_PASS;
}

QGstreamerBusHelper::QGstreamerBusHelper(GstBus *bus, QObject *parent)
   : QObject(parent)
{
   d = new QGstreamerBusHelperPrivate(this, bus);

#if GST_CHECK_VERSION(1,0,0)
   gst_bus_set_sync_handler(bus, (GstBusSyncHandler)syncGstBusFilter, d, nullptr);
#else
   gst_bus_set_sync_handler(bus, (GstBusSyncHandler)syncGstBusFilter, d);
#endif

   gst_object_ref(GST_OBJECT(bus));
}

QGstreamerBusHelper::~QGstreamerBusHelper()
{
#if GST_CHECK_VERSION(1,0,0)
   gst_bus_set_sync_handler(d->bus(), nullptr, nullptr, nullptr);
#else
   gst_bus_set_sync_handler(d->bus(), nullptr, nullptr);
#endif
   gst_object_unref(GST_OBJECT(d->bus()));
}

void QGstreamerBusHelper::installMessageFilter(QObject *filter)
{
   QGstreamerSyncMessageFilter *syncFilter = dynamic_cast<QGstreamerSyncMessageFilter *>(filter);
   if (syncFilter) {
      QMutexLocker lock(&d->filterMutex);
      if (!d->syncFilters.contains(syncFilter)) {
         d->syncFilters.append(syncFilter);
      }
   }

   QGstreamerBusMessageFilter *busFilter = dynamic_cast<QGstreamerBusMessageFilter *>(filter);
   if (busFilter && !d->busFilters.contains(busFilter)) {
      d->busFilters.append(busFilter);
   }
}

void QGstreamerBusHelper::removeMessageFilter(QObject *filter)
{
   QGstreamerSyncMessageFilter *syncFilter = dynamic_cast<QGstreamerSyncMessageFilter *>(filter);
   if (syncFilter) {
      QMutexLocker lock(&d->filterMutex);
      d->syncFilters.removeAll(syncFilter);
   }

   QGstreamerBusMessageFilter *busFilter = dynamic_cast<QGstreamerBusMessageFilter *>(filter);
   if (busFilter) {
      d->busFilters.removeAll(busFilter);
   }
}


