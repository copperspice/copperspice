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

#include <qgstreamerplayersession.h>

#include <qmediametadata.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qsize.h>
#include <qtimer.h>
#include <qdebug.h>
#include <qdir.h>
#include <qstandardpaths.h>

#include <qgstreamerbushelper_p.h>
#include <qgstreameraudioprobecontrol_p.h>
#include <qgstreamervideoprobecontrol_p.h>
#include <qgstreamervideorendererinterface_p.h>

#if ! GST_CHECK_VERSION(1,0,0)
#include <gstvideoconnector_p.h>
#endif

#include <qgstutils_p.h>
#include <playlistfileparser_p.h>
#include <qgstutils_p.h>

#include <gst/gstvalue.h>
#include <gst/base/gstbasesrc.h>

// #define DEBUG_PLAYBIN
// #define DEBUG_VO_BIN_DUMP

static bool usePlaybinVolume()
{
   static enum { Yes, No, Unknown } status = Unknown;
   if (status == Unknown) {
      QByteArray v = qgetenv("QT_GSTREAMER_USE_PLAYBIN_VOLUME");
      bool value = !v.isEmpty() && v != "0" && v != "false";
      if (value) {
         status = Yes;
      } else {
         status = No;
      }
   }
   return status == Yes;
}

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

#if !GST_CHECK_VERSION(1,0,0)
#define DEFAULT_RAW_CAPS \
    "video/x-raw-yuv; " \
    "video/x-raw-rgb; " \
    "video/x-raw-gray; " \
    "video/x-surface; " \
    "video/x-android-buffer; " \
    "audio/x-raw-int; " \
    "audio/x-raw-float; " \
    "text/plain; " \
    "text/x-pango-markup; " \
    "video/x-dvd-subpicture; " \
    "subpicture/x-pgs"

static GstStaticCaps static_RawCaps = GST_STATIC_CAPS(DEFAULT_RAW_CAPS);
#endif

QGstreamerPlayerSession::QGstreamerPlayerSession(QObject *parent)
   : QObject(parent),
     m_state(QMediaPlayer::StoppedState),
     m_pendingState(QMediaPlayer::StoppedState),
     m_busHelper(nullptr),
     m_playbin(nullptr),
     m_videoSink(nullptr),
#if !GST_CHECK_VERSION(1,0,0)
     m_usingColorspaceElement(false),
#endif
     m_pendingVideoSink(nullptr),
     m_nullVideoSink(nullptr),
     m_audioSink(nullptr),
     m_volumeElement(nullptr),
     m_bus(nullptr),
     m_videoOutput(nullptr),
     m_renderer(nullptr),
#if defined(HAVE_GST_APPSRC)
     m_appSrc(nullptr),
#endif
     m_videoProbe(nullptr),
     m_audioProbe(nullptr),
     m_volume(100),
     m_playbackRate(1.0),
     m_muted(false),
     m_audioAvailable(false),
     m_videoAvailable(false),
     m_seekable(false),
     m_lastPosition(0),
     m_duration(-1),
     m_durationQueries(0),
     m_displayPrerolledFrame(true),
     m_sourceType(UnknownSrc),
     m_everPlayed(false),
     m_isLiveSource(false),
     m_isPlaylist(false)
{
   gboolean result = gst_type_find_register(nullptr, "playlist", GST_RANK_MARGINAL, playlistTypeFindFunction,
            nullptr, nullptr, this, nullptr);
   Q_ASSERT(result == TRUE);

   (void) result;

   m_playbin = gst_element_factory_make(QT_GSTREAMER_PLAYBIN_ELEMENT_NAME, nullptr);

   if (m_playbin) {
      //GST_PLAY_FLAG_NATIVE_VIDEO omits configuration of ffmpegcolorspace and videoscale,
      //since those elements are included in the video output bin when necessary.
#ifdef Q_WS_MAEMO_6
      int flags = GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO |
         GST_PLAY_FLAG_NATIVE_VIDEO | GST_PLAY_FLAG_NATIVE_AUDIO;
#else
      int flags = GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO;
      QByteArray envFlags = qgetenv("QT_GSTREAMER_PLAYBIN_FLAGS");

      if (!envFlags.isEmpty()) {
         flags |= envFlags.toInt();
#if !GST_CHECK_VERSION(1,0,0)
      } else {
         flags |= GST_PLAY_FLAG_NATIVE_VIDEO;
#endif
      }
#endif
      g_object_set(G_OBJECT(m_playbin), "flags", flags, nullptr);

      GstElement *audioSink = gst_element_factory_make("autoaudiosink", "audiosink");
      if (audioSink) {
         if (usePlaybinVolume()) {
            m_audioSink = audioSink;
            m_volumeElement = m_playbin;
         } else {
            m_volumeElement = gst_element_factory_make("volume", "volumeelement");
            if (m_volumeElement) {
               m_audioSink = gst_bin_new("audio-output-bin");

               gst_bin_add_many(GST_BIN(m_audioSink), m_volumeElement, audioSink, nullptr);
               gst_element_link(m_volumeElement, audioSink);

               GstPad *pad = gst_element_get_static_pad(m_volumeElement, "sink");
               gst_element_add_pad(GST_ELEMENT(m_audioSink), gst_ghost_pad_new("sink", pad));
               gst_object_unref(GST_OBJECT(pad));
            } else {
               m_audioSink = audioSink;
               m_volumeElement = m_playbin;
            }
         }

         g_object_set(G_OBJECT(m_playbin), "audio-sink", m_audioSink, nullptr);
         addAudioBufferProbe();
      }
   }

#if GST_CHECK_VERSION(1,0,0)
   m_videoIdentity = gst_element_factory_make("identity", nullptr); // floating ref
#else
   m_videoIdentity = GST_ELEMENT(g_object_new(gst_video_connector_get_type(), nullptr)); // floating ref
   g_signal_connect(G_OBJECT(m_videoIdentity), "connection-failed", G_CALLBACK(insertColorSpaceElement), (gpointer)this);
   m_colorSpace = gst_element_factory_make(QT_GSTREAMER_COLORCONVERSION_ELEMENT_NAME, "ffmpegcolorspace-vo");

   // might not get a parent, take ownership to avoid leak
   qt_gst_object_ref_sink(GST_OBJECT(m_colorSpace));
#endif

   m_nullVideoSink = gst_element_factory_make("fakesink", nullptr);
   g_object_set(G_OBJECT(m_nullVideoSink), "sync", true, nullptr);
   gst_object_ref(GST_OBJECT(m_nullVideoSink));

   m_videoOutputBin = gst_bin_new("video-output-bin");
   // might not get a parent, take ownership to avoid leak
   qt_gst_object_ref_sink(GST_OBJECT(m_videoOutputBin));
   gst_bin_add_many(GST_BIN(m_videoOutputBin), m_videoIdentity, m_nullVideoSink, nullptr);
   gst_element_link(m_videoIdentity, m_nullVideoSink);

   m_videoSink = m_nullVideoSink;

   // add ghostpads
   GstPad *pad = gst_element_get_static_pad(m_videoIdentity, "sink");
   gst_element_add_pad(GST_ELEMENT(m_videoOutputBin), gst_ghost_pad_new("sink", pad));
   gst_object_unref(GST_OBJECT(pad));

   if (m_playbin != nullptr) {
      // Sort out messages
      m_bus = gst_element_get_bus(m_playbin);
      m_busHelper = new QGstreamerBusHelper(m_bus, this);
      m_busHelper->installMessageFilter(this);

      g_object_set(G_OBJECT(m_playbin), "video-sink", m_videoOutputBin, nullptr);

      g_signal_connect(G_OBJECT(m_playbin), "notify::source", G_CALLBACK(playbinNotifySource), this);
      g_signal_connect(G_OBJECT(m_playbin), "element-added",  G_CALLBACK(handleElementAdded), this);

      if (usePlaybinVolume()) {
         updateVolume();
         updateMuted();
         g_signal_connect(G_OBJECT(m_playbin), "notify::volume", G_CALLBACK(handleVolumeChange), this);
         g_signal_connect(G_OBJECT(m_playbin), "notify::mute", G_CALLBACK(handleMutedChange), this);
      }

      g_signal_connect(G_OBJECT(m_playbin), "video-changed", G_CALLBACK(handleStreamsChange), this);
      g_signal_connect(G_OBJECT(m_playbin), "audio-changed", G_CALLBACK(handleStreamsChange), this);
      g_signal_connect(G_OBJECT(m_playbin), "text-changed", G_CALLBACK(handleStreamsChange), this);

#if defined(HAVE_GST_APPSRC)
      g_signal_connect(G_OBJECT(m_playbin), "deep-notify::source", G_CALLBACK(configureAppSrcElement), this);
#endif
   }
}

