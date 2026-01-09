/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qdatetime.h>
#include <qdebug.h>
#include <qdir.h>
#include <qmediametadata.h>
#include <qsize.h>
#include <qstandardpaths.h>
#include <qtimer.h>

#include <playlistfileparser_p.h>
#include <qgstreameraudioprobecontrol_p.h>
#include <qgstreamerbushelper_p.h>
#include <qgstreamervideoprobecontrol_p.h>
#include <qgstreamervideorendererinterface_p.h>
#include <qgstutils_p.h>

#include <gst/gstvalue.h>
#include <gst/base/gstbasesrc.h>

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

QGstreamerPlayerSession::QGstreamerPlayerSession(QObject *parent)
   : QObject(parent),
     m_state(QMediaPlayer::StoppedState),
     m_pendingState(QMediaPlayer::StoppedState),
     m_busHelper(nullptr),
     m_playbin(nullptr),
     m_videoSink(nullptr),
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

   m_videoIdentity = gst_element_factory_make("identity", nullptr); // floating ref

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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

   if (ranges.isEmpty() && !isLiveSource() && isSeekable()) {
      ranges.addInterval(0, duration());
   }

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

static GstPadProbeReturn block_pad_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
   (void) pad;
   (void) info;
   (void) user_data;

   return GST_PAD_PROBE_OK;
}

void QGstreamerPlayerSession::updateVideoRenderer()
{
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
   qDebug() << "Video sink has changed, reload video output";
#endif

   if (m_videoOutput) {
      setVideoRenderer(m_videoOutput);
   }
}

void QGstreamerPlayerSession::setVideoRenderer(QObject *videoOutput)
{
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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
   gst_debug_bin_to_dot_file_with_ts(GST_BIN(m_playbin), GstDebugGraphDetails(GST_DEBUG_GRAPH_SHOW_ALL), "playbin_set");
#endif

   GstElement *videoSink = nullptr;
   if (m_renderer && m_renderer->isReady()) {
      videoSink = m_renderer->videoSink();
   }

   if (!videoSink) {
      videoSink = m_nullVideoSink;
   }

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
   qDebug() << "Set video output:" << videoOutput;
   qDebug() << "Current sink:" << (m_videoSink ? GST_ELEMENT_NAME(m_videoSink) : "") <<  m_videoSink
      << "pending:" << (m_pendingVideoSink ? GST_ELEMENT_NAME(m_pendingVideoSink) : "") << m_pendingVideoSink
      << "new sink:" << (videoSink ? GST_ELEMENT_NAME(videoSink) : "") << videoSink;
#endif

   if (m_pendingVideoSink == videoSink || (m_pendingVideoSink == nullptr && m_videoSink == videoSink)) {

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
      qDebug() << "Video sink has not changed, skip video output reconfiguration";
#endif

      return;
   }

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
   qDebug() << "Reconfigure video output";
#endif

   if (m_state == QMediaPlayer::StoppedState) {

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
      qDebug() << "The pipeline has not started yet, pending state:" << m_pendingState;
#endif

      //the pipeline has not started yet
      flushVideoProbes();
      m_pendingVideoSink = nullptr;
      gst_element_set_state(m_videoSink, GST_STATE_NULL);
      gst_element_set_state(m_playbin, GST_STATE_NULL);

      removeVideoBufferProbe();

      gst_bin_remove(GST_BIN(m_videoOutputBin), m_videoSink);

      m_videoSink = videoSink;

      gst_bin_add(GST_BIN(m_videoOutputBin), m_videoSink);

      bool linked = gst_element_link(m_videoIdentity, m_videoSink);

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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
         qDebug() << "already waiting for pad to be blocked, just change the pending sink";
#endif
         m_pendingVideoSink = videoSink;
         return;
      }

      m_pendingVideoSink = videoSink;

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
      qDebug() << "Blocking the video output pad";
#endif

      //block pads, async to avoid locking in paused state
      GstPad *srcPad = gst_element_get_static_pad(m_videoIdentity, "src");

      this->pad_probe_id = gst_pad_add_probe(srcPad, (GstPadProbeType)(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BLOCKING),
            block_pad_cb, this, nullptr);

      gst_object_unref(GST_OBJECT(srcPad));

      //Unpause the sink to avoid waiting until the buffer is processed
      //while the sink is paused. The pad will be blocked as soon as the current
      //buffer is processed.
      if (m_state == QMediaPlayer::PausedState) {
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
      qDebug() << "Abort, no change";
#endif

      //video output was change back to the current one,
      //no need to torment the pipeline, just unblock the pad

      if (gst_pad_is_blocked(srcPad)) {
         gst_pad_remove_probe(srcPad, this->pad_probe_id);
      }

      m_pendingVideoSink = nullptr;
      gst_object_unref(GST_OBJECT(srcPad));

      return;
   }

   {
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

   if (!linked) {
      qWarning() << "Linking video output element failed";
   }

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
   qDebug() << "notify the video connector it has to emit a new segment message";
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
   if (gst_pad_is_blocked(srcPad)) {
      gst_pad_remove_probe(srcPad, this->pad_probe_id);
   }

   gst_object_unref(GST_OBJECT(srcPad));

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
   gst_debug_bin_to_dot_file_with_ts(GST_BIN(m_playbin),
      GstDebugGraphDetails(GST_DEBUG_GRAPH_SHOW_ALL), "playbin_finish");
#endif
}

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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
               QStringList states;
               states << "GST_STATE_VOID_PENDING" <<  "GST_STATE_NULL" << "GST_STATE_READY" << "GST_STATE_PAUSED" << "GST_STATE_PLAYING";

               qDebug() << QString("state changed: old: %1  new: %2  pending: %3")
                  .formatArg(states[oldState]).formatArg(states[newState]).formatArg(states[pending]);
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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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
            case GST_MESSAGE_ASYNC_START:
               break;

            case GST_MESSAGE_ASYNC_DONE: {
               gint64 position = 0;

               if (qt_gst_element_query_position(m_playbin, GST_FORMAT_TIME, &position)) {
                  position /= 1000000;
                  m_lastPosition = position;
                  emit positionChanged(position);
               }
               break;
            }

            case GST_MESSAGE_REQUEST_STATE:
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

      if (tags && GST_IS_TAG_LIST(tags)) {
         gchar *languageCode = nullptr;

         if (gst_tag_list_get_string(tags, GST_TAG_LANGUAGE_CODE, &languageCode)) {
            streamProperties[QMediaMetaData::Language] = QString::fromUtf8(languageCode);
         }

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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
      qDebug() << Q_FUNC_INFO << m_muted;
#endif

      emit mutedStateChanged(muted);
   }
}

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
      GstPad *sinkPad   = gst_element_get_static_pad(session->m_videoSink, "sink");
      GstCaps *sinkCaps = gst_pad_query_caps(sinkPad, nullptr);

      if (! gst_element_factory_can_src_any_caps(factory, sinkCaps)) {
         res = GST_AUTOPLUG_SELECT_SKIP;
      }

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
      g_str_has_prefix(elementName, "decodebin")) {

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
#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
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

   g_object_get(G_OBJECT(session->m_playbin), "current-uri", &uri, nullptr);

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
