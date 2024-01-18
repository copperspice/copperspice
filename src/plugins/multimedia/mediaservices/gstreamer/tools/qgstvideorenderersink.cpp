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

#include <qgstvideorenderersink_p.h>

#include <qalgorithms.h>
#include <qabstractvideosurface.h>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <qevent.h>
#include <qmap.h>
#include <qthread.h>
#include <qvideoframe.h>

#include <qfactoryloader_p.h>
#include <qgstvideobuffer_p.h>
#include <qgstutils_p.h>

#include <gst/video/video.h>

// #define DEBUG_VIDEO_SURFACE_SINK

QGstDefaultVideoRenderer::QGstDefaultVideoRenderer()
   : m_flushed(true)
{
}

QGstDefaultVideoRenderer::~QGstDefaultVideoRenderer()
{
}

GstCaps *QGstDefaultVideoRenderer::getCaps(QAbstractVideoSurface *surface)
{
   return QGstUtils::capsForFormats(surface->supportedPixelFormats());
}

bool QGstDefaultVideoRenderer::start(QAbstractVideoSurface *surface, GstCaps *caps)
{
   m_flushed = true;
   m_format = QGstUtils::formatForCaps(caps, &m_videoInfo);

   return m_format.isValid() && surface->start(m_format);
}

void QGstDefaultVideoRenderer::stop(QAbstractVideoSurface *surface)
{
   m_flushed = true;
   if (surface) {
      surface->stop();
   }
}

bool QGstDefaultVideoRenderer::present(QAbstractVideoSurface *surface, GstBuffer *buffer)
{
   m_flushed = false;
   QVideoFrame frame(new QGstVideoBuffer(buffer, m_videoInfo), m_format.frameSize(), m_format.pixelFormat());
   QGstUtils::setFrameTimeStamps(&frame, buffer);

   return surface->present(frame);
}

void QGstDefaultVideoRenderer::flush(QAbstractVideoSurface *surface)
{
   if (surface && !m_flushed) {
      surface->present(QVideoFrame());
   }
   m_flushed = true;
}

bool QGstDefaultVideoRenderer::proposeAllocation(GstQuery *)
{
   return true;
}

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QGstVideoRendererInterface_iid, "/video/gstvideorenderer", Qt::CaseInsensitive);
   return &retval;
}

QVideoSurfaceGstDelegate::QVideoSurfaceGstDelegate(QAbstractVideoSurface *surface)
   : m_surface(surface), m_renderer(nullptr), m_activeRenderer(nullptr), m_surfaceCaps(nullptr), m_startCaps(nullptr),
     m_renderBuffer(nullptr), m_notified(false), m_stop(false), m_flush(false)
{
   QFactoryLoader *factoryObj = loader();

   for (QLibraryHandle *handle : factoryObj->librarySet(QGstVideoRendererPluginKey)) {

      QObject *instance = factoryObj->instance(handle);
      QGstVideoRendererInterface *plugin = dynamic_cast<QGstVideoRendererInterface *>(instance);

      if (QGstVideoRenderer *renderer = plugin ? plugin->createRenderer() : nullptr) {
         m_renderers.append(renderer);
      }
   }

   m_renderers.append(new QGstDefaultVideoRenderer);
   updateSupportedFormats();
   connect(m_surface, SIGNAL(supportedFormatsChanged()), this, SLOT(updateSupportedFormats()));
}

QVideoSurfaceGstDelegate::~QVideoSurfaceGstDelegate()
{
   qDeleteAll(m_renderers);

   if (m_surfaceCaps) {
      gst_caps_unref(m_surfaceCaps);
   }

   if (m_startCaps) {
      gst_caps_unref(m_startCaps);
   }
}

GstCaps *QVideoSurfaceGstDelegate::caps()
{
   QMutexLocker locker(&m_mutex);

   gst_caps_ref(m_surfaceCaps);

   return m_surfaceCaps;
}