QGstreamerPlayerSession::~QGstreamerPlayerSession()
{
   if (m_playbin) {
      stop();

      removeVideoBufferProbe();
      removeAudioBufferProbe();

      delete m_busHelper;
      gst_object_unref(GST_OBJECT(m_bus));
      gst_object_unref(GST_OBJECT(m_playbin));
#if !GST_CHECK_VERSION(1,0,0)
      gst_object_unref(GST_OBJECT(m_colorSpace));
#endif
      gst_object_unref(GST_OBJECT(m_nullVideoSink));
      gst_object_unref(GST_OBJECT(m_videoOutputBin));
   }
}

GstElement *QGstreamerPlayerSession::playbin() const
{
   return m_playbin;
}

#if defined(HAVE_GST_APPSRC)
void QGstreamerPlayerSession::configureAppSrcElement(GObject *object, GObject *orig, GParamSpec *pspec, QGstreamerPlayerSession *self)
{
   (void) object;
   (void) pspec;

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

void QGstreamerPlayerSession::loadFromStream(const QNetworkRequest &request, QIODevice *appSrcStream)
{
#if defined(HAVE_GST_APPSRC)

#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif

   m_request = request;
   m_duration = -1;
   m_lastPosition = 0;
   m_isPlaylist = false;

   if (!m_appSrc) {
      m_appSrc = new QGstAppSrc(this);
   }
   m_appSrc->setStream(appSrcStream);

   if (m_playbin) {
      m_tags.clear();
      emit tagsChanged();

      g_object_set(G_OBJECT(m_playbin), "uri", "appsrc://", nullptr);

      if (!m_streamTypes.isEmpty()) {
         m_streamProperties.clear();
         m_streamTypes.clear();

         emit streamsChanged();
      }
   }

#else
   (void) request;
   (void) appSrcStream;
#endif
}

void QGstreamerPlayerSession::loadFromUri(const QNetworkRequest &request)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << request.url();
#endif
   m_request = request;
   m_duration = -1;
   m_lastPosition = 0;
   m_isPlaylist = false;

#if defined(HAVE_GST_APPSRC)
   if (m_appSrc) {
      m_appSrc->deleteLater();
      m_appSrc = 0;
   }
#endif

   if (m_playbin) {
      m_tags.clear();
      emit tagsChanged();

      g_object_set(G_OBJECT(m_playbin), "uri", m_request.url().toEncoded().constData(), nullptr);

      if (!m_streamTypes.isEmpty()) {
         m_streamProperties.clear();
         m_streamTypes.clear();

         emit streamsChanged();
      }
   }
}

qint64 QGstreamerPlayerSession::duration() const
{
   return m_duration;
}

qint64 QGstreamerPlayerSession::position() const
{
   gint64      position = 0;

   if (m_playbin && qt_gst_element_query_position(m_playbin, GST_FORMAT_TIME, &position)) {
      m_lastPosition = position / 1000000;
   }
   return m_lastPosition;
}

qreal QGstreamerPlayerSession::playbackRate() const
{
   return m_playbackRate;
}

void QGstreamerPlayerSession::setPlaybackRate(qreal rate)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << rate;
#endif
   if (!qFuzzyCompare(m_playbackRate, rate)) {
      m_playbackRate = rate;
      if (m_playbin && m_seekable) {
         gst_element_seek(m_playbin, rate, GST_FORMAT_TIME,
            GstSeekFlags(GST_SEEK_FLAG_FLUSH),
            GST_SEEK_TYPE_NONE, 0,
            GST_SEEK_TYPE_NONE, 0 );
      }
      emit playbackRateChanged(m_playbackRate);
   }
}

QMediaTimeRange QGstreamerPlayerSession::availablePlaybackRanges() const
{
   QMediaTimeRange ranges;

   if (duration() <= 0) {
      return ranges;
   }

#if GST_CHECK_VERSION(0, 10, 31)
   //GST_FORMAT_TIME would be more appropriate, but unfortunately it's not supported.
   //with GST_FORMAT_PERCENT media is treated as encoded with constant bitrate.
   GstQuery *query = gst_query_new_buffering(GST_FORMAT_PERCENT);

   if (!gst_element_query(m_playbin, query)) {
      gst_query_unref(query);
      return ranges;
   }

   gint64 rangeStart = 0;
   gint64 rangeStop = 0;
   for (guint index = 0; index < gst_query_get_n_buffering_ranges(query); index++) {
      if (gst_query_parse_nth_buffering_range(query, index, &rangeStart, &rangeStop))
         ranges.addInterval(rangeStart * duration() / 100,
            rangeStop * duration() / 100);
   }

   gst_query_unref(query);
#endif

   if (ranges.isEmpty() && !isLiveSource() && isSeekable()) {
      ranges.addInterval(0, duration());
   }

#ifdef DEBUG_PLAYBIN
   qDebug() << ranges;
#endif

   return ranges;
}

int QGstreamerPlayerSession::activeStream(QMediaStreamsControl::StreamType streamType) const
{
   int streamNumber = -1;
   if (m_playbin) {
      switch (streamType) {
         case QMediaStreamsControl::AudioStream:
            g_object_get(G_OBJECT(m_playbin), "current-audio", &streamNumber, nullptr);
            break;
         case QMediaStreamsControl::VideoStream:
            g_object_get(G_OBJECT(m_playbin), "current-video", &streamNumber, nullptr);
            break;
         case QMediaStreamsControl::SubPictureStream:
            g_object_get(G_OBJECT(m_playbin), "current-text", &streamNumber, nullptr);
            break;
         default:
            break;
      }
   }

   if (streamNumber >= 0) {
      streamNumber += m_playbin2StreamOffset.value(streamType, 0);
   }

   return streamNumber;
}

void QGstreamerPlayerSession::setActiveStream(QMediaStreamsControl::StreamType streamType, int streamNumber)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << streamType << streamNumber;
#endif

   if (streamNumber >= 0) {
      streamNumber -= m_playbin2StreamOffset.value(streamType, 0);
   }

   if (m_playbin) {
      switch (streamType) {
         case QMediaStreamsControl::AudioStream:
            g_object_set(G_OBJECT(m_playbin), "current-audio", streamNumber, nullptr);
            break;
         case QMediaStreamsControl::VideoStream:
            g_object_set(G_OBJECT(m_playbin), "current-video", streamNumber, nullptr);
            break;
         case QMediaStreamsControl::SubPictureStream:
            g_object_set(G_OBJECT(m_playbin), "current-text", streamNumber, nullptr);
            break;
         default:
            break;
      }
   }
}

int QGstreamerPlayerSession::volume() const
{
   return m_volume;
}

bool QGstreamerPlayerSession::isMuted() const
{
   return m_muted;
}

bool QGstreamerPlayerSession::isAudioAvailable() const
{
   return m_audioAvailable;
}

#if GST_CHECK_VERSION(1,0,0)
static GstPadProbeReturn block_pad_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
#else
static void block_pad_cb(GstPad *pad, gboolean blocked, gpointer user_data)
#endif
{
   (void) pad;

#if GST_CHECK_VERSION(1,0,0)
   (void) info;
   (void) user_data;
   return GST_PAD_PROBE_OK;

#else
#ifdef DEBUG_PLAYBIN
   qDebug() << "block_pad_cb, blocked:" << blocked;
#endif

   if (blocked && user_data) {
      QGstreamerPlayerSession *session = reinterpret_cast<QGstreamerPlayerSession *>(user_data);
      QMetaObject::invokeMethod(session, "finishVideoOutputChange", Qt::QueuedConnection);
   }
#endif
}

