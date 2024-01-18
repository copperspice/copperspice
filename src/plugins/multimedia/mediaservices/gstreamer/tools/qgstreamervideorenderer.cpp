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

#include <qgstreamervideorenderer_p.h>
#include <qvideosurfacegstsink_p.h>
#include <qgstutils_p.h>
#include <qabstractvideosurface.h>
#include <qdebug.h>

#include <gst/gst.h>

QGstreamerVideoRenderer::QGstreamerVideoRenderer(QObject *parent)
   : QVideoRendererControl(parent), m_videoSink(nullptr), m_surface(nullptr)
{
}

QGstreamerVideoRenderer::~QGstreamerVideoRenderer()
{
   if (m_videoSink) {
      gst_object_unref(GST_OBJECT(m_videoSink));
   }
}

GstElement *QGstreamerVideoRenderer::videoSink()
{
   if (!m_videoSink && m_surface) {
      m_videoSink = QVideoSurfaceGstSink::createSink(m_surface);
      qt_gst_object_ref_sink(GST_OBJECT(m_videoSink)); //Take ownership
   }

   return reinterpret_cast<GstElement *>(m_videoSink);
}

void QGstreamerVideoRenderer::stopRenderer()
{
   if (m_surface) {
      m_surface->stop();
   }
}

QAbstractVideoSurface *QGstreamerVideoRenderer::surface() const
{
   return m_surface;
}

void QGstreamerVideoRenderer::setSurface(QAbstractVideoSurface *surface)
{
   if (m_surface != surface) {
      //qDebug() << Q_FUNC_INFO << surface;
      if (m_videoSink) {
         gst_object_unref(GST_OBJECT(m_videoSink));
      }

      m_videoSink = nullptr;

      if (m_surface) {
         disconnect(m_surface.data(), SIGNAL(supportedFormatsChanged()), this, SLOT(handleFormatChange()));
      }

      bool wasReady = isReady();

      m_surface = surface;

      if (m_surface) {
         connect(m_surface.data(), SIGNAL(supportedFormatsChanged()), this, SLOT(handleFormatChange()));
      }

      if (wasReady != isReady()) {
         emit readyChanged(isReady());
      }

      emit sinkChanged();
   }
}

void QGstreamerVideoRenderer::handleFormatChange()
{
   //qDebug() << "Supported formats list has changed, reload video output";

   if (m_videoSink) {
      gst_object_unref(GST_OBJECT(m_videoSink));
   }

   m_videoSink = nullptr;
   emit sinkChanged();
}
