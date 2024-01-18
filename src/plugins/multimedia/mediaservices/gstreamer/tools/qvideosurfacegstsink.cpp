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

#include <qvideosurfacegstsink_p.h>

#include <qabstractvideosurface.h>
#include <qvideoframe.h>
#include <qdebug.h>
#include <qmap.h>
#include <qdebug.h>
#include <qthread.h>

#include <qfactoryloader_p.h>
#include <qgstvideobuffer_p.h>
#include <qgstutils_p.h>

#if GST_VERSION_MAJOR >=1
#include <gst/video/video.h>
#endif

// #define DEBUG_VIDEO_SURFACE_SINK

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QGstBufferPoolInterface_iid, "/video/bufferpool", Qt::CaseInsensitive);
   return &retval;
}

QVideoSurfaceGstDelegate::QVideoSurfaceGstDelegate(QAbstractVideoSurface *surface)
   : m_surface(surface), m_pool(nullptr), m_renderReturn(GST_FLOW_ERROR), m_bytesPerLine(0), m_startCanceled(false)
{
   QFactoryLoader *factoryObj = loader();

   if (m_surface) {
      for (QLibraryHandle *handle : factoryObj->librarySet(QGstBufferPoolPluginKey)) {

         QObject *instance = factoryObj->instance(handle);
         QGstBufferPoolInterface *plugin = dynamic_cast<QGstBufferPoolInterface *>(instance);

         if (plugin) {
            m_pools.append(plugin);
         }
      }

      updateSupportedFormats();
      connect(m_surface, SIGNAL(supportedFormatsChanged()), this, SLOT(updateSupportedFormats()));
   }
}

QVideoSurfaceGstDelegate::~QVideoSurfaceGstDelegate()
{
}

QList<QVideoFrame::PixelFormat> QVideoSurfaceGstDelegate::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
   QMutexLocker locker(const_cast<QMutex *>(&m_mutex));

   if (!m_surface) {
      return QList<QVideoFrame::PixelFormat>();
   } else if (handleType == QAbstractVideoBuffer::NoHandle) {
      return m_supportedPixelFormats;
   } else if (handleType == m_pool->handleType()) {
      return m_supportedPoolPixelFormats;
   } else {
      return m_surface->supportedPixelFormats(handleType);
   }
}

QVideoSurfaceFormat QVideoSurfaceGstDelegate::surfaceFormat() const
{
   QMutexLocker locker(const_cast<QMutex *>(&m_mutex));
   return m_format;
}

bool QVideoSurfaceGstDelegate::start(const QVideoSurfaceFormat &format, int bytesPerLine)
{
   if (!m_surface) {
      return false;
   }

   QMutexLocker locker(&m_mutex);

   m_format = format;
   m_bytesPerLine = bytesPerLine;

   if (QThread::currentThread() == thread()) {
      m_started = !m_surface.isNull() ? m_surface->start(m_format) : false;
   } else {
      m_started = false;
      m_startCanceled = false;
      QMetaObject::invokeMethod(this, "queuedStart", Qt::QueuedConnection);

      /*
      Waiting for start() to be invoked in the main thread may block
      if gstreamer blocks the main thread until this call is finished.
      This situation is rare and usually caused by setState(Null)
      while pipeline is being prerolled.

      The proper solution to this involves controlling gstreamer pipeline from
      other thread than video surface.

      Currently start() fails if wait() timed out.
      */
      if (!m_setupCondition.wait(&m_mutex, 1000)) {
         qWarning() << "Failed to start video surface due to main thread blocked.";
         m_startCanceled = true;
      }
   }

   m_format = m_surface->surfaceFormat();

   return m_started;
}

void QVideoSurfaceGstDelegate::stop()
{
   if (!m_surface) {
      return;
   }

   QMutexLocker locker(&m_mutex);

   if (QThread::currentThread() == thread()) {
      if (!m_surface.isNull()) {
         m_surface->stop();
      }
   } else {
      QMetaObject::invokeMethod(this, "queuedStop", Qt::QueuedConnection);

      // Waiting for stop() to be invoked in the main thread may block
      // if gstreamer blocks the main thread until this call is finished.
      m_setupCondition.wait(&m_mutex, 500);
   }

   m_started = false;
}

void QVideoSurfaceGstDelegate::unlock()
{
   QMutexLocker locker(&m_mutex);

   m_startCanceled = true;
   m_setupCondition.wakeAll();
   m_renderCondition.wakeAll();
}

bool QVideoSurfaceGstDelegate::isActive()
{
   QMutexLocker locker(&m_mutex);
   return !m_surface.isNull() && m_surface->isActive();
}