void QGstreamerPlayerSession::updateVideoRenderer()
{
#ifdef DEBUG_PLAYBIN
   qDebug() << "Video sink has chaged, reload video output";
#endif

   if (m_videoOutput) {
      setVideoRenderer(m_videoOutput);
   }
}

void QGstreamerPlayerSession::setVideoRenderer(QObject *videoOutput)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif

   if (m_videoOutput != videoOutput) {
      if (m_videoOutput) {
         disconnect(m_videoOutput, SIGNAL(sinkChanged()),      this, SLOT(updateVideoRenderer()));
         disconnect(m_videoOutput, SIGNAL(readyChanged(bool)), this, SLOT(updateVideoRenderer()));

         m_busHelper->removeMessageFilter(m_videoOutput);
      }

      m_videoOutput = videoOutput;

      if (m_videoOutput) {
         connect(m_videoOutput, SIGNAL(sinkChanged()),      this, SLOT(updateVideoRenderer()));
         connect(m_videoOutput, SIGNAL(readyChanged(bool)), this, SLOT(updateVideoRenderer()));

         m_busHelper->installMessageFilter(m_videoOutput);
      }
   }

   QGstreamerVideoRendererInterface *renderer = dynamic_cast<QGstreamerVideoRendererInterface *>(videoOutput);
   m_renderer = renderer;

#ifdef DEBUG_VO_BIN_DUMP
   gst_debug_bin_to_dot_file_with_ts(GST_BIN(m_playbin), GstDebugGraphDetails(GST_DEBUG_GRAPH_SHOW_ALL), "playbin_set");
   // GST_DEBUG_GRAPH_SHOW_MEDIA_TYPE | GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS | GST_DEBUG_GRAPH_SHOW_STATES
#endif

   GstElement *videoSink = nullptr;
   if (m_renderer && m_renderer->isReady()) {
      videoSink = m_renderer->videoSink();
   }

   if (!videoSink) {
      videoSink = m_nullVideoSink;
   }

#ifdef DEBUG_PLAYBIN
   qDebug() << "Set video output:" << videoOutput;
   qDebug() << "Current sink:" << (m_videoSink ? GST_ELEMENT_NAME(m_videoSink) : "") <<  m_videoSink
      << "pending:" << (m_pendingVideoSink ? GST_ELEMENT_NAME(m_pendingVideoSink) : "") << m_pendingVideoSink
      << "new sink:" << (videoSink ? GST_ELEMENT_NAME(videoSink) : "") << videoSink;
#endif

   if (m_pendingVideoSink == videoSink || (m_pendingVideoSink == nullptr && m_videoSink == videoSink)) {
#ifdef DEBUG_PLAYBIN
      qDebug() << "Video sink has not changed, skip video output reconfiguration";
#endif
      return;
   }

#ifdef DEBUG_PLAYBIN
   qDebug() << "Reconfigure video output";
#endif

   if (m_state == QMediaPlayer::StoppedState) {
#ifdef DEBUG_PLAYBIN
      qDebug() << "The pipeline has not started yet, pending state:" << m_pendingState;
#endif
      //the pipeline has not started yet
      flushVideoProbes();
      m_pendingVideoSink = nullptr;
      gst_element_set_state(m_videoSink, GST_STATE_NULL);
      gst_element_set_state(m_playbin, GST_STATE_NULL);

#if !GST_CHECK_VERSION(1,0,0)
      if (m_usingColorspaceElement) {
         gst_element_unlink(m_colorSpace, m_videoSink);
         gst_bin_remove(GST_BIN(m_videoOutputBin), m_colorSpace);
      } else {
         gst_element_unlink(m_videoIdentity, m_videoSink);
      }
#endif

      removeVideoBufferProbe();

      gst_bin_remove(GST_BIN(m_videoOutputBin), m_videoSink);

      m_videoSink = videoSink;

      gst_bin_add(GST_BIN(m_videoOutputBin), m_videoSink);

      bool linked = gst_element_link(m_videoIdentity, m_videoSink);
#if !GST_CHECK_VERSION(1,0,0)
      m_usingColorspaceElement = false;
      if (!linked) {
         m_usingColorspaceElement = true;
#ifdef DEBUG_PLAYBIN
         qDebug() << "Failed to connect video output, inserting the colorspace element.";
#endif
         gst_bin_add(GST_BIN(m_videoOutputBin), m_colorSpace);
         linked = gst_element_link_many(m_videoIdentity, m_colorSpace, m_videoSink, nullptr);
      }
#endif

      if (!linked) {
         qWarning() << "Linking video output element failed";
      }

      if (g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "show-preroll-frame") != nullptr) {
         gboolean value = m_displayPrerolledFrame;
         g_object_set(G_OBJECT(m_videoSink), "show-preroll-frame", value, nullptr);
      }

      addVideoBufferProbe();

      switch (m_pendingState) {
         case QMediaPlayer::PausedState:
            gst_element_set_state(m_playbin, GST_STATE_PAUSED);
            break;
         case QMediaPlayer::PlayingState:
            gst_element_set_state(m_playbin, GST_STATE_PLAYING);
            break;
         default:
            break;
      }

      resumeVideoProbes();

   } else {
      if (m_pendingVideoSink) {
#ifdef DEBUG_PLAYBIN
         qDebug() << "already waiting for pad to be blocked, just change the pending sink";
#endif
         m_pendingVideoSink = videoSink;
         return;
      }

      m_pendingVideoSink = videoSink;

#ifdef DEBUG_PLAYBIN
      qDebug() << "Blocking the video output pad...";
#endif

      //block pads, async to avoid locking in paused state
      GstPad *srcPad = gst_element_get_static_pad(m_videoIdentity, "src");
#if GST_CHECK_VERSION(1,0,0)
      this->pad_probe_id = gst_pad_add_probe(srcPad, (GstPadProbeType)(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BLOCKING),
            block_pad_cb, this, nullptr);
#else
      gst_pad_set_blocked_async(srcPad, true, &block_pad_cb, this);
#endif
      gst_object_unref(GST_OBJECT(srcPad));

      //Unpause the sink to avoid waiting until the buffer is processed
      //while the sink is paused. The pad will be blocked as soon as the current
      //buffer is processed.
      if (m_state == QMediaPlayer::PausedState) {
#ifdef DEBUG_PLAYBIN
         qDebug() << "Starting video output to avoid blocking in paused state...";
#endif
         gst_element_set_state(m_videoSink, GST_STATE_PLAYING);
      }
   }
}

