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

//#define DEBUG_DECODER

#include <qgstreameraudiodecodersession.h>

#include <qdatetime.h>
#include <qdebug.h>
#include <qsize.h>
#include <qtimer.h>
#include <qdebug.h>
#include <qdir.h>
#include <qstandardpaths.h>
#include <qurl.h>

#include <qgstreamerbushelper_p.h>
#include <qgstutils_p.h>

#include <gst/gstvalue.h>
#include <gst/base/gstbasesrc.h>

#define MAX_BUFFERS_IN_QUEUE 4

typedef enum {
   GST_PLAY_FLAG_VIDEO         = 0x00000001,
   GST_PLAY_FLAG_AUDIO         = 0x00000002,
   GST_PLAY_FLAG_TEXT          = 0x00000004,
   GST_PLAY_FLAG_VIS           = 0x00000008,
   GST_PLAY_FLAG_SOFT_VOLUME   = 0x00000010,
   GST_PLAY_FLAG_NATIVE_AUDIO  = 0x00000020,
   GST_PLAY_FLAG_NATIVE_VIDEO  = 0x00000040,
   GST_PLAY_FLAG_DOWNLOAD      = 0x00000080,
   GST_PLAY_FLAG_BUFFERING     = 0x000000100
} GstPlayFlags;

QGstreamerAudioDecoderSession::QGstreamerAudioDecoderSession(QObject *parent)
   : QObject(parent),
     m_state(QAudioDecoder::StoppedState),
     m_pendingState(QAudioDecoder::StoppedState),
     m_busHelper(nullptr),
     m_bus(nullptr),
     m_playbin(nullptr),
     m_outputBin(nullptr),
     m_audioConvert(nullptr),
     m_appSink(nullptr),
#if defined(HAVE_GST_APPSRC)
     m_appSrc(nullptr),
#endif
     mDevice(nullptr),
     m_buffersAvailable(0),
     m_position(-1),
     m_duration(-1),
     m_durationQueries(0)
{
   // Create pipeline here
   m_playbin = gst_element_factory_make(QT_GSTREAMER_PLAYBIN_ELEMENT_NAME, nullptr);

   if (m_playbin != nullptr) {
      // Sort out messages
      m_bus = gst_element_get_bus(m_playbin);
      m_busHelper = new QGstreamerBusHelper(m_bus, this);
      m_busHelper->installMessageFilter(this);

      // Set the rest of the pipeline up
      setAudioFlags(true);

      m_audioConvert = gst_element_factory_make("audioconvert", nullptr);

      m_outputBin = gst_bin_new("audio-output-bin");
      gst_bin_add(GST_BIN(m_outputBin), m_audioConvert);

      // add ghostpad
      GstPad *pad = gst_element_get_static_pad(m_audioConvert, "sink");
      Q_ASSERT(pad);
      gst_element_add_pad(GST_ELEMENT(m_outputBin), gst_ghost_pad_new("sink", pad));
      gst_object_unref(GST_OBJECT(pad));

      g_object_set(G_OBJECT(m_playbin), "audio-sink", m_outputBin, nullptr);

#if defined(HAVE_GST_APPSRC)
      g_signal_connect(G_OBJECT(m_playbin), "deep-notify::source", (GCallback) &QGstreamerAudioDecoderSession::configureAppSrcElement,
         (gpointer)this);
#endif

      // Set volume to 100%
      gdouble volume = 1.0;
      g_object_set(G_OBJECT(m_playbin), "volume", volume, nullptr);
   }
}

QGstreamerAudioDecoderSession::~QGstreamerAudioDecoderSession()
{
   if (m_playbin) {
      stop();

      delete m_busHelper;
#if defined(HAVE_GST_APPSRC)
      delete m_appSrc;
#endif
      gst_object_unref(GST_OBJECT(m_bus));
      gst_object_unref(GST_OBJECT(m_playbin));
   }
}