void QVideoSurfaceGstDelegate::clearPoolBuffers()
{
   QMutexLocker locker(&m_poolMutex);
   if (m_pool) {
      m_pool->clear();
   }
}

void QVideoSurfaceGstDelegate::flush()
{
   QMutexLocker locker(&m_mutex);

   m_frame = QVideoFrame();
   m_renderCondition.wakeAll();

   if (QThread::currentThread() == thread()) {
      if (!m_surface.isNull()) {
         m_surface->present(m_frame);
      }
   } else {
      QMetaObject::invokeMethod(this, "queuedFlush", Qt::QueuedConnection);
   }
}

GstFlowReturn QVideoSurfaceGstDelegate::render(GstBuffer *buffer)
{
   if (!m_surface) {
      qWarning() << "Rendering video frame to deleted surface, skip.";
      //return GST_FLOW_NOT_NEGOTIATED;
      return GST_FLOW_OK;
   }

   QMutexLocker locker(&m_mutex);

   QAbstractVideoBuffer *videoBuffer = nullptr;

   if (m_pool) {
      videoBuffer = m_pool->prepareVideoBuffer(buffer, m_bytesPerLine);
   }

   if (! videoBuffer) {
      videoBuffer = new QGstVideoBuffer(buffer, m_bytesPerLine);
   }

   m_frame = QVideoFrame(videoBuffer, m_format.frameSize(), m_format.pixelFormat());

   QGstUtils::setFrameTimeStamps(&m_frame, buffer);

   m_renderReturn = GST_FLOW_OK;

   if (QThread::currentThread() == thread()) {
      if (!m_surface.isNull()) {
         m_surface->present(m_frame);
      } else {
         qWarning() << "m_surface.isNull().";
      }
   } else {
      QMetaObject::invokeMethod(this, "queuedRender", Qt::QueuedConnection);
      m_renderCondition.wait(&m_mutex, 300);
   }

   m_frame = QVideoFrame();
   return m_renderReturn;
}

void QVideoSurfaceGstDelegate::queuedStart()
{
   QMutexLocker locker(&m_mutex);

   if (!m_startCanceled) {
      m_started = m_surface->start(m_format);
      m_setupCondition.wakeAll();
   }
}

void QVideoSurfaceGstDelegate::queuedStop()
{
   QMutexLocker locker(&m_mutex);

   m_surface->stop();

   m_setupCondition.wakeAll();
}

void QVideoSurfaceGstDelegate::queuedFlush()
{
   QMutexLocker locker(&m_mutex);

   if (!m_surface.isNull()) {
      m_surface->present(QVideoFrame());
   }
}

void QVideoSurfaceGstDelegate::queuedRender()
{
   QMutexLocker locker(&m_mutex);

   if (!m_frame.isValid()) {
      return;
   }

   if (m_surface.isNull()) {
      qWarning() << "Rendering video frame to deleted surface, skip the frame";
      m_renderReturn = GST_FLOW_OK;

   } else if (m_surface->present(m_frame)) {
      m_renderReturn = GST_FLOW_OK;

   } else {
      switch (m_surface->error()) {
         case QAbstractVideoSurface::NoError:
            m_renderReturn = GST_FLOW_OK;
            break;

         case QAbstractVideoSurface::StoppedError:
            //It's likely we are in process of changing video output
            //and the surface is already stopped, ignore the frame
            m_renderReturn = GST_FLOW_OK;
            break;

         default:
            qWarning() << "Failed to render video frame:" << m_surface->error();
            m_renderReturn = GST_FLOW_OK;
            break;
      }
   }

   m_renderCondition.wakeAll();
}

void QVideoSurfaceGstDelegate::updateSupportedFormats()
{
   QGstBufferPoolInterface *newPool = nullptr;
   for (QGstBufferPoolInterface *pool : m_pools) {
      if (!m_surface->supportedPixelFormats(pool->handleType()).isEmpty()) {
         newPool = pool;
         break;
      }
   }

   if (newPool != m_pool) {
      QMutexLocker lock(&m_poolMutex);

      if (m_pool) {
         m_pool->clear();
      }
      m_pool = newPool;
   }

   QMutexLocker locker(&m_mutex);

   m_supportedPixelFormats.clear();
   m_supportedPoolPixelFormats.clear();

   if (m_surface) {
      m_supportedPixelFormats = m_surface->supportedPixelFormats();
      if (m_pool) {
         m_supportedPoolPixelFormats = m_surface->supportedPixelFormats(m_pool->handleType());
      }
   }
}

static GstVideoSinkClass *sink_parent_class;