void QGstreamerPlayerSession::finishVideoOutputChange()
{
   if (!m_pendingVideoSink) {
      return;
   }

#ifdef DEBUG_PLAYBIN
   qDebug() << "finishVideoOutputChange" << m_pendingVideoSink;
#endif

   GstPad *srcPad = gst_element_get_static_pad(m_videoIdentity, "src");

   if (!gst_pad_is_blocked(srcPad)) {
      //pad is not blocked, it's possible to swap outputs only in the null state
      qWarning() << "Pad is not blocked yet, could not switch video sink";
      GstState identityElementState = GST_STATE_NULL;
      gst_element_get_state(m_videoIdentity, &identityElementState, nullptr, GST_CLOCK_TIME_NONE);
      if (identityElementState != GST_STATE_NULL) {
         gst_object_unref(GST_OBJECT(srcPad));
         return; //can't change vo yet, received async call from the previous change
      }
   }

   if (m_pendingVideoSink == m_videoSink) {
      qDebug() << "Abort, no change";
      //video output was change back to the current one,
      //no need to torment the pipeline, just unblock the pad
      if (gst_pad_is_blocked(srcPad))
#if GST_CHECK_VERSION(1,0,0)
         gst_pad_remove_probe(srcPad, this->pad_probe_id);
#else
         gst_pad_set_blocked_async(srcPad, false, &block_pad_cb, nullptr);
#endif

      m_pendingVideoSink = nullptr;
      gst_object_unref(GST_OBJECT(srcPad));
      return;
   }

#if !GST_CHECK_VERSION(1,0,0)
   if (m_usingColorspaceElement) {
      gst_element_set_state(m_colorSpace, GST_STATE_NULL);
      gst_element_set_state(m_videoSink, GST_STATE_NULL);

      gst_element_unlink(m_colorSpace, m_videoSink);
      gst_bin_remove(GST_BIN(m_videoOutputBin), m_colorSpace);
   } else {
#else
   {
#endif
      gst_element_set_state(m_videoSink, GST_STATE_NULL);
      gst_element_unlink(m_videoIdentity, m_videoSink);
   }

   removeVideoBufferProbe();

   gst_bin_remove(GST_BIN(m_videoOutputBin), m_videoSink);

   m_videoSink = m_pendingVideoSink;
   m_pendingVideoSink = nullptr;

   gst_bin_add(GST_BIN(m_videoOutputBin), m_videoSink);

   addVideoBufferProbe();

   bool linked = gst_element_link(m_videoIdentity, m_videoSink);
#if !GST_CHECK_VERSION(1,0,0)
   m_usingColorspaceElement = false;
   if (!linked) {
      m_usingColorspaceElement = true;
#ifdef DEBUG_PLAYBIN
      qDebug() << "Failed to connect video output, inserting the colorspace element.";
#endif
      gst_bin_add(GST_BIN(m_videoOutputBin), m_colorSpace);
      linked = gst_element_link_many(m_videoIdentity, m_colorSpace, m_videoSink, nullptr);
   }
#endif

   if (!linked) {
      qWarning() << "Linking video output element failed";
   }

#ifdef DEBUG_PLAYBIN
   qDebug() << "notify the video connector it has to emit a new segment message...";
#endif

#if !GST_CHECK_VERSION(1,0,0)
   //it's necessary to send a new segment event just before
   //the first buffer pushed to the new sink
   g_signal_emit_by_name(m_videoIdentity,
      "resend-new-segment",
      true //emit connection-failed signal
      //to have a chance to insert colorspace element
   );
#endif

   GstState state = GST_STATE_VOID_PENDING;

   switch (m_pendingState) {
      case QMediaPlayer::StoppedState:
         state = GST_STATE_NULL;
         break;
      case QMediaPlayer::PausedState:
         state = GST_STATE_PAUSED;
         break;
      case QMediaPlayer::PlayingState:
         state = GST_STATE_PLAYING;
         break;
   }

#if !GST_CHECK_VERSION(1,0,0)
   if (m_usingColorspaceElement) {
      gst_element_set_state(m_colorSpace, state);
   }
#endif

   gst_element_set_state(m_videoSink, state);

   if (state == GST_STATE_NULL) {
      flushVideoProbes();
   }

   // Set state change that was deferred due the video output
   // change being pending
   gst_element_set_state(m_playbin, state);

   if (state != GST_STATE_NULL) {
      resumeVideoProbes();
   }

   //don't have to wait here, it will unblock eventually
   if (gst_pad_is_blocked(srcPad))
#if GST_CHECK_VERSION(1,0,0)
      gst_pad_remove_probe(srcPad, this->pad_probe_id);
#else
      gst_pad_set_blocked_async(srcPad, false, &block_pad_cb, nullptr);
#endif

   gst_object_unref(GST_OBJECT(srcPad));

#ifdef DEBUG_VO_BIN_DUMP
   gst_debug_bin_to_dot_file_with_ts(GST_BIN(m_playbin),
      GstDebugGraphDetails(GST_DEBUG_GRAPH_SHOW_ALL), "playbin_finish");
#endif
}

#if !GST_CHECK_VERSION(1,0,0)

void QGstreamerPlayerSession::insertColorSpaceElement(GstElement *element, gpointer data)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif

   (void) element;

   QGstreamerPlayerSession *session = reinterpret_cast<QGstreamerPlayerSession *>(data);

   if (session->m_usingColorspaceElement) {
      return;
   }
   session->m_usingColorspaceElement = true;

#ifdef DEBUG_PLAYBIN
   qDebug() << "Failed to connect video output, inserting the colorspace elemnt.";
   qDebug() << "notify the video connector it has to emit a new segment message...";
#endif
   //it's necessary to send a new segment event just before
   //the first buffer pushed to the new sink
   g_signal_emit_by_name(session->m_videoIdentity,
      "resend-new-segment",
      false // don't emit connection-failed signal
   );

   gst_element_unlink(session->m_videoIdentity, session->m_videoSink);
   gst_bin_add(GST_BIN(session->m_videoOutputBin), session->m_colorSpace);
   gst_element_link_many(session->m_videoIdentity, session->m_colorSpace, session->m_videoSink, nullptr);

   GstState state = GST_STATE_VOID_PENDING;

   switch (session->m_pendingState) {
      case QMediaPlayer::StoppedState:
         state = GST_STATE_NULL;
         break;
      case QMediaPlayer::PausedState:
         state = GST_STATE_PAUSED;
         break;
      case QMediaPlayer::PlayingState:
         state = GST_STATE_PLAYING;
         break;
   }

   gst_element_set_state(session->m_colorSpace, state);
}

#endif

bool QGstreamerPlayerSession::isVideoAvailable() const
{
   return m_videoAvailable;
}

bool QGstreamerPlayerSession::isSeekable() const
{
   return m_seekable;
}

bool QGstreamerPlayerSession::play()
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif

   m_everPlayed = false;
   if (m_playbin) {
      m_pendingState = QMediaPlayer::PlayingState;
      if (gst_element_set_state(m_playbin, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
         if (!m_isPlaylist) {
            qWarning() << "GStreamer; Unable to play -" << m_request.url().toString();
            m_pendingState = m_state = QMediaPlayer::StoppedState;
            emit stateChanged(m_state);
         } else {
            return true;
         }
      } else {
         resumeVideoProbes();
         return true;
      }
   }

   return false;
}

bool QGstreamerPlayerSession::pause()
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif
   if (m_playbin) {
      m_pendingState = QMediaPlayer::PausedState;
      if (m_pendingVideoSink != nullptr) {
         return true;
      }

      if (gst_element_set_state(m_playbin, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE) {
         if (!m_isPlaylist) {
            qWarning() << "GStreamer; Unable to pause -" << m_request.url().toString();
            m_pendingState = m_state = QMediaPlayer::StoppedState;
            emit stateChanged(m_state);
         } else {
            return true;
         }
      } else {
         resumeVideoProbes();
         return true;
      }
   }

   return false;
}

void QGstreamerPlayerSession::stop()
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif
   m_everPlayed = false;
   if (m_playbin) {

      if (m_renderer) {
         m_renderer->stopRenderer();
      }

      flushVideoProbes();
      gst_element_set_state(m_playbin, GST_STATE_NULL);

      m_lastPosition = 0;
      QMediaPlayer::State oldState = m_state;
      m_pendingState = m_state = QMediaPlayer::StoppedState;

      finishVideoOutputChange();

      //we have to do it here, since gstreamer will not emit bus messages any more
      setSeekable(false);
      if (oldState != m_state) {
         emit stateChanged(m_state);
      }
   }
}

bool QGstreamerPlayerSession::seek(qint64 ms)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << ms;
#endif
   //seek locks when the video output sink is changing and pad is blocked
   if (m_playbin && !m_pendingVideoSink && m_state != QMediaPlayer::StoppedState && m_seekable) {
      ms = qMax(ms, qint64(0));
      gint64  position = ms * 1000000;
      bool isSeeking = gst_element_seek(m_playbin,
            m_playbackRate,
            GST_FORMAT_TIME,
            GstSeekFlags(GST_SEEK_FLAG_FLUSH),
            GST_SEEK_TYPE_SET,
            position,
            GST_SEEK_TYPE_NONE,
            0);
      if (isSeeking) {
         m_lastPosition = ms;
      }

      return isSeeking;
   }

   return false;
}

void QGstreamerPlayerSession::setVolume(int volume)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << volume;
#endif

   if (m_volume != volume) {
      m_volume = volume;

      if (m_volumeElement) {
         g_object_set(G_OBJECT(m_volumeElement), "volume", m_volume / 100.0, nullptr);
      }

      emit volumeChanged(m_volume);
   }
}

