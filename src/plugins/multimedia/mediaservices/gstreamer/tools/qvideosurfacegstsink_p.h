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

#ifndef QVIDEOSURFACEGSTSINK_P_H
#define QVIDEOSURFACEGSTSINK_P_H

#include <gst/gst.h>

#if GST_CHECK_VERSION(1,0,0)

#include <qgstvideorenderersink_p.h>
using QVideoSurfaceGstSink = QGstVideoRendererSink;

#else

#include <qlist.h>
#include <qmutex.h>
#include <qqueue.h>
#include <qpointer.h>
#include <qwaitcondition.h>
#include <qvideosurfaceformat.h>
#include <qvideoframe.h>
#include <qabstractvideobuffer.h>

#include <qgstbufferpoolinterface_p.h>

#include <gst/video/gstvideosink.h>

class QAbstractVideoSurface;

class QVideoSurfaceGstDelegate : public QObject
{
   CS_OBJECT(QVideoSurfaceGstDelegate)

 public:
   QVideoSurfaceGstDelegate(QAbstractVideoSurface *surface);
   ~QVideoSurfaceGstDelegate();

   QList<QVideoFrame::PixelFormat> supportedPixelFormats(
      QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

   QVideoSurfaceFormat surfaceFormat() const;

   bool start(const QVideoSurfaceFormat &format, int bytesPerLine);
   void stop();

   void unlock();

   bool isActive();

   QGstBufferPoolInterface *pool() {
      return m_pool;
   }
   QMutex *poolMutex() {
      return &m_poolMutex;
   }
   void clearPoolBuffers();

   void flush();

   GstFlowReturn render(GstBuffer *buffer);

 private:
   CS_SLOT_1(Private, void queuedStart())
   CS_SLOT_2(queuedStart)

   CS_SLOT_1(Private, void queuedStop())
   CS_SLOT_2(queuedStop)

   CS_SLOT_1(Private, void queuedFlush())
   CS_SLOT_2(queuedFlush)

   CS_SLOT_1(Private, void queuedRender())
   CS_SLOT_2(queuedRender)

   CS_SLOT_1(Private, void updateSupportedFormats())
   CS_SLOT_2(updateSupportedFormats)

   QPointer<QAbstractVideoSurface> m_surface;
   QList<QVideoFrame::PixelFormat> m_supportedPixelFormats;
   //pixel formats of buffers pool native type
   QList<QVideoFrame::PixelFormat> m_supportedPoolPixelFormats;
   QGstBufferPoolInterface *m_pool;
   QList<QGstBufferPoolInterface *> m_pools;
   QMutex m_poolMutex;
   QMutex m_mutex;
   QWaitCondition m_setupCondition;
   QWaitCondition m_renderCondition;
   QVideoSurfaceFormat m_format;
   QVideoFrame m_frame;
   GstFlowReturn m_renderReturn;
   int m_bytesPerLine;
   bool m_started;
   bool m_startCanceled;
};

class QVideoSurfaceGstSink
{
 public:
   GstVideoSink parent;

   static QVideoSurfaceGstSink *createSink(QAbstractVideoSurface *surface);

 private:
   static GType get_type();
   static void class_init(gpointer g_class, gpointer class_data);
   static void base_init(gpointer g_class);
   static void instance_init(GTypeInstance *instance, gpointer g_class);

   static void finalize(GObject *object);

   static void handleShowPrerollChange(GObject *o, GParamSpec *p, gpointer d);

   static GstStateChangeReturn change_state(GstElement *element, GstStateChange transition);

   static GstCaps *get_caps(GstBaseSink *sink);
   static gboolean set_caps(GstBaseSink *sink, GstCaps *caps);

   static GstFlowReturn buffer_alloc(
      GstBaseSink *sink, guint64 offset, guint size, GstCaps *caps, GstBuffer **buffer);

   static gboolean start(GstBaseSink *sink);
   static gboolean stop(GstBaseSink *sink);

   static gboolean unlock(GstBaseSink *sink);

#if GST_CHECK_VERSION(0, 10, 25)
   static GstFlowReturn show_frame(GstVideoSink *sink, GstBuffer *buffer);
#else
   static GstFlowReturn preroll(GstBaseSink *sink, GstBuffer *buffer);
   static GstFlowReturn render(GstBaseSink *sink, GstBuffer *buffer);
#endif

 private:
   QVideoSurfaceGstDelegate *delegate;

   GstCaps *lastRequestedCaps;
   GstCaps *lastBufferCaps;
   QVideoSurfaceFormat *lastSurfaceFormat;
};

class QVideoSurfaceGstSinkClass
{
 public:
   GstVideoSinkClass parent_class;
};

#endif

#endif
