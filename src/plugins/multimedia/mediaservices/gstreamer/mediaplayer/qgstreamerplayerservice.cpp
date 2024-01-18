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

#include <qdebug.h>
#include <qvariant.h>
#include <qwidget.h>

#include <qgstreamerplayerservice.h>
#include <qgstreamerplayercontrol.h>
#include <qgstreamerplayersession.h>
#include <qgstreamermetadataprovider.h>
#include <qgstreameravailabilitycontrol.h>
#include <qgstreamerstreamscontrol.h>
#include <qmediaplaylist.h>

#include <qgstreamervideowidget_p.h>
#include <qgstreamervideowindow_p.h>
#include <qgstreamervideorenderer_p.h>

#if defined(HAVE_MIR) && defined (__arm__)
#include <qgstreamermirtexturerenderer_p.h>
#endif

#include <qgstreameraudioprobecontrol_p.h>
#include <qgstreamervideoprobecontrol_p.h>
#include <qmediaplaylistnavigator_p.h>
#include <qmediaresourceset_p.h>

QGstreamerPlayerService::QGstreamerPlayerService(QObject *parent)
   : QMediaService(parent), m_audioProbeControl(nullptr), m_videoProbeControl(nullptr), m_videoOutput(nullptr),
     m_videoRenderer(nullptr), m_videoWindow(nullptr), m_videoWidget(nullptr), m_videoReferenceCount(0)
{
   m_session  = new QGstreamerPlayerSession(this);
   m_control  = new QGstreamerPlayerControl(m_session, this);
   m_metaData = new QGstreamerMetaDataProvider(m_session, this);
   m_streamsControl      = new QGstreamerStreamsControl(m_session, this);
   m_availabilityControl = new QGStreamerAvailabilityControl(m_control->resources(), this);

#if defined(HAVE_MIR) && defined (__arm__)
   m_videoRenderer = new QGstreamerMirTextureRenderer(this, m_session);
#else
   m_videoRenderer = new QGstreamerVideoRenderer(this);
#endif

   m_videoWindow = new QGstreamerVideoWindow(this);

   // If the GStreamer video sink is not available, don't provide the video window control since
   // it won't work anyway.
   if (!m_videoWindow->videoSink()) {
      delete m_videoWindow;
      m_videoWindow = nullptr;
   }


   m_videoWidget = new QGstreamerVideoWidgetControl(this);

   // If the GStreamer video sink is not available, don't provide the video widget control since
   // it won't work anyway.
   // QVideoWidget will fall back to QVideoRendererControl in that case.
   if (!m_videoWidget->videoSink()) {
      delete m_videoWidget;
      m_videoWidget = nullptr;
   }
}

QGstreamerPlayerService::~QGstreamerPlayerService()
{
}

QMediaControl *QGstreamerPlayerService::requestControl(const QString &name)
{
   if (name == QMediaPlayerControl_Key) {
      return m_control;
   }

   if (name == QMetaDataReaderControl_iid) {
      return m_metaData;
   }

   if (name == QMediaStreamsControl_iid) {
      return m_streamsControl;
   }

   if (name == QMediaAvailabilityControl_iid) {
      return m_availabilityControl;
   }

   if (name == QMediaVideoProbeControl_iid) {
      if (! m_videoProbeControl) {
         increaseVideoRef();
         m_videoProbeControl = new QGstreamerVideoProbeControl(this);
         m_session->addProbe(m_videoProbeControl);
      }

      m_videoProbeControl->ref.ref();
      return m_videoProbeControl;
   }

   if (name == QMediaAudioProbeControl_iid) {
      if (! m_audioProbeControl) {
         m_audioProbeControl = new QGstreamerAudioProbeControl(this);
         m_session->addProbe(m_audioProbeControl);
      }

      m_audioProbeControl->ref.ref();
      return m_audioProbeControl;
   }

   if (! m_videoOutput) {
      if (name == QVideoRendererControl_iid) {
         m_videoOutput = m_videoRenderer;
      }

      else if (name == QVideoWindowControl_iid) {
         m_videoOutput = m_videoWindow;
      }

      else if (name == QVideoWidgetControl_iid) {
         m_videoOutput = m_videoWidget;
      }

      if (m_videoOutput) {
         increaseVideoRef();
         m_control->setVideoOutput(m_videoOutput);

         return m_videoOutput;
      }
   }

   return nullptr;
}

void QGstreamerPlayerService::releaseControl(QMediaControl *control)
{
   if (! control) {
      return;

   } else if (control == m_videoOutput) {
      m_videoOutput = nullptr;
      m_control->setVideoOutput(nullptr);
      decreaseVideoRef();

   } else if (control == m_videoProbeControl && !m_videoProbeControl->ref.deref()) {
      m_session->removeProbe(m_videoProbeControl);
      delete m_videoProbeControl;
      m_videoProbeControl = nullptr;
      decreaseVideoRef();

   } else if (control == m_audioProbeControl && !m_audioProbeControl->ref.deref()) {
      m_session->removeProbe(m_audioProbeControl);
      delete m_audioProbeControl;
      m_audioProbeControl = nullptr;
   }
}

void QGstreamerPlayerService::increaseVideoRef()
{
   m_videoReferenceCount++;

   if (m_videoReferenceCount == 1) {
      m_control->resources()->setVideoEnabled(true);
   }
}

void QGstreamerPlayerService::decreaseVideoRef()
{
   m_videoReferenceCount--;
   if (m_videoReferenceCount == 0) {
      m_control->resources()->setVideoEnabled(false);
   }
}