void QGstreamerPlayerSession::setMuted(bool muted)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << muted;
#endif
   if (m_muted != muted) {
      m_muted = muted;

      if (m_volumeElement) {
         g_object_set(G_OBJECT(m_volumeElement), "mute", m_muted ? TRUE : FALSE, nullptr);
      }

      emit mutedStateChanged(m_muted);
   }
}


void QGstreamerPlayerSession::setSeekable(bool seekable)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << seekable;
#endif
   if (seekable != m_seekable) {
      m_seekable = seekable;
      emit seekableChanged(m_seekable);
   }
}

bool QGstreamerPlayerSession::processBusMessage(const QGstreamerMessage &message)
{
   GstMessage *gm = message.rawMessage();
   if (gm) {
      //tag message comes from elements inside playbin, not from playbin itself
      if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_TAG) {
         GstTagList *tag_list;
         gst_message_parse_tag(gm, &tag_list);

         QMap<QByteArray, QVariant> newTags = QGstUtils::gstTagListToMap(tag_list);
         QMap<QByteArray, QVariant>::const_iterator it = newTags.constBegin();
         for ( ; it != newTags.constEnd(); ++it) {
            m_tags.insert(it.key(), it.value());   // overwrite existing tags
         }

         gst_tag_list_free(tag_list);

         emit tagsChanged();
      } else if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_DURATION) {
         updateDuration();
      }

#ifdef DEBUG_PLAYBIN
      if (m_sourceType == MMSSrc && qstrcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(gm)), "source") == 0) {
         qDebug() << "Message from MMSSrc: " << GST_MESSAGE_TYPE(gm);
      } else if (m_sourceType == RTSPSrc && qstrcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(gm)), "source") == 0) {
         qDebug() << "Message from RTSPSrc: " << GST_MESSAGE_TYPE(gm);
      } else {
         qDebug() << "Message from " << GST_OBJECT_NAME(GST_MESSAGE_SRC(gm)) << ":" << GST_MESSAGE_TYPE(gm);
      }
#endif

      if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_BUFFERING) {
         int progress = 0;
         gst_message_parse_buffering(gm, &progress);
         emit bufferingProgressChanged(progress);
      }

      bool handlePlaybin2 = false;
      if (GST_MESSAGE_SRC(gm) == GST_OBJECT_CAST(m_playbin)) {
         switch (GST_MESSAGE_TYPE(gm))  {
            case GST_MESSAGE_STATE_CHANGED: {
               GstState    oldState;
               GstState    newState;
               GstState    pending;

               gst_message_parse_state_changed(gm, &oldState, &newState, &pending);

#ifdef DEBUG_PLAYBIN
               QStringList states;
               states << "GST_STATE_VOID_PENDING" <<  "GST_STATE_NULL" << "GST_STATE_READY" << "GST_STATE_PAUSED" << "GST_STATE_PLAYING";

               qDebug() << QString("state changed: old: %1  new: %2  pending: %3") \
                  .formatArg(states[oldState]) \
                  .formatArg(states[newState]) \
                  .formatArg(states[pending]);
#endif

               switch (newState) {
                  case GST_STATE_VOID_PENDING:
                  case GST_STATE_NULL:
                     setSeekable(false);
                     finishVideoOutputChange();
                     if (m_state != QMediaPlayer::StoppedState) {
                        emit stateChanged(m_state = QMediaPlayer::StoppedState);
                     }
                     break;
                  case GST_STATE_READY:
                     setSeekable(false);
                     if (m_state != QMediaPlayer::StoppedState) {
                        emit stateChanged(m_state = QMediaPlayer::StoppedState);
                     }
                     break;
                  case GST_STATE_PAUSED: {
                     QMediaPlayer::State prevState = m_state;
                     m_state = QMediaPlayer::PausedState;

                     //check for seekable
                     if (oldState == GST_STATE_READY) {
                        if (m_sourceType == SoupHTTPSrc || m_sourceType == MMSSrc) {
                           //since udpsrc is a live source, it is not applicable here
                           m_everPlayed = true;
                        }

                        getStreamsInfo();
                        updateVideoResolutionTag();

                        //gstreamer doesn't give a reliable indication the duration
                        //information is ready, GST_MESSAGE_DURATION is not sent by most elements
                        //the duration is queried up to 5 times with increasing delay
                        m_durationQueries = 5;
                        // This should also update the seekable flag.
                        updateDuration();

                        if (!qFuzzyCompare(m_playbackRate, qreal(1.0))) {
                           qreal rate = m_playbackRate;
                           m_playbackRate = 1.0;
                           setPlaybackRate(rate);
                        }
                     }

                     if (m_state != prevState) {
                        emit stateChanged(m_state);
                     }

                     break;
                  }
                  case GST_STATE_PLAYING:
                     m_everPlayed = true;
                     if (m_state != QMediaPlayer::PlayingState) {
                        emit stateChanged(m_state = QMediaPlayer::PlayingState);

                        // For rtsp streams duration information might not be available
                        // until playback starts.
                        if (m_duration <= 0) {
                           m_durationQueries = 5;
                           updateDuration();
                        }
                     }

                     break;
               }
            }
            break;

            case GST_MESSAGE_EOS:
               emit playbackFinished();
               break;

            case GST_MESSAGE_TAG:
            case GST_MESSAGE_STREAM_STATUS:
            case GST_MESSAGE_UNKNOWN:
               break;
            case GST_MESSAGE_ERROR: {
               GError *err;
               gchar *debug;
               gst_message_parse_error(gm, &err, &debug);
               if (err->domain == GST_STREAM_ERROR && err->code == GST_STREAM_ERROR_CODEC_NOT_FOUND) {
                  processInvalidMedia(QMediaPlayer::FormatError, tr("Cannot play stream of type: <unknown>"));
               } else {
                  processInvalidMedia(QMediaPlayer::ResourceError, QString::fromUtf8(err->message));
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
            case GST_MESSAGE_INFO:
#ifdef DEBUG_PLAYBIN
            {
               GError *err;
               gchar *debug;
               gst_message_parse_info (gm, &err, &debug);
               qDebug() << "Info:" << QString::fromUtf8(err->message);
               g_error_free (err);
               g_free (debug);
            }
#endif
            break;
            case GST_MESSAGE_BUFFERING:
            case GST_MESSAGE_STATE_DIRTY:
            case GST_MESSAGE_STEP_DONE:
            case GST_MESSAGE_CLOCK_PROVIDE:
            case GST_MESSAGE_CLOCK_LOST:
            case GST_MESSAGE_NEW_CLOCK:
            case GST_MESSAGE_STRUCTURE_CHANGE:
            case GST_MESSAGE_APPLICATION:
            case GST_MESSAGE_ELEMENT:
               break;
            case GST_MESSAGE_SEGMENT_START: {
               const GstStructure *structure = gst_message_get_structure(gm);
               qint64 position = g_value_get_int64(gst_structure_get_value(structure, "position"));
               position /= 1000000;
               m_lastPosition = position;
               emit positionChanged(position);
            }
            break;
            case GST_MESSAGE_SEGMENT_DONE:
               break;
            case GST_MESSAGE_LATENCY:
#if GST_CHECK_VERSION(0,10,13)
            case GST_MESSAGE_ASYNC_START:
               break;
            case GST_MESSAGE_ASYNC_DONE: {
               gint64      position = 0;
               if (qt_gst_element_query_position(m_playbin, GST_FORMAT_TIME, &position)) {
                  position /= 1000000;
                  m_lastPosition = position;
                  emit positionChanged(position);
               }
               break;
            }
#if GST_CHECK_VERSION(0,10,23)
            case GST_MESSAGE_REQUEST_STATE:
#endif
#endif
            case GST_MESSAGE_ANY:
               break;
            default:
               break;
         }
      } else if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ERROR) {
         GError *err;
         gchar *debug;
         gst_message_parse_error(gm, &err, &debug);
         // If the source has given up, so do we.
         if (qstrcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(gm)), "source") == 0) {
            bool everPlayed = m_everPlayed;
            // Try and differentiate network related resource errors from the others
            if (!m_request.url().isRelative() && m_request.url().scheme().compare(QLatin1String("file"), Qt::CaseInsensitive) != 0 ) {
               if (everPlayed ||
                  (err->domain == GST_RESOURCE_ERROR && (
                        err->code == GST_RESOURCE_ERROR_BUSY ||
                        err->code == GST_RESOURCE_ERROR_OPEN_READ ||
                        err->code == GST_RESOURCE_ERROR_READ ||
                        err->code == GST_RESOURCE_ERROR_SEEK ||
                        err->code == GST_RESOURCE_ERROR_SYNC))) {
                  processInvalidMedia(QMediaPlayer::NetworkError, QString::fromUtf8(err->message));
               } else {
                  processInvalidMedia(QMediaPlayer::ResourceError, QString::fromUtf8(err->message));
               }
            } else {
               processInvalidMedia(QMediaPlayer::ResourceError, QString::fromUtf8(err->message));
            }
         } else if (err->domain == GST_STREAM_ERROR
            && (err->code == GST_STREAM_ERROR_DECRYPT || err->code == GST_STREAM_ERROR_DECRYPT_NOKEY)) {
            processInvalidMedia(QMediaPlayer::AccessDeniedError, QString::fromUtf8(err->message));
         } else {
            handlePlaybin2 = true;
         }
         if (!handlePlaybin2) {
            qWarning() << "Error:" << QString::fromUtf8(err->message);
         }
         g_error_free(err);
         g_free(debug);
      } else if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ELEMENT
         && qstrcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(gm)), "source") == 0
         && m_sourceType == UDPSrc
         && gst_structure_has_name(gst_message_get_structure(gm), "GstUDPSrcTimeout")) {
         //since udpsrc will not generate an error for the timeout event,
         //we need to process its element message here and treat it as an error.
         processInvalidMedia(m_everPlayed ? QMediaPlayer::NetworkError : QMediaPlayer::ResourceError,
            tr("UDP source timeout"));
      } else {
         handlePlaybin2 = true;
      }

      if (handlePlaybin2) {
         if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_WARNING) {
            GError *err;
            gchar *debug;
            gst_message_parse_warning(gm, &err, &debug);
            if (err->domain == GST_STREAM_ERROR && err->code == GST_STREAM_ERROR_CODEC_NOT_FOUND) {
               emit error(int(QMediaPlayer::FormatError), tr("Cannot play stream of type: <unknown>"));
            }
            // GStreamer shows warning for HTTP playlists
            if (!m_isPlaylist) {
               qWarning() << "Warning:" << QString::fromUtf8(err->message);
            }
            g_error_free(err);
            g_free(debug);
         } else if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ERROR) {
            GError *err;
            gchar *debug;
            gst_message_parse_error(gm, &err, &debug);

            // remember playlist value,
            // it could be set to false after call to processInvalidMedia
            bool isPlaylist = m_isPlaylist;

            // Nearly all errors map to ResourceError
            QMediaPlayer::Error qerror = QMediaPlayer::ResourceError;
            if (err->domain == GST_STREAM_ERROR
               && (err->code == GST_STREAM_ERROR_DECRYPT
                  || err->code == GST_STREAM_ERROR_DECRYPT_NOKEY)) {
               qerror = QMediaPlayer::AccessDeniedError;
            }
            processInvalidMedia(qerror, QString::fromUtf8(err->message));
            if (!isPlaylist) {
               qWarning() << "Error:" << QString::fromUtf8(err->message);
            }

            g_error_free(err);
            g_free(debug);
         }
      }
   }

   return false;
}