bool QVideoSurfaceGstDelegate::start(GstCaps *caps)
{
   QMutexLocker locker(&m_mutex);

   if (m_activeRenderer) {
      m_flush = true;
      m_stop = true;
   }

   if (m_startCaps) {
      gst_caps_unref(m_startCaps);
   }
   m_startCaps = caps;
   gst_caps_ref(m_startCaps);

   /*
   Waiting for start() to be invoked in the main thread may block
   if gstreamer blocks the main thread until this call is finished.
   This situation is rare and usually caused by setState(Null)
   while pipeline is being prerolled.

   The proper solution to this involves controlling gstreamer pipeline from
   other thread than video surface.

   Currently start() fails if wait() timed out.
   */
   if (!waitForAsyncEvent(&locker, &m_setupCondition, 1000) && m_startCaps) {
      qWarning() << "Failed to start video surface due to main thread blocked.";
      gst_caps_unref(m_startCaps);
      m_startCaps = nullptr;
   }

   return m_activeRenderer != nullptr;
}

void QVideoSurfaceGstDelegate::stop()
{
   QMutexLocker locker(&m_mutex);

   if (!m_activeRenderer) {
      return;
   }

   m_flush = true;
   m_stop = true;

   if (m_startCaps) {
      gst_caps_unref(m_startCaps);
      m_startCaps = nullptr;
   }

   waitForAsyncEvent(&locker, &m_setupCondition, 500);
}

void QVideoSurfaceGstDelegate::unlock()
{
   QMutexLocker locker(&m_mutex);

   m_setupCondition.wakeAll();
   m_renderCondition.wakeAll();
}

bool QVideoSurfaceGstDelegate::proposeAllocation(GstQuery *query)
{
   QMutexLocker locker(&m_mutex);

   if (QGstVideoRenderer *pool = m_activeRenderer) {
      locker.unlock();

      return pool->proposeAllocation(query);
   } else {
      return false;
   }
}

void QVideoSurfaceGstDelegate::flush()
{
   QMutexLocker locker(&m_mutex);

   m_flush = true;
   m_renderBuffer = nullptr;
   m_renderCondition.wakeAll();

   notify();
}

GstFlowReturn QVideoSurfaceGstDelegate::render(GstBuffer *buffer)
{
   QMutexLocker locker(&m_mutex);

   m_renderReturn = GST_FLOW_OK;
   m_renderBuffer = buffer;

   GstFlowReturn flowReturn = waitForAsyncEvent(&locker, &m_renderCondition, 300)
      ? m_renderReturn
      : GST_FLOW_ERROR;

   m_renderBuffer = nullptr;

   return flowReturn;
}

bool QVideoSurfaceGstDelegate::event(QEvent *event)
{
   if (event->type() == QEvent::UpdateRequest) {
      QMutexLocker locker(&m_mutex);

      if (m_notified) {
         while (handleEvent(&locker)) {}
         m_notified = false;
      }
      return true;
   } else {
      return QObject::event(event);
   }
}

bool QVideoSurfaceGstDelegate::handleEvent(QMutexLocker *locker)
{
   if (m_flush) {
      m_flush = false;
      if (m_activeRenderer) {
         locker->unlock();

         m_activeRenderer->flush(m_surface);
      }
   } else if (m_stop) {
      m_stop = false;

      if (QGstVideoRenderer *const activePool = m_activeRenderer) {
         m_activeRenderer = nullptr;
         locker->unlock();

         activePool->stop(m_surface);

         locker->relock();
      }
   } else if (m_startCaps) {
      Q_ASSERT(!m_activeRenderer);

      GstCaps *const startCaps = m_startCaps;
      m_startCaps = nullptr;

      if (m_renderer && m_surface) {
         locker->unlock();

         const bool started = m_renderer->start(m_surface, startCaps);

         locker->relock();

         m_activeRenderer = started ? m_renderer : nullptr;

      } else if (QGstVideoRenderer *const activePool = m_activeRenderer) {
         m_activeRenderer = nullptr;
         locker->unlock();

         activePool->stop(m_surface);

         locker->relock();
      }

      gst_caps_unref(startCaps);
   } else if (m_renderBuffer) {
      GstBuffer *buffer = m_renderBuffer;
      m_renderBuffer = nullptr;
      m_renderReturn = GST_FLOW_ERROR;

      if (m_activeRenderer && m_surface) {
         gst_buffer_ref(buffer);

         locker->unlock();

         const bool rendered = m_activeRenderer->present(m_surface, buffer);

         gst_buffer_unref(buffer);

         locker->relock();

         if (rendered) {
            m_renderReturn = GST_FLOW_OK;
         }
      }

      m_renderCondition.wakeAll();
   } else {
      m_setupCondition.wakeAll();

      return false;
   }
   return true;
}

