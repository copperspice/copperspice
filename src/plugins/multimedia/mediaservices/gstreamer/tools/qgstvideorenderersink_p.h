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

#ifndef QGSTVIDEORENDERERSINK_P_H
#define QGSTVIDEORENDERERSINK_P_H

#include <gst/video/gstvideosink.h>
#include <gst/video/video.h>

#include <qlist.h>
#include <qmutex.h>
#include <qqueue.h>
#include <qpointer.h>
#include <qwaitcondition.h>
#include <qvideosurfaceformat.h>
#include <qvideoframe.h>
#include <qabstractvideobuffer.h>

#include <qgstvideorendererplugin_p.h>

class QAbstractVideoSurface;

class QGstDefaultVideoRenderer : public QGstVideoRenderer
{
 public:
   QGstDefaultVideoRenderer();
   ~QGstDefaultVideoRenderer();

   GstCaps *getCaps(QAbstractVideoSurface *surface) override;
   bool start(QAbstractVideoSurface *surface, GstCaps *caps) override;
   void stop(QAbstractVideoSurface *surface) override;

   bool proposeAllocation(GstQuery *query) override;

   bool present(QAbstractVideoSurface *surface, GstBuffer *buffer) override;
   void flush(QAbstractVideoSurface *surface) override;

 private:
   QVideoSurfaceFormat m_format;
   GstVideoInfo m_videoInfo;
   bool m_flushed;
};

class QVideoSurfaceGstDelegate : public QObject
{
   CS_OBJECT(QVideoSurfaceGstDelegate)

 public:
   QVideoSurfaceGstDelegate(QAbstractVideoSurface *surface);
   ~QVideoSurfaceGstDelegate();

   GstCaps *caps();

   bool start(GstCaps *caps);
   void stop();
   void unlock();
   bool proposeAllocation(GstQuery *query);

   void flush();

   GstFlowReturn render(GstBuffer *buffer);

   bool event(QEvent *event) override;

 private:
   CS_SLOT_1(Private, bool handleEvent(QMutexLocker *locker))
   CS_SLOT_2(handleEvent)

   CS_SLOT_1(Private, void updateSupportedFormats())
   CS_SLOT_2(updateSupportedFormats)

   void notify();
   bool waitForAsyncEvent(QMutexLocker *locker, QWaitCondition *condition, unsigned long time);

   QPointer<QAbstractVideoSurface> m_surface;

   QMutex m_mutex;
   QWaitCondition m_setupCondition;
   QWaitCondition m_renderCondition;
   GstFlowReturn m_renderReturn;
   QList<QGstVideoRenderer *> m_renderers;
   QGstVideoRenderer *m_renderer;
   QGstVideoRenderer *m_activeRenderer;

   GstCaps *m_surfaceCaps;
   GstCaps *m_startCaps;
   GstBuffer *m_renderBuffer;

   bool m_notified;
   bool m_stop;
   bool m_flush;
};

class QGstVideoRendererSink
{
 public:
   GstVideoSink parent;

   static QGstVideoRendererSink *createSink(QAbstractVideoSurface *surface);

 private:
   static GType get_type();
   static void class_init(gpointer g_class, gpointer class_data);
   static void base_init(gpointer g_class);
   static void instance_init(GTypeInstance *instance, gpointer g_class);

   static void finalize(GObject *object);

   static void handleShowPrerollChange(GObject *o, GParamSpec *p, gpointer d);

   static GstStateChangeReturn change_state(GstElement *element, GstStateChange transition);

   static GstCaps *get_caps(GstBaseSink *sink, GstCaps *filter);

   static gboolean set_caps(GstBaseSink *sink, GstCaps *caps);
   static gboolean propose_allocation(GstBaseSink *sink, GstQuery *query);
   static gboolean stop(GstBaseSink *sink);
   static gboolean unlock(GstBaseSink *sink);

   static GstFlowReturn show_frame(GstVideoSink *sink, GstBuffer *buffer);

   QVideoSurfaceGstDelegate *delegate;
};


class QGstVideoRendererSinkClass
{
 public:
   GstVideoSinkClass parent_class;
};

#endif