void QGstreamerPlayerSession::getStreamsInfo()
{
   QList< QMap<QString, QVariant>> oldProperties = m_streamProperties;
   QList<QMediaStreamsControl::StreamType> oldTypes = m_streamTypes;
   QMap<QMediaStreamsControl::StreamType, int> oldOffset = m_playbin2StreamOffset;

   //check if video is available:
   bool haveAudio = false;
   bool haveVideo = false;
   m_streamProperties.clear();
   m_streamTypes.clear();
   m_playbin2StreamOffset.clear();

   gint audioStreamsCount = 0;
   gint videoStreamsCount = 0;
   gint textStreamsCount = 0;

   g_object_get(G_OBJECT(m_playbin), "n-audio", &audioStreamsCount, nullptr);
   g_object_get(G_OBJECT(m_playbin), "n-video", &videoStreamsCount, nullptr);
   g_object_get(G_OBJECT(m_playbin), "n-text", &textStreamsCount, nullptr);

   haveAudio = audioStreamsCount > 0;
   haveVideo = videoStreamsCount > 0;

   m_playbin2StreamOffset[QMediaStreamsControl::AudioStream] = 0;
   m_playbin2StreamOffset[QMediaStreamsControl::VideoStream] = audioStreamsCount;
   m_playbin2StreamOffset[QMediaStreamsControl::SubPictureStream] = audioStreamsCount + videoStreamsCount;

   for (int i = 0; i < audioStreamsCount; i++) {
      m_streamTypes.append(QMediaStreamsControl::AudioStream);
   }

   for (int i = 0; i < videoStreamsCount; i++) {
      m_streamTypes.append(QMediaStreamsControl::VideoStream);
   }

   for (int i = 0; i < textStreamsCount; i++) {
      m_streamTypes.append(QMediaStreamsControl::SubPictureStream);
   }

   for (int i = 0; i < m_streamTypes.count(); i++) {
      QMediaStreamsControl::StreamType streamType = m_streamTypes[i];
      QMap<QString, QVariant> streamProperties;

      int streamIndex = i - m_playbin2StreamOffset[streamType];

      GstTagList *tags = nullptr;
      switch (streamType) {
         case QMediaStreamsControl::AudioStream:
            g_signal_emit_by_name(G_OBJECT(m_playbin), "get-audio-tags", streamIndex, &tags);
            break;
         case QMediaStreamsControl::VideoStream:
            g_signal_emit_by_name(G_OBJECT(m_playbin), "get-video-tags", streamIndex, &tags);
            break;
         case QMediaStreamsControl::SubPictureStream:
            g_signal_emit_by_name(G_OBJECT(m_playbin), "get-text-tags", streamIndex, &tags);
            break;
         default:
            break;
      }
#if GST_CHECK_VERSION(1,0,0)
      if (tags && GST_IS_TAG_LIST(tags)) {
#else
      if (tags && gst_is_tag_list(tags)) {
#endif
         gchar *languageCode = nullptr;
         if (gst_tag_list_get_string(tags, GST_TAG_LANGUAGE_CODE, &languageCode)) {
            streamProperties[QMediaMetaData::Language] = QString::fromUtf8(languageCode);
         }

         //qDebug() << "language for setream" << i << QString::fromUtf8(languageCode);
         g_free (languageCode);
         gst_tag_list_free(tags);
      }

      m_streamProperties.append(streamProperties);
   }

   bool emitAudioChanged = (haveAudio != m_audioAvailable);
   bool emitVideoChanged = (haveVideo != m_videoAvailable);

   m_audioAvailable = haveAudio;
   m_videoAvailable = haveVideo;

   if (emitAudioChanged) {
      emit audioAvailableChanged(m_audioAvailable);
   }
   if (emitVideoChanged) {
      emit videoAvailableChanged(m_videoAvailable);
   }

   if (oldProperties != m_streamProperties || oldTypes != m_streamTypes || oldOffset != m_playbin2StreamOffset) {
      emit streamsChanged();
   }
}

void QGstreamerPlayerSession::updateVideoResolutionTag()
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif
   QSize size;
   QSize aspectRatio;
   GstPad *pad = gst_element_get_static_pad(m_videoIdentity, "src");
   GstCaps *caps = qt_gst_pad_get_current_caps(pad);

   if (caps) {
      const GstStructure *structure = gst_caps_get_structure(caps, 0);
      gst_structure_get_int(structure, "width", &size.rwidth());
      gst_structure_get_int(structure, "height", &size.rheight());

      gint aspectNum = 0;
      gint aspectDenum = 0;
      if (!size.isEmpty() && gst_structure_get_fraction(
            structure, "pixel-aspect-ratio", &aspectNum, &aspectDenum)) {
         if (aspectDenum > 0) {
            aspectRatio = QSize(aspectNum, aspectDenum);
         }
      }
      gst_caps_unref(caps);
   }

   gst_object_unref(GST_OBJECT(pad));

   QSize currentSize = m_tags.value("resolution").toSize();
   QSize currentAspectRatio = m_tags.value("pixel-aspect-ratio").toSize();

   if (currentSize != size || currentAspectRatio != aspectRatio) {
      if (aspectRatio.isEmpty()) {
         m_tags.remove("pixel-aspect-ratio");
      }

      if (size.isEmpty()) {
         m_tags.remove("resolution");
      } else {
         m_tags.insert("resolution", QVariant(size));
         if (!aspectRatio.isEmpty()) {
            m_tags.insert("pixel-aspect-ratio", QVariant(aspectRatio));
         }
      }

      emit tagsChanged();
   }
}