void QVideoSurfaceGstDelegate::notify()
{
   if (!m_notified) {
      m_notified = true;
      QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
   }
}

bool QVideoSurfaceGstDelegate::waitForAsyncEvent(
   QMutexLocker *locker, QWaitCondition *condition, unsigned long time)
{
   if (QThread::currentThread() == thread()) {
      while (handleEvent(locker)) {}
      m_notified = false;

      return true;
   } else {
      notify();

      return condition->wait(&m_mutex, time);
   }
}

void QVideoSurfaceGstDelegate::updateSupportedFormats()
{
   if (m_surfaceCaps) {
      gst_caps_unref(m_surfaceCaps);
      m_surfaceCaps = nullptr;
   }

   for (QGstVideoRenderer *pool : m_renderers) {
      if (GstCaps *caps = pool->getCaps(m_surface)) {
         if (gst_caps_is_empty(caps)) {
            gst_caps_unref(caps);
            continue;
         }

         if (m_surfaceCaps) {
            gst_caps_unref(m_surfaceCaps);
         }

         m_renderer = pool;
         m_surfaceCaps = caps;
         break;
      } else {
         gst_caps_unref(caps);
      }
   }
}

static GstVideoSinkClass *sink_parent_class;

#define VO_SINK(s) QGstVideoRendererSink *sink(reinterpret_cast<QGstVideoRendererSink *>(s))

QGstVideoRendererSink *QGstVideoRendererSink::createSink(QAbstractVideoSurface *surface)
{
   QGstVideoRendererSink *sink = reinterpret_cast<QGstVideoRendererSink *>(
         g_object_new(QGstVideoRendererSink::get_type(), nullptr));

   sink->delegate = new QVideoSurfaceGstDelegate(surface);

   g_signal_connect(G_OBJECT(sink), "notify::show-preroll-frame", G_CALLBACK(handleShowPrerollChange), sink);

   return sink;
}

GType QGstVideoRendererSink::get_type()
{
   static GType type = 0;

   if (type == 0) {
      static const GTypeInfo info = {
         sizeof(QGstVideoRendererSinkClass),               // class_size
         base_init,                                        // base_init
         nullptr,                                          // base_finalize
         class_init,                                       // class_init
         nullptr,                                          // class_finalize
         nullptr,                                          // class_data
         sizeof(QGstVideoRendererSink),                    // instance_size
         0,                                                // n_preallocs
         instance_init,                                    // instance_init
         nullptr                                           // value_table
      };

      type = g_type_register_static(
            GST_TYPE_VIDEO_SINK, "QGstVideoRendererSink", &info, GTypeFlags(0));
   }

   return type;
}

void QGstVideoRendererSink::class_init(gpointer g_class, gpointer class_data)
{
   (void) class_data;

   sink_parent_class = reinterpret_cast<GstVideoSinkClass *>(g_type_class_peek_parent(g_class));

   GstVideoSinkClass *video_sink_class = reinterpret_cast<GstVideoSinkClass *>(g_class);
   video_sink_class->show_frame = QGstVideoRendererSink::show_frame;

   GstBaseSinkClass *base_sink_class = reinterpret_cast<GstBaseSinkClass *>(g_class);
   base_sink_class->get_caps = QGstVideoRendererSink::get_caps;
   base_sink_class->set_caps = QGstVideoRendererSink::set_caps;
   base_sink_class->propose_allocation = QGstVideoRendererSink::propose_allocation;
   base_sink_class->stop = QGstVideoRendererSink::stop;
   base_sink_class->unlock = QGstVideoRendererSink::unlock;

   GstElementClass *element_class = reinterpret_cast<GstElementClass *>(g_class);
   element_class->change_state = QGstVideoRendererSink::change_state;

   GObjectClass *object_class = reinterpret_cast<GObjectClass *>(g_class);
   object_class->finalize = QGstVideoRendererSink::finalize;
}