#define VO_SINK(s) QVideoSurfaceGstSink *sink(reinterpret_cast<QVideoSurfaceGstSink *>(s))

QVideoSurfaceGstSink *QVideoSurfaceGstSink::createSink(QAbstractVideoSurface *surface)
{
   QVideoSurfaceGstSink *sink = reinterpret_cast<QVideoSurfaceGstSink *>(
         g_object_new(QVideoSurfaceGstSink::get_type(), nullptr));

   sink->delegate = new QVideoSurfaceGstDelegate(surface);

   g_signal_connect(G_OBJECT(sink), "notify::show-preroll-frame", G_CALLBACK(handleShowPrerollChange), sink);

   return sink;
}

GType QVideoSurfaceGstSink::get_type()
{
   static GType type = 0;

   if (type == 0) {
      static const GTypeInfo info = {
         sizeof(QVideoSurfaceGstSinkClass),                 // class_size
         base_init,                                         // base_init
         nullptr,                                          // base_finalize
         class_init,                                       // class_init
         nullptr,                                          // class_finalize
         nullptr,                                          // class_data
         sizeof(QVideoSurfaceGstSink),                      // instance_size
         0,                                                 // n_preallocs
         instance_init,                                     // instance_init
         nullptr                                           // value_table
      };

      type = g_type_register_static(GST_TYPE_VIDEO_SINK, "QVideoSurfaceGstSink", &info, GTypeFlags(0));
   }

   return type;
}

void QVideoSurfaceGstSink::class_init(gpointer g_class, gpointer class_data)
{
   (void) class_data;

   sink_parent_class = reinterpret_cast<GstVideoSinkClass *>(g_type_class_peek_parent(g_class));

   GstBaseSinkClass *base_sink_class = reinterpret_cast<GstBaseSinkClass *>(g_class);
   base_sink_class->get_caps = QVideoSurfaceGstSink::get_caps;
   base_sink_class->set_caps = QVideoSurfaceGstSink::set_caps;
   base_sink_class->buffer_alloc = QVideoSurfaceGstSink::buffer_alloc;
   base_sink_class->start = QVideoSurfaceGstSink::start;
   base_sink_class->stop = QVideoSurfaceGstSink::stop;
   base_sink_class->unlock = QVideoSurfaceGstSink::unlock;

#if GST_CHECK_VERSION(0, 10, 25)
   GstVideoSinkClass *video_sink_class = reinterpret_cast<GstVideoSinkClass *>(g_class);
   video_sink_class->show_frame = QVideoSurfaceGstSink::show_frame;
#else
   base_sink_class->preroll = QVideoSurfaceGstSink::preroll;
   base_sink_class->render = QVideoSurfaceGstSink::render;
#endif

   GstElementClass *element_class = reinterpret_cast<GstElementClass *>(g_class);
   element_class->change_state = QVideoSurfaceGstSink::change_state;

   GObjectClass *object_class = reinterpret_cast<GObjectClass *>(g_class);
   object_class->finalize = QVideoSurfaceGstSink::finalize;
}

void QVideoSurfaceGstSink::base_init(gpointer g_class)
{
   static GstStaticPadTemplate sink_pad_template = GST_STATIC_PAD_TEMPLATE(
         "sink", GST_PAD_SINK, GST_PAD_ALWAYS, GST_STATIC_CAPS(
            "video/x-raw-rgb, "
            "framerate = (fraction) [ 0, MAX ], "
            "width = (int) [ 1, MAX ], "
            "height = (int) [ 1, MAX ]; "
            "video/x-raw-yuv, "
            "framerate = (fraction) [ 0, MAX ], "
            "width = (int) [ 1, MAX ], "
            "height = (int) [ 1, MAX ]"));

   gst_element_class_add_pad_template(
      GST_ELEMENT_CLASS(g_class), gst_static_pad_template_get(&sink_pad_template));
}

void QVideoSurfaceGstSink::instance_init(GTypeInstance *instance, gpointer g_class)
{
   VO_SINK(instance);

   (void) g_class;

   sink->delegate          = nullptr;
   sink->lastRequestedCaps = nullptr;
   sink->lastBufferCaps    = nullptr;
   sink->lastSurfaceFormat = new QVideoSurfaceFormat;
}

void QVideoSurfaceGstSink::finalize(GObject *object)
{
   VO_SINK(object);

   delete sink->lastSurfaceFormat;
   sink->lastSurfaceFormat = nullptr;

   if (sink->lastBufferCaps) {
      gst_caps_unref(sink->lastBufferCaps);
   }
   sink->lastBufferCaps = nullptr;

   if (sink->lastRequestedCaps) {
      gst_caps_unref(sink->lastRequestedCaps);
   }
   sink->lastRequestedCaps = nullptr;

   delete sink->delegate;

   // Chain up
   G_OBJECT_CLASS(sink_parent_class)->finalize(object);
}

