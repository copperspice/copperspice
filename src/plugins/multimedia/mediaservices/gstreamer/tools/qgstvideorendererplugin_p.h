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

#ifndef QGSTVIDEORENDERERPLUGIN_P_H
#define QGSTVIDEORENDERERPLUGIN_P_H

#include <qobject.h>
#include <qplugin.h>
#include <qabstractvideobuffer.h>
#include <qvideosurfaceformat.h>

#include <gst/gst.h>

class QAbstractVideoSurface;

const QString QGstVideoRendererPluginKey("gstvideorenderer");

class QGstVideoRenderer
{
 public:
   virtual ~QGstVideoRenderer() {}

   virtual GstCaps *getCaps(QAbstractVideoSurface *surface) = 0;
   virtual bool start(QAbstractVideoSurface *surface, GstCaps *caps) = 0;
   virtual void stop(QAbstractVideoSurface *surface) = 0;  // surface may be null if unexpectedly deleted.
   virtual bool proposeAllocation(GstQuery *query) = 0;    // may be called from a thread.

   virtual bool present(QAbstractVideoSurface *surface, GstBuffer *buffer) = 0;
   virtual void flush(QAbstractVideoSurface *surface) = 0; // surface may be null if unexpectedly deleted.
};

class QGstVideoRendererInterface
{
 public:
   virtual ~QGstVideoRendererInterface() {}

   virtual QGstVideoRenderer *createRenderer() = 0;
};

#define QGstVideoRendererInterface_iid "com.copperspice.CS.gstVideoRenderer/1.0"
CS_DECLARE_INTERFACE(QGstVideoRendererInterface, QGstVideoRendererInterface_iid)

class QGstVideoRendererPlugin : public QObject, public QGstVideoRendererInterface
{
   CS_OBJECT(QGstVideoRendererPlugin)
   CS_INTERFACES(QGstVideoRendererInterface)

 public:
   explicit QGstVideoRendererPlugin(QObject *parent = nullptr);
   virtual ~QGstVideoRendererPlugin() {}
};

#endif