void QGstVideoRendererSink::base_init(gpointer g_class)
{
   static GstStaticPadTemplate sink_pad_template = GST_STATIC_PAD_TEMPLATE(
         "sink", GST_PAD_SINK, GST_PAD_ALWAYS, GST_STATIC_CAPS(
            "video/x-raw, "
            "framerate = (fraction) [ 0, MAX ], "
            "width = (int) [ 1, MAX ], "
            "height = (int) [ 1, MAX ]"));

   gst_element_class_add_pad_template(
      GST_ELEMENT_CLASS(g_class), gst_static_pad_template_get(&sink_pad_template));
}

void QGstVideoRendererSink::instance_init(GTypeInstance *instance, gpointer g_class)
{
   VO_SINK(instance);

   (void) g_class;

   sink->delegate = nullptr;
}

void QGstVideoRendererSink::finalize(GObject *object)
{
   VO_SINK(object);

   delete sink->delegate;

   // Chain up
   G_OBJECT_CLASS(sink_parent_class)->finalize(object);
}

void QGstVideoRendererSink::handleShowPrerollChange(GObject *o, GParamSpec *p, gpointer d)
{
   (void) o;
   (void) p;

   QGstVideoRendererSink *sink = reinterpret_cast<QGstVideoRendererSink *>(d);

   gboolean showPrerollFrame = true; // "show-preroll-frame" property is true by default
   g_object_get(G_OBJECT(sink), "show-preroll-frame", &showPrerollFrame, NULL);

   if (!showPrerollFrame) {
      GstState state = GST_STATE_VOID_PENDING;
      gst_element_get_state(GST_ELEMENT(sink), &state, nullptr, GST_CLOCK_TIME_NONE);
      // show-preroll-frame being set to 'false' while in GST_STATE_PAUSED means
      // the QMediaPlayer was stopped from the paused state.
      // We need to flush the current frame.
      if (state == GST_STATE_PAUSED) {
         sink->delegate->flush();
      }
   }
}

GstStateChangeReturn QGstVideoRendererSink::change_state(
   GstElement *element, GstStateChange transition)
{
   QGstVideoRendererSink *sink = reinterpret_cast<QGstVideoRendererSink *>(element);

   gboolean showPrerollFrame = true; // "show-preroll-frame" property is true by default
   g_object_get(G_OBJECT(element), "show-preroll-frame", &showPrerollFrame, NULL);

   // If show-preroll-frame is 'false' when transitioning from GST_STATE_PLAYING to
   // GST_STATE_PAUSED, it means the QMediaPlayer was stopped.
   // We need to flush the current frame.
   if (transition == GST_STATE_CHANGE_PLAYING_TO_PAUSED && !showPrerollFrame) {
      sink->delegate->flush();
   }

   return GST_ELEMENT_CLASS(sink_parent_class)->change_state(element, transition);
}

GstCaps *QGstVideoRendererSink::get_caps(GstBaseSink *base, GstCaps *filter)
{
   VO_SINK(base);

   GstCaps *caps = sink->delegate->caps();
   GstCaps *unfiltered = caps;
   if (filter) {
      caps = gst_caps_intersect(unfiltered, filter);
      gst_caps_unref(unfiltered);
   }

   return caps;
}

gboolean QGstVideoRendererSink::set_caps(GstBaseSink *base, GstCaps *caps)
{
   VO_SINK(base);

#ifdef DEBUG_VIDEO_SURFACE_SINK
   qDebug() << "set_caps:";
   qDebug() << caps;
#endif

   if (!caps) {
      sink->delegate->stop();

      return TRUE;
   } else if (sink->delegate->start(caps)) {
      return TRUE;
   } else {
      return FALSE;
   }
}

gboolean QGstVideoRendererSink::propose_allocation(GstBaseSink *base, GstQuery *query)
{
   VO_SINK(base);
   return sink->delegate->proposeAllocation(query);
}

gboolean QGstVideoRendererSink::stop(GstBaseSink *base)
{
   VO_SINK(base);
   sink->delegate->stop();
   return TRUE;
}

gboolean QGstVideoRendererSink::unlock(GstBaseSink *base)
{
   VO_SINK(base);
   sink->delegate->unlock();
   return TRUE;
}

GstFlowReturn QGstVideoRendererSink::show_frame(GstVideoSink *base, GstBuffer *buffer)
{
   VO_SINK(base);
   return sink->delegate->render(buffer);
}
