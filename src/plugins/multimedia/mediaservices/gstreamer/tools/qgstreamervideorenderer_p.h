/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QGSTREAMERVIDEORENDERER_H
#define QGSTREAMERVIDEORENDERER_H

#include <qvideorenderercontrol.h>
#include <qvideosurfacegstsink_p.h>
#include <qabstractvideosurface.h>

#include <qgstreamervideorendererinterface_p.h>

class QGstreamerVideoRenderer : public QVideoRendererControl, public QGstreamerVideoRendererInterface
{
   CS_OBJECT_MULTIPLE(QGstreamerVideoRenderer, QVideoRendererControl)
   CS_INTERFACES(QGstreamerVideoRendererInterface)

 public:
   QGstreamerVideoRenderer(QObject *parent = nullptr);
   virtual ~QGstreamerVideoRenderer();

   QAbstractVideoSurface *surface() const override;
   void setSurface(QAbstractVideoSurface *surface) override;

   GstElement *videoSink() override;

   void stopRenderer() override;
   bool isReady() const override {
      return m_surface != 0;
   }

   CS_SIGNAL_1(Public, void sinkChanged())
   CS_SIGNAL_2(sinkChanged)

   CS_SIGNAL_1(Public, void readyChanged(bool un_named_arg1))
   CS_SIGNAL_2(readyChanged, un_named_arg1)

 private:
   CS_SLOT_1(Private, void handleFormatChange())
   CS_SLOT_2(handleFormatChange)

   QVideoSurfaceGstSink *m_videoSink;
   QPointer<QAbstractVideoSurface> m_surface;
};

#endif