#if defined(HAVE_GST_APPSRC)
void QGstreamerAudioDecoderSession::configureAppSrcElement(GObject *object, GObject *orig, GParamSpec *pspec,
   QGstreamerAudioDecoderSession *self)
{
   (void) object;
   (void) pspec;

   // In case we switch from appsrc to not
   if (!self->appsrc()) {
      return;
   }

   GstElement *appsrc;
   g_object_get(orig, "source", &appsrc, nullptr);

   if (!self->appsrc()->setup(appsrc)) {
      qWarning() << "Could not setup appsrc element";
   }

   g_object_unref(G_OBJECT(appsrc));
}
#endif

bool QGstreamerAudioDecoderSession::processBusMessage(const QGstreamerMessage &message)
{
   GstMessage *gm = message.rawMessage();
   if (gm) {
      if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_DURATION) {
         updateDuration();
      } else if (GST_MESSAGE_SRC(gm) == GST_OBJECT_CAST(m_playbin)) {
         switch (GST_MESSAGE_TYPE(gm))  {
            case GST_MESSAGE_STATE_CHANGED: {
               GstState    oldState;
               GstState    newState;
               GstState    pending;

               gst_message_parse_state_changed(gm, &oldState, &newState, &pending);

#ifdef DEBUG_DECODER
               QStringList states;
               states << "GST_STATE_VOID_PENDING" <<  "GST_STATE_NULL" << "GST_STATE_READY" << "GST_STATE_PAUSED" << "GST_STATE_PLAYING";

               qDebug() << QString("state changed: old: %1  new: %2  pending: %3") \
                  .formatArg(states[oldState]) \
                  .formatArg(states[newState]) \
                  .formatArg(states[pending]) << "internal" << m_state;
#endif

               QAudioDecoder::State prevState = m_state;

               switch (newState) {
                  case GST_STATE_VOID_PENDING:
                  case GST_STATE_NULL:
                     m_state = QAudioDecoder::StoppedState;
                     break;
                  case GST_STATE_READY:
                     m_state = QAudioDecoder::StoppedState;
                     break;
                  case GST_STATE_PLAYING:
                     m_state = QAudioDecoder::DecodingState;
                     break;
                  case GST_STATE_PAUSED:
                     m_state = QAudioDecoder::DecodingState;

                     //gstreamer doesn't give a reliable indication the duration
                     //information is ready, GST_MESSAGE_DURATION is not sent by most elements
                     //the duration is queried up to 5 times with increasing delay
                     m_durationQueries = 5;
                     updateDuration();
                     break;
               }

               if (prevState != m_state) {
                  emit stateChanged(m_state);
               }
            }
            break;

            case GST_MESSAGE_EOS:
               m_pendingState = m_state = QAudioDecoder::StoppedState;
               emit finished();
               emit stateChanged(m_state);
               break;

            case GST_MESSAGE_ERROR: {
               GError *err;
               gchar *debug;
               gst_message_parse_error(gm, &err, &debug);

               if (err->domain == GST_STREAM_ERROR && err->code == GST_STREAM_ERROR_CODEC_NOT_FOUND) {
                  processInvalidMedia(QAudioDecoder::FormatError, tr("Unable to play stream of type: <unknown>"));
               } else {
                  processInvalidMedia(QAudioDecoder::ResourceError, QString::fromUtf8(err->message));
               }

               qWarning() << "Error:" << QString::fromUtf8(err->message);
               g_error_free(err);
               g_free(debug);
            }
            break;

            case GST_MESSAGE_WARNING: {
               GError *err;
               gchar *debug;
               gst_message_parse_warning (gm, &err, &debug);
               qWarning() << "Warning:" << QString::fromUtf8(err->message);
               g_error_free (err);
               g_free (debug);
            }
            break;

#ifdef DEBUG_DECODER
            case GST_MESSAGE_INFO: {
               GError *err;
               gchar *debug;
               gst_message_parse_info (gm, &err, &debug);
               qDebug() << "Info:" << QString::fromUtf8(err->message);
               g_error_free (err);
               g_free (debug);
            }
            break;
#endif
            default:
               break;
         }
      } else if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ERROR) {
         GError *err;
         gchar *debug;
         gst_message_parse_error(gm, &err, &debug);
         QAudioDecoder::Error qerror = QAudioDecoder::ResourceError;
         if (err->domain == GST_STREAM_ERROR) {
            switch (err->code) {
               case GST_STREAM_ERROR_DECRYPT:
               case GST_STREAM_ERROR_DECRYPT_NOKEY:
                  qerror = QAudioDecoder::AccessDeniedError;
                  break;
               case GST_STREAM_ERROR_FORMAT:
               case GST_STREAM_ERROR_DEMUX:
               case GST_STREAM_ERROR_DECODE:
               case GST_STREAM_ERROR_WRONG_TYPE:
               case GST_STREAM_ERROR_TYPE_NOT_FOUND:
               case GST_STREAM_ERROR_CODEC_NOT_FOUND:
                  qerror = QAudioDecoder::FormatError;
                  break;
               default:
                  break;
            }
         } else if (err->domain == GST_CORE_ERROR) {
            switch (err->code) {
               case GST_CORE_ERROR_MISSING_PLUGIN:
                  qerror = QAudioDecoder::FormatError;
                  break;
               default:
                  break;
            }
         }

         processInvalidMedia(qerror, QString::fromUtf8(err->message));
         g_error_free(err);
         g_free(debug);
      }
   }

   return false;
}