void QGstreamerPlayerSession::updateDuration()
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

   gboolean seekable = false;
   if (m_duration > 0) {
      m_durationQueries = 0;
      GstQuery *query = gst_query_new_seeking(GST_FORMAT_TIME);
      if (gst_element_query(m_playbin, query)) {
         gst_query_parse_seeking(query, nullptr, &seekable, nullptr, nullptr);
      }
      gst_query_unref(query);
   }
   setSeekable(seekable);

   if (m_durationQueries > 0) {
      //increase delay between duration requests
      int delay = 25 << (5 - m_durationQueries);
      QTimer::singleShot(delay, this, SLOT(updateDuration()));
      m_durationQueries--;
   }
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << m_duration;
#endif
}

void QGstreamerPlayerSession::playbinNotifySource(GObject *o, GParamSpec *p, gpointer d)
{
   (void) p;

   GstElement *source = nullptr;
   g_object_get(o, "source", &source, nullptr);
   if (source == nullptr) {
      return;
   }

#ifdef DEBUG_PLAYBIN
   qDebug() << "Playbin source added:" << G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(source));
#endif

   // Set Headers
   const QByteArray userAgentString("User-Agent");

   QGstreamerPlayerSession *self = reinterpret_cast<QGstreamerPlayerSession *>(d);

   // User-Agent - special case, souphhtpsrc will always set something, even if
   // defined in extra-headers
   if (g_object_class_find_property(G_OBJECT_GET_CLASS(source), "user-agent") != nullptr) {
      g_object_set(G_OBJECT(source), "user-agent",
         self->m_request.rawHeader(userAgentString).constData(), nullptr);
   }

   // The rest
   if (g_object_class_find_property(G_OBJECT_GET_CLASS(source), "extra-headers") != nullptr) {
      GstStructure *extras = qt_gst_structure_new_empty("extras");

      for (const QByteArray &rawHeader : self->m_request.rawHeaderList()) {
         if (rawHeader == userAgentString) { // Filter User-Agent
            continue;
         } else {
            GValue headerValue;

            memset(&headerValue, 0, sizeof(GValue));
            g_value_init(&headerValue, G_TYPE_STRING);

            g_value_set_string(&headerValue,
               self->m_request.rawHeader(rawHeader).constData());

            gst_structure_set_value(extras, rawHeader.constData(), &headerValue);
         }
      }

      if (gst_structure_n_fields(extras) > 0) {
         g_object_set(G_OBJECT(source), "extra-headers", extras, nullptr);
      }

      gst_structure_free(extras);
   }

   //set timeout property to 30 seconds
   const int timeout = 30;
   if (qstrcmp(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(source)), "GstUDPSrc") == 0) {
      //udpsrc timeout unit = microsecond
      //The udpsrc is always a live source.
      g_object_set(G_OBJECT(source), "timeout", G_GUINT64_CONSTANT(timeout * 1000000), nullptr);
      self->m_sourceType = UDPSrc;
      self->m_isLiveSource = true;
   } else if (qstrcmp(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(source)), "GstSoupHTTPSrc") == 0) {
      //souphttpsrc timeout unit = second
      g_object_set(G_OBJECT(source), "timeout", guint(timeout), nullptr);
      self->m_sourceType = SoupHTTPSrc;
      //since gst_base_src_is_live is not reliable, so we check the source property directly
      gboolean isLive = false;
      g_object_get(G_OBJECT(source), "is-live", &isLive, nullptr);
      self->m_isLiveSource = isLive;
   } else if (qstrcmp(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(source)), "GstMMSSrc") == 0) {
      self->m_sourceType = MMSSrc;
      self->m_isLiveSource = gst_base_src_is_live(GST_BASE_SRC(source));
      g_object_set(G_OBJECT(source), "tcp-timeout", G_GUINT64_CONSTANT(timeout * 1000000), nullptr);
   } else if (qstrcmp(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(source)), "GstRTSPSrc") == 0) {
      //rtspsrc acts like a live source and will therefore only generate data in the PLAYING state.
      self->m_sourceType = RTSPSrc;
      self->m_isLiveSource = true;
      g_object_set(G_OBJECT(source), "buffer-mode", 1, nullptr);
   } else {
      self->m_sourceType = UnknownSrc;
      self->m_isLiveSource = gst_base_src_is_live(GST_BASE_SRC(source));
   }

#ifdef DEBUG_PLAYBIN
   if (self->m_isLiveSource) {
      qDebug() << "Current source is a live source";
   } else {
      qDebug() << "Current source is a non-live source";
   }
#endif

   if (self->m_videoSink) {
      g_object_set(G_OBJECT(self->m_videoSink), "sync", !self->m_isLiveSource, nullptr);
   }

   gst_object_unref(source);
}

bool QGstreamerPlayerSession::isLiveSource() const
{
   return m_isLiveSource;
}

void QGstreamerPlayerSession::handleVolumeChange(GObject *o, GParamSpec *p, gpointer d)
{
   (void) o;
   (void) p;

   QGstreamerPlayerSession *session = reinterpret_cast<QGstreamerPlayerSession *>(d);
   QMetaObject::invokeMethod(session, "updateVolume", Qt::QueuedConnection);
}

void QGstreamerPlayerSession::updateVolume()
{
   double volume = 1.0;
   g_object_get(m_playbin, "volume", &volume, nullptr);

   if (m_volume != int(volume * 100 + 0.5)) {
      m_volume = int(volume * 100 + 0.5);
#ifdef DEBUG_PLAYBIN
      qDebug() << Q_FUNC_INFO << m_volume;
#endif
      emit volumeChanged(m_volume);
   }
}

void QGstreamerPlayerSession::handleMutedChange(GObject *o, GParamSpec *p, gpointer d)
{
   (void) o;
   (void) p;

   QGstreamerPlayerSession *session = reinterpret_cast<QGstreamerPlayerSession *>(d);
   QMetaObject::invokeMethod(session, "updateMuted", Qt::QueuedConnection);
}

void QGstreamerPlayerSession::updateMuted()
{
   gboolean muted = FALSE;
   g_object_get(G_OBJECT(m_playbin), "mute", &muted, nullptr);
   if (m_muted != muted) {
      m_muted = muted;
#ifdef DEBUG_PLAYBIN
      qDebug() << Q_FUNC_INFO << m_muted;
#endif
      emit mutedStateChanged(muted);
   }
}

#if !GST_CHECK_VERSION(0, 10, 33)
static gboolean factory_can_src_any_caps (GstElementFactory *factory, const GstCaps *caps)
{
   GList *templates;

   g_return_val_if_fail(factory != nullptr, FALSE);
   g_return_val_if_fail(caps != nullptr, FALSE);

   templates = factory->staticpadtemplates;

   while (templates) {
      GstStaticPadTemplate *templ = (GstStaticPadTemplate *)templates->data;

      if (templ->direction == GST_PAD_SRC) {
         GstCaps *templcaps = gst_static_caps_get(&templ->static_caps);

         if (qt_gst_caps_can_intersect(caps, templcaps)) {
            gst_caps_unref(templcaps);
            return TRUE;
         }
         gst_caps_unref(templcaps);
      }
      templates = g_list_next(templates);
   }

   return FALSE;
}
#endif

