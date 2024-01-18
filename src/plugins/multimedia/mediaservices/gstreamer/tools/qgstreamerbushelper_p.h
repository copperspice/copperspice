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

#ifndef QGSTREAMERBUSHELPER_P_H
#define QGSTREAMERBUSHELPER_P_H

#include <qobject.h>
#include <qgstreamermessage_p.h>

#include <gst/gst.h>

class QGstreamerSyncMessageFilter
{
 public:
   //returns true if message was processed and should be dropped, false otherwise
   virtual bool processSyncMessage(const QGstreamerMessage &message) = 0;
};

#define QGstreamerSyncMessageFilter_iid "com.copperspice.CS.gstreamerSyncMessageFilter/1.0"
CS_DECLARE_INTERFACE(QGstreamerSyncMessageFilter, QGstreamerSyncMessageFilter_iid)


class QGstreamerBusMessageFilter
{
 public:
   //returns true if message was processed and should be dropped, false otherwise
   virtual bool processBusMessage(const QGstreamerMessage &message) = 0;
};

#define QGstreamerBusMessageFilter_iid "com.copperspice.CS.gstreamerBusMessagefilter/1.0"
CS_DECLARE_INTERFACE(QGstreamerBusMessageFilter, QGstreamerBusMessageFilter_iid)


class QGstreamerBusHelperPrivate;

class QGstreamerBusHelper : public QObject
{
   CS_OBJECT(QGstreamerBusHelper)
   friend class QGstreamerBusHelperPrivate;

 public:
   QGstreamerBusHelper(GstBus *bus, QObject *parent = nullptr);
   ~QGstreamerBusHelper();

   void installMessageFilter(QObject *filter);
   void removeMessageFilter(QObject *filter);

   CS_SIGNAL_1(Public, void message(QGstreamerMessage const &msg))
   CS_SIGNAL_2(message, msg)

 private:
   QGstreamerBusHelperPrivate *d;
};

#endif
