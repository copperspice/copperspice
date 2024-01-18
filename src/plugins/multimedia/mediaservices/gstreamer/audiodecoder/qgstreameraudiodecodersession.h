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

#ifndef QGSTREAMERPLAYERSESSION_H
#define QGSTREAMERPLAYERSESSION_H

#include <qobject.h>
#include <qmutex.h>
#include <qaudiodecoder.h>

#include <qgstreameraudiodecodercontrol.h>
#include <qgstreamerbushelper_p.h>

#if defined(HAVE_GST_APPSRC)
#include <qgstappsrc_p.h>
#endif

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class QGstreamerBusHelper;
class QGstreamerMessage;

class QGstreamerAudioDecoderSession : public QObject, public QGstreamerBusMessageFilter
{
   CS_OBJECT_MULTIPLE(QGstreamerAudioDecoderSession, QObject)

   CS_INTERFACES(QGstreamerBusMessageFilter)

 public:
   QGstreamerAudioDecoderSession(QObject *parent);
   virtual ~QGstreamerAudioDecoderSession();

   QGstreamerBusHelper *bus() const {
      return m_busHelper;
   }

   QAudioDecoder::State state() const {
      return m_state;
   }
   QAudioDecoder::State pendingState() const {
      return m_pendingState;
   }

   bool processBusMessage(const QGstreamerMessage &message) override;

#if defined(HAVE_GST_APPSRC)
   QGstAppSrc *appsrc() const {
      return m_appSrc;
   }
   static void configureAppSrcElement(GObject *, GObject *, GParamSpec *, QGstreamerAudioDecoderSession *_this);
#endif

   QString sourceFilename() const;
   void setSourceFilename(const QString &fileName);

   QIODevice *sourceDevice() const;
   void setSourceDevice(QIODevice *device);

   void start();
   void stop();

   QAudioFormat audioFormat() const;
   void setAudioFormat(const QAudioFormat &format);

   QAudioBuffer read();
   bool bufferAvailable() const;

   qint64 position() const;
   qint64 duration() const;

   static GstFlowReturn new_sample(GstAppSink *sink, gpointer user_data);

   CS_SIGNAL_1(Public, void stateChanged(QAudioDecoder::State newState))
   CS_SIGNAL_2(stateChanged, newState)
   CS_SIGNAL_1(Public, void formatChanged(const QAudioFormat &format))
   CS_SIGNAL_2(formatChanged, format)
   CS_SIGNAL_1(Public, void sourceChanged())
   CS_SIGNAL_2(sourceChanged)

   CS_SIGNAL_1(Public, void error(int error, const QString &errorString))
   CS_SIGNAL_2(error, error, errorString)

   CS_SIGNAL_1(Public, void bufferReady())
   CS_SIGNAL_2(bufferReady)
   CS_SIGNAL_1(Public, void bufferAvailableChanged(bool available))
   CS_SIGNAL_2(bufferAvailableChanged, available)
   CS_SIGNAL_1(Public, void finished())
   CS_SIGNAL_2(finished)

   CS_SIGNAL_1(Public, void positionChanged(qint64 position))
   CS_SIGNAL_2(positionChanged, position)
   CS_SIGNAL_1(Public, void durationChanged(qint64 duration))
   CS_SIGNAL_2(durationChanged, duration)

 private:
   void setAudioFlags(bool wantNativeAudio);
   void addAppSink();
   void removeAppSink();

   void processInvalidMedia(QAudioDecoder::Error errorCode, const QString &errorString);
   static qint64 getPositionFromBuffer(GstBuffer *buffer);

   QAudioDecoder::State m_state;
   QAudioDecoder::State m_pendingState;
   QGstreamerBusHelper *m_busHelper;
   GstBus *m_bus;
   GstElement *m_playbin;
   GstElement *m_outputBin;
   GstElement *m_audioConvert;
   GstAppSink *m_appSink;

#if defined(HAVE_GST_APPSRC)
   QGstAppSrc *m_appSrc;
#endif

   QString mSource;
   QIODevice *mDevice; // QWeakPointer perhaps
   QAudioFormat mFormat;

   mutable QMutex m_buffersMutex;
   int m_buffersAvailable;

   qint64 m_position;
   qint64 m_duration;

   int m_durationQueries;

   CS_SLOT_1(Private, void updateDuration())
   CS_SLOT_2(updateDuration)
};

#endif