QString QGstreamerAudioDecoderSession::sourceFilename() const
{
   return mSource;
}

void QGstreamerAudioDecoderSession::setSourceFilename(const QString &fileName)
{
   stop();
   mDevice = nullptr;

#if defined(HAVE_GST_APPSRC)
   if (m_appSrc) {
      m_appSrc->deleteLater();
   }
   m_appSrc = nullptr;
#endif

   bool isSignalRequired = (mSource != fileName);
   mSource = fileName;
   if (isSignalRequired) {
      emit sourceChanged();
   }
}

QIODevice *QGstreamerAudioDecoderSession::sourceDevice() const
{
   return mDevice;
}

void QGstreamerAudioDecoderSession::setSourceDevice(QIODevice *device)
{
   stop();
   mSource.clear();
   bool isSignalRequired = (mDevice != device);
   mDevice = device;
   if (isSignalRequired) {
      emit sourceChanged();
   }
}

void QGstreamerAudioDecoderSession::start()
{
   if (!m_playbin) {
      processInvalidMedia(QAudioDecoder::ResourceError, "Playbin element is not valid");
      return;
   }

   addAppSink();

   if (!mSource.isEmpty()) {
      g_object_set(G_OBJECT(m_playbin), "uri", QUrl::fromLocalFile(mSource).toEncoded().constData(), nullptr);
   } else if (mDevice) {
#if defined(HAVE_GST_APPSRC)
      // make sure we can read from device
      if (!mDevice->isOpen() || !mDevice->isReadable()) {
         processInvalidMedia(QAudioDecoder::AccessDeniedError, "Unable to read from specified device");
         return;
      }

      if (!m_appSrc) {
         m_appSrc = new QGstAppSrc(this);
      }
      m_appSrc->setStream(mDevice);

      g_object_set(G_OBJECT(m_playbin), "uri", "appsrc://", nullptr);
#endif
   } else {
      return;
   }

   // Set audio format
   if (m_appSink) {
      if (mFormat.isValid()) {
         setAudioFlags(false);
         GstCaps *caps = QGstUtils::capsForAudioFormat(mFormat);
         gst_app_sink_set_caps(m_appSink, caps);
         gst_caps_unref(caps);
      } else {
         // We want whatever the native audio format is
         setAudioFlags(true);
         gst_app_sink_set_caps(m_appSink, nullptr);
      }
   }

   m_pendingState = QAudioDecoder::DecodingState;
   if (gst_element_set_state(m_playbin, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
      qWarning() << "GStreamer; Unable to start decoding process";
      m_pendingState = m_state = QAudioDecoder::StoppedState;

      emit stateChanged(m_state);
   }
}

void QGstreamerAudioDecoderSession::stop()
{
   if (m_playbin) {
      gst_element_set_state(m_playbin, GST_STATE_NULL);
      removeAppSink();

      QAudioDecoder::State oldState = m_state;
      m_pendingState = m_state = QAudioDecoder::StoppedState;

      // GStreamer thread is stopped. Can safely access m_buffersAvailable
      if (m_buffersAvailable != 0) {
         m_buffersAvailable = 0;
         emit bufferAvailableChanged(false);
      }

      if (m_position != -1) {
         m_position = -1;
         emit positionChanged(m_position);
      }

      if (m_duration != -1) {
         m_duration = -1;
         emit durationChanged(m_duration);
      }

      if (oldState != m_state) {
         emit stateChanged(m_state);
      }
   }
}

QAudioFormat QGstreamerAudioDecoderSession::audioFormat() const
{
   return mFormat;
}

void QGstreamerAudioDecoderSession::setAudioFormat(const QAudioFormat &format)
{
   if (mFormat != format) {
      mFormat = format;
      emit formatChanged(mFormat);
   }
}

QAudioBuffer QGstreamerAudioDecoderSession::read()
{
   QAudioBuffer audioBuffer;

   int buffersAvailable;
   {
      QMutexLocker locker(&m_buffersMutex);
      buffersAvailable = m_buffersAvailable;

      // need to decrement before pulling a buffer
      // to make sure assert in QGstreamerAudioDecoderSession::new_buffer works
      m_buffersAvailable--;
   }


   if (buffersAvailable) {
      if (buffersAvailable == 1) {
         emit bufferAvailableChanged(false);
      }

      const char *bufferData = nullptr;
      int bufferSize = 0;

#if GST_CHECK_VERSION(1,0,0)
      GstSample *sample = gst_app_sink_pull_sample(m_appSink);
      GstBuffer *buffer = gst_sample_get_buffer(sample);
      GstMapInfo mapInfo;
      gst_buffer_map(buffer, &mapInfo, GST_MAP_READ);
      bufferData = (const char *)mapInfo.data;
      bufferSize = mapInfo.size;
      QAudioFormat format = QGstUtils::audioFormatForSample(sample);
#else
      GstBuffer *buffer = gst_app_sink_pull_buffer(m_appSink);
      bufferData = (const char *)buffer->data;
      bufferSize = buffer->size;
      QAudioFormat format = QGstUtils::audioFormatForBuffer(buffer);
#endif

      if (format.isValid()) {
         // XXX At the moment we have to copy data from GstBuffer into QAudioBuffer.
         // We could improve performance by implementing QAbstractAudioBuffer for GstBuffer.
         qint64 position = getPositionFromBuffer(buffer);
         audioBuffer = QAudioBuffer(QByteArray((const char *)bufferData, bufferSize), format, position);
         position /= 1000; // convert to milliseconds
         if (position != m_position) {
            m_position = position;
            emit positionChanged(m_position);
         }
      }
#if GST_CHECK_VERSION(1,0,0)
      gst_sample_unref(sample);
#else
      gst_buffer_unref(buffer);
#endif
   }

   return audioBuffer;
}

bool QGstreamerAudioDecoderSession::bufferAvailable() const
{
   QMutexLocker locker(&m_buffersMutex);
   return m_buffersAvailable > 0;
}

qint64 QGstreamerAudioDecoderSession::position() const
{
   return m_position;
}

qint64 QGstreamerAudioDecoderSession::duration() const
{
   return m_duration;
}

void QGstreamerAudioDecoderSession::processInvalidMedia(QAudioDecoder::Error errorCode, const QString &errorString)
{
   stop();
   emit error(int(errorCode), errorString);
}

GstFlowReturn QGstreamerAudioDecoderSession::new_sample(GstAppSink *, gpointer user_data)
{
   // "Note that the preroll buffer will also be returned as the first buffer when calling gst_app_sink_pull_buffer()."
   QGstreamerAudioDecoderSession *session = reinterpret_cast<QGstreamerAudioDecoderSession *>(user_data);

   int buffersAvailable;
   {
      QMutexLocker locker(&session->m_buffersMutex);
      buffersAvailable = session->m_buffersAvailable;
      session->m_buffersAvailable++;
      Q_ASSERT(session->m_buffersAvailable <= MAX_BUFFERS_IN_QUEUE);
   }

   if (!buffersAvailable) {
      QMetaObject::invokeMethod(session, "bufferAvailableChanged", Qt::QueuedConnection, Q_ARG(bool, true));
   }

   QMetaObject::invokeMethod(session, "bufferReady", Qt::QueuedConnection);
   return GST_FLOW_OK;
}

void QGstreamerAudioDecoderSession::setAudioFlags(bool wantNativeAudio)
{
   int flags = 0;
   if (m_playbin) {
      g_object_get(G_OBJECT(m_playbin), "flags", &flags, nullptr);
      // make sure not to use GST_PLAY_FLAG_NATIVE_AUDIO unless desired
      // it prevents audio format conversion
      flags &= ~(GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_NATIVE_VIDEO | GST_PLAY_FLAG_TEXT | GST_PLAY_FLAG_VIS | GST_PLAY_FLAG_NATIVE_AUDIO);
      flags |= GST_PLAY_FLAG_AUDIO;
      if (wantNativeAudio) {
         flags |= GST_PLAY_FLAG_NATIVE_AUDIO;
      }
      g_object_set(G_OBJECT(m_playbin), "flags", flags, nullptr);
   }
}

void QGstreamerAudioDecoderSession::addAppSink()
{
   if (m_appSink) {
      return;
   }

   m_appSink = (GstAppSink *)gst_element_factory_make("appsink", nullptr);

   GstAppSinkCallbacks callbacks;
   memset(&callbacks, 0, sizeof(callbacks));

#if GST_CHECK_VERSION(1,0,0)
   callbacks.new_sample = &new_sample;
#else
   callbacks.new_buffer = &new_sample;
#endif

   gst_app_sink_set_callbacks(m_appSink, &callbacks, this, nullptr);
   gst_app_sink_set_max_buffers(m_appSink, MAX_BUFFERS_IN_QUEUE);
   gst_base_sink_set_sync(GST_BASE_SINK(m_appSink), FALSE);

   gst_bin_add(GST_BIN(m_outputBin), GST_ELEMENT(m_appSink));
   gst_element_link(m_audioConvert, GST_ELEMENT(m_appSink));
}

void QGstreamerAudioDecoderSession::removeAppSink()
{
   if (!m_appSink) {
      return;
   }

   gst_element_unlink(m_audioConvert, GST_ELEMENT(m_appSink));
   gst_bin_remove(GST_BIN(m_outputBin), GST_ELEMENT(m_appSink));

   m_appSink = nullptr;
}

void QGstreamerAudioDecoderSession::updateDuration()
{
   gint64 gstDuration = 0;
   int duration = -1;

   if (m_playbin && qt_gst_element_query_duration(m_playbin, GST_FORMAT_TIME, &gstDuration)) {
      duration = gstDuration / 1000000;
   }

   if (m_duration != duration) {
      m_duration = duration;
      emit durationChanged(m_duration);
   }

   if (m_duration > 0) {
      m_durationQueries = 0;
   }

   if (m_durationQueries > 0) {
      //increase delay between duration requests
      int delay = 25 << (5 - m_durationQueries);
      QTimer::singleShot(delay, this, SLOT(updateDuration()));
      m_durationQueries--;
   }
}

qint64 QGstreamerAudioDecoderSession::getPositionFromBuffer(GstBuffer *buffer)
{
   qint64 position = GST_BUFFER_TIMESTAMP(buffer);
   if (position >= 0) {
      position = position / G_GINT64_CONSTANT(1000);   // microseconds
   } else {
      position = -1;
   }
   return position;
}

