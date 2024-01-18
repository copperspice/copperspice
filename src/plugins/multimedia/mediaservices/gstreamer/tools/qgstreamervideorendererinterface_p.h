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

#ifndef QGSTREAMERVIDEOOUTPUTCONTROL_H
#define QGSTREAMERVIDEOOUTPUTCONTROL_H

#include <qobject.h>

#include <gst/gst.h>

class QGstreamerVideoRendererInterface
{
 public:
   virtual ~QGstreamerVideoRendererInterface();
   virtual GstElement *videoSink() = 0;

   // stopRenderer() is called when the renderer element is stopped.
   // it can be reimplemented when video renderer can't detect
   // changes to NULL state but has to free video resources.
   virtual void stopRenderer() {}

   // the video output is configured, usually after the first paint event
   // (winId is known,
   virtual bool isReady() const {
      return true;
   }

   // signals:
   // void sinkChanged();
   // void readyChanged(bool);
};

#define QGstreamerVideoRendererInterface_iid "com.copperspice.CS.gstreamerVideoRenderer/1.0"
CS_DECLARE_INTERFACE(QGstreamerVideoRendererInterface, QGstreamerVideoRendererInterface_iid)

#endif