GstAutoplugSelectResult QGstreamerPlayerSession::handleAutoplugSelect(GstBin *bin, GstPad *pad, GstCaps *caps,
   GstElementFactory *factory, QGstreamerPlayerSession *session)
{
   (void) bin;
   (void) pad;
   (void) caps;

   GstAutoplugSelectResult res = GST_AUTOPLUG_SELECT_TRY;

   // if VAAPI is available and can be used to decode but the current video sink cannot handle
   // the decoded format, don't use it
   const gchar *factoryName = gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory));
   if (g_str_has_prefix(factoryName, "vaapi")) {
      GstPad *sinkPad = gst_element_get_static_pad(session->m_videoSink, "sink");
#if GST_CHECK_VERSION(1,0,0)
      GstCaps *sinkCaps = gst_pad_query_caps(sinkPad, nullptr);
#else
      GstCaps *sinkCaps = gst_pad_get_caps(sinkPad);
#endif

#if !GST_CHECK_VERSION(0, 10, 33)
      if (!factory_can_src_any_caps(factory, sinkCaps))
#else
      if (!gst_element_factory_can_src_any_caps(factory, sinkCaps))
#endif
         res = GST_AUTOPLUG_SELECT_SKIP;

      gst_object_unref(sinkPad);
      gst_caps_unref(sinkCaps);
   }

   return res;
}

void QGstreamerPlayerSession::handleElementAdded(GstBin *bin, GstElement *element, QGstreamerPlayerSession *session)
{
   (void) bin;

   //we have to configure queue2 element to enable media downloading
   //and reporting available ranges,
   //but it's added dynamically to playbin2

   gchar *elementName = gst_element_get_name(element);

   if (g_str_has_prefix(elementName, "queue2")) {
      // Disable on-disk buffering.
      g_object_set(G_OBJECT(element), "temp-template", nullptr, nullptr);
   } else if (g_str_has_prefix(elementName, "uridecodebin") ||
#if GST_CHECK_VERSION(1,0,0)
      g_str_has_prefix(elementName, "decodebin")) {
#else
      g_str_has_prefix(elementName, "decodebin2")) {
      if (g_str_has_prefix(elementName, "uridecodebin")) {
         // Add video/x-surface (VAAPI) to default raw formats
         g_object_set(G_OBJECT(element), "caps", gst_static_caps_get(&static_RawCaps), nullptr);
         // listen for uridecodebin autoplug-select to skip VAAPI usage when the current
         // video sink doesn't support it
         g_signal_connect(element, "autoplug-select", G_CALLBACK(handleAutoplugSelect), session);
      }
#endif
      //listen for queue2 element added to uridecodebin/decodebin2 as well.
      //Don't touch other bins since they may have unrelated queues
      g_signal_connect(element, "element-added",
         G_CALLBACK(handleElementAdded), session);
   }

   g_free(elementName);
}

void QGstreamerPlayerSession::handleStreamsChange(GstBin *bin, gpointer user_data)
{
   (void) bin;

   QGstreamerPlayerSession *session = reinterpret_cast<QGstreamerPlayerSession *>(user_data);
   QMetaObject::invokeMethod(session, "getStreamsInfo", Qt::QueuedConnection);
}

//doing proper operations when detecting an invalidMedia: change media status before signal the erorr
void QGstreamerPlayerSession::processInvalidMedia(QMediaPlayer::Error errorCode, const QString &errorString)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif
   if (m_isPlaylist) {
      stop();
      emit error(int(QMediaPlayer::MediaIsPlaylist), tr("Media is loaded as a playlist"));
   } else {
      emit invalidMedia();
      stop();
      emit error(int(errorCode), errorString);
   }
}

void QGstreamerPlayerSession::showPrerollFrames(bool enabled)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << enabled;
#endif
   if (enabled != m_displayPrerolledFrame && m_videoSink &&
      g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "show-preroll-frame") != nullptr) {

      gboolean value = enabled;
      g_object_set(G_OBJECT(m_videoSink), "show-preroll-frame", value, nullptr);
      m_displayPrerolledFrame = enabled;
   }
}

void QGstreamerPlayerSession::addProbe(QGstreamerVideoProbeControl *probe)
{
   Q_ASSERT(!m_videoProbe);
   m_videoProbe = probe;
   addVideoBufferProbe();
}

void QGstreamerPlayerSession::removeProbe(QGstreamerVideoProbeControl *probe)
{
   Q_ASSERT(m_videoProbe == probe);
   removeVideoBufferProbe();
   m_videoProbe = nullptr;
}

void QGstreamerPlayerSession::addProbe(QGstreamerAudioProbeControl *probe)
{
   Q_ASSERT(!m_audioProbe);
   m_audioProbe = probe;
   addAudioBufferProbe();
}

void QGstreamerPlayerSession::removeProbe(QGstreamerAudioProbeControl *probe)
{
   Q_ASSERT(m_audioProbe == probe);
   removeAudioBufferProbe();
   m_audioProbe = nullptr;
}

// This function is similar to stop(),
// but does not set m_everPlayed, m_lastPosition,
// and setSeekable() values.
void QGstreamerPlayerSession::endOfMediaReset()
{
   if (m_renderer) {
      m_renderer->stopRenderer();
   }

   flushVideoProbes();
   gst_element_set_state(m_playbin, GST_STATE_NULL);

   QMediaPlayer::State oldState = m_state;
   m_pendingState = m_state = QMediaPlayer::StoppedState;

   finishVideoOutputChange();

   if (oldState != m_state) {
      emit stateChanged(m_state);
   }
}

void QGstreamerPlayerSession::removeVideoBufferProbe()
{
   if (!m_videoProbe) {
      return;
   }

   GstPad *pad = gst_element_get_static_pad(m_videoSink, "sink");
   if (pad) {
      m_videoProbe->removeProbeFromPad(pad);
      gst_object_unref(GST_OBJECT(pad));
   }
}

void QGstreamerPlayerSession::addVideoBufferProbe()
{
   if (!m_videoProbe) {
      return;
   }

   GstPad *pad = gst_element_get_static_pad(m_videoSink, "sink");
   if (pad) {
      m_videoProbe->addProbeToPad(pad);
      gst_object_unref(GST_OBJECT(pad));
   }
}

void QGstreamerPlayerSession::removeAudioBufferProbe()
{
   if (!m_audioProbe) {
      return;
   }

   GstPad *pad = gst_element_get_static_pad(m_audioSink, "sink");
   if (pad) {
      m_audioProbe->removeProbeFromPad(pad);
      gst_object_unref(GST_OBJECT(pad));
   }
}

void QGstreamerPlayerSession::addAudioBufferProbe()
{
   if (!m_audioProbe) {
      return;
   }

   GstPad *pad = gst_element_get_static_pad(m_audioSink, "sink");
   if (pad) {
      m_audioProbe->addProbeToPad(pad);
      gst_object_unref(GST_OBJECT(pad));
   }
}

void QGstreamerPlayerSession::flushVideoProbes()
{
   if (m_videoProbe) {
      m_videoProbe->startFlushing();
   }
}

void QGstreamerPlayerSession::resumeVideoProbes()
{
   if (m_videoProbe) {
      m_videoProbe->stopFlushing();
   }
}

void QGstreamerPlayerSession::playlistTypeFindFunction(GstTypeFind *find, gpointer userData)
{
   QGstreamerPlayerSession *session = (QGstreamerPlayerSession *)userData;

   const gchar *uri = nullptr;

#if GST_CHECK_VERSION(1,0,0)
   g_object_get(G_OBJECT(session->m_playbin), "current-uri", &uri, nullptr);
#else
   g_object_get(G_OBJECT(session->m_playbin), "uri", &uri, nullptr);
#endif

   guint64 length = gst_type_find_get_length(find);
   if (! length) {
      length = 1024;
   } else {
      length = qMin(length, guint64(1024));
   }

   while (length > 0) {
      const guint8 *data = gst_type_find_peek(find, 0, length);

      if (data) {
         session->m_isPlaylist = (QPlaylistFileParser::findPlaylistType(QString::fromUtf8(uri), QString(),
                  QByteArray((const char *)data, length)) != QPlaylistFileParser::UNKNOWN);
         return;
      }

      // for HTTP files length is not available,
      // so we have to try different buffer sizes
      length >>= 1;
   }
}