void QVideoSurfaceGstSink::handleShowPrerollChange(GObject *o, GParamSpec *p, gpointer d)
{
   (void) o;
   (void) p;

   QVideoSurfaceGstSink *sink = reinterpret_cast<QVideoSurfaceGstSink *>(d);

   gboolean showPrerollFrame = true; // "show-preroll-frame" property is true by default
   g_object_get(G_OBJECT(sink), "show-preroll-frame", &showPrerollFrame, nullptr);

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

GstStateChangeReturn QVideoSurfaceGstSink::change_state(GstElement *element, GstStateChange transition)
{
   QVideoSurfaceGstSink *sink = reinterpret_cast<QVideoSurfaceGstSink *>(element);

   gboolean showPrerollFrame = true; // "show-preroll-frame" property is true by default
   g_object_get(G_OBJECT(element), "show-preroll-frame", &showPrerollFrame, nullptr);

   // If show-preroll-frame is 'false' when transitioning from GST_STATE_PLAYING to
   // GST_STATE_PAUSED, it means the QMediaPlayer was stopped.
   // We need to flush the current frame.
   if (transition == GST_STATE_CHANGE_PLAYING_TO_PAUSED && !showPrerollFrame) {
      sink->delegate->flush();
   }

   return GST_ELEMENT_CLASS(sink_parent_class)->change_state(element, transition);
}

GstCaps *QVideoSurfaceGstSink::get_caps(GstBaseSink *base)
{
   VO_SINK(base);

   // Find the supported pixel formats
   // with buffer pool specific formats listed first
   QList<QVideoFrame::PixelFormat> supportedFormats;

   QList<QVideoFrame::PixelFormat> poolHandleFormats;
   sink->delegate->poolMutex()->lock();
   QGstBufferPoolInterface *pool = sink->delegate->pool();

   if (pool) {
      poolHandleFormats = sink->delegate->supportedPixelFormats(pool->handleType());
   }
   sink->delegate->poolMutex()->unlock();

   supportedFormats = poolHandleFormats;
   for (QVideoFrame::PixelFormat format : sink->delegate->supportedPixelFormats()) {
      if (!poolHandleFormats.contains(format)) {
         supportedFormats.append(format);
      }
   }

   return QGstUtils::capsForFormats(supportedFormats);
}

gboolean QVideoSurfaceGstSink::set_caps(GstBaseSink *base, GstCaps *caps)
{
   VO_SINK(base);

#ifdef DEBUG_VIDEO_SURFACE_SINK
   qDebug() << "set_caps:";
   qDebug() << gst_caps_to_string(caps);
#endif

   if (!caps) {
      sink->delegate->stop();

      return TRUE;
   } else {
      int bytesPerLine = 0;
      QGstBufferPoolInterface *pool = sink->delegate->pool();
      QAbstractVideoBuffer::HandleType handleType =
         pool ? pool->handleType() : QAbstractVideoBuffer::NoHandle;

      QVideoSurfaceFormat format = QGstUtils::formatForCaps(caps, &bytesPerLine, handleType);

      if (sink->delegate->isActive()) {
         QVideoSurfaceFormat surfaceFormst = sink->delegate->surfaceFormat();

         if (format.pixelFormat() == surfaceFormst.pixelFormat() &&
            format.frameSize() == surfaceFormst.frameSize()) {
            return TRUE;
         } else {
            sink->delegate->stop();
         }
      }

      if (sink->lastRequestedCaps) {
         gst_caps_unref(sink->lastRequestedCaps);
      }
      sink->lastRequestedCaps = nullptr;

#ifdef DEBUG_VIDEO_SURFACE_SINK
      qDebug() << "Starting video surface, format:";
      qDebug() << format;
      qDebug() << "bytesPerLine:" << bytesPerLine;
#endif

      if (sink->delegate->start(format, bytesPerLine)) {
         return TRUE;
      } else {
         qWarning() << "Failed to start video surface";
      }
   }

   return FALSE;
}

GstFlowReturn QVideoSurfaceGstSink::buffer_alloc(
   GstBaseSink *base, guint64 offset, guint size, GstCaps *caps, GstBuffer **buffer)
{
   VO_SINK(base);

   (void) offset;
   (void) size;

   if (!buffer) {
      return GST_FLOW_ERROR;
   }

   *buffer = nullptr;

   if (!sink->delegate->pool()) {
      return GST_FLOW_OK;
   }

   QMutexLocker poolLock(sink->delegate->poolMutex());
   QGstBufferPoolInterface *pool = sink->delegate->pool();

   if (!pool) {
      return GST_FLOW_OK;
   }

   if (sink->lastRequestedCaps && gst_caps_is_equal(sink->lastRequestedCaps, caps)) {
      //qDebug() << "reusing last caps";
      *buffer = GST_BUFFER(pool->takeBuffer(*sink->lastSurfaceFormat, sink->lastBufferCaps));
      return GST_FLOW_OK;
   }

   if (sink->delegate->supportedPixelFormats(pool->handleType()).isEmpty()) {
      return GST_FLOW_OK;
   }

   poolLock.unlock();

   GstCaps *intersection = gst_caps_intersect(get_caps(GST_BASE_SINK(sink)), caps);

   if (gst_caps_is_empty (intersection)) {
      gst_caps_unref(intersection);
      return GST_FLOW_NOT_NEGOTIATED;
   }

   if (sink->delegate->isActive()) {
      //if format was changed, restart the surface
      QVideoSurfaceFormat format = QGstUtils::formatForCaps(intersection);
      QVideoSurfaceFormat surfaceFormat = sink->delegate->surfaceFormat();

      if (format.pixelFormat() != surfaceFormat.pixelFormat() ||
         format.frameSize() != surfaceFormat.frameSize()) {
#ifdef DEBUG_VIDEO_SURFACE_SINK
         qDebug() << "new format requested, restart video surface";
#endif
         sink->delegate->stop();
      }
   }

   if (!sink->delegate->isActive()) {
      int bytesPerLine = 0;
      QGstBufferPoolInterface *pool = sink->delegate->pool();
      QAbstractVideoBuffer::HandleType handleType =
         pool ? pool->handleType() : QAbstractVideoBuffer::NoHandle;

      QVideoSurfaceFormat format = QGstUtils::formatForCaps(intersection, &bytesPerLine, handleType);

      if (!sink->delegate->start(format, bytesPerLine)) {
         qWarning() << "failed to start video surface";
         return GST_FLOW_NOT_NEGOTIATED;
      }
   }

   poolLock.relock();
   pool = sink->delegate->pool();

   QVideoSurfaceFormat surfaceFormat = sink->delegate->surfaceFormat();

   if (!pool->isFormatSupported(surfaceFormat)) {
      qDebug() << "sink does not support native pool format, skip custom buffers allocation";
      return GST_FLOW_OK;
   }

   if (sink->lastRequestedCaps) {
      gst_caps_unref(sink->lastRequestedCaps);
   }
   sink->lastRequestedCaps = caps;
   gst_caps_ref(sink->lastRequestedCaps);

   if (sink->lastBufferCaps) {
      gst_caps_unref(sink->lastBufferCaps);
   }
   sink->lastBufferCaps = intersection;
   gst_caps_ref(sink->lastBufferCaps);

   *sink->lastSurfaceFormat = surfaceFormat;

   *buffer =  GST_BUFFER(pool->takeBuffer(surfaceFormat, intersection));

   return GST_FLOW_OK;
}

gboolean QVideoSurfaceGstSink::start(GstBaseSink *base)
{
   (void) base;

   return TRUE;
}

gboolean QVideoSurfaceGstSink::stop(GstBaseSink *base)
{
   VO_SINK(base);
   sink->delegate->clearPoolBuffers();

   return TRUE;
}

gboolean QVideoSurfaceGstSink::unlock(GstBaseSink *base)
{
   VO_SINK(base);
   sink->delegate->unlock();
   return TRUE;
}

#if GST_CHECK_VERSION(0, 10, 25)
GstFlowReturn QVideoSurfaceGstSink::show_frame(GstVideoSink *base, GstBuffer *buffer)
{
   VO_SINK(base);
   return sink->delegate->render(buffer);
}
#else
GstFlowReturn QVideoSurfaceGstSink::preroll(GstBaseSink *base, GstBuffer *buffer)
{
   VO_SINK(base);
   gboolean showPrerollFrame = true;
   g_object_get(G_OBJECT(sink), "show-preroll-frame", &showPrerollFrame, nullptr);

   if (showPrerollFrame) {
      return sink->delegate->render(buffer);
   }

   return GST_FLOW_OK;
}

GstFlowReturn QVideoSurfaceGstSink::render(GstBaseSink *base, GstBuffer *buffer)
{
   VO_SINK(base);
   return sink->delegate->render(buffer);
}
#endif


