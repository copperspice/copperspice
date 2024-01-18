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

#include "avfmediaplayerservice.h"

#include "avfmediaplayersession.h"
#include "avfmediaplayercontrol.h"
#include "avfmediaplayermetadatacontrol.h"
#include "avfvideooutput.h"
#include "avfvideorenderercontrol.h"
#include "avfvideowidgetcontrol.h"
#include "avfvideowindowcontrol.h"

#import <AVFoundation/AVFoundation.h>

AVFMediaPlayerService::AVFMediaPlayerService(QObject *parent)
   : QMediaService(parent), m_videoOutput(nullptr), m_enableRenderControl(true)
{
   m_session = new AVFMediaPlayerSession(this);
   m_control = new AVFMediaPlayerControl(this);
   m_control->setSession(m_session);
   m_playerMetaDataControl = new AVFMediaPlayerMetaDataControl(m_session, this);

   // AVPlayerItemVideoOutput is available in SDK

   // might not be available at runtime
#if defined(Q_OS_IOS)
   m_enableRenderControl = [AVPlayerItemVideoOutput class] != 0;
#endif

   connect(m_control, &AVFMediaPlayerControl::mediaChanged, m_playerMetaDataControl, &AVFMediaPlayerMetaDataControl::updateTags);
}

AVFMediaPlayerService::~AVFMediaPlayerService()
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO;
#endif
   delete m_session;
}

QMediaControl *AVFMediaPlayerService::requestControl(const QString &name)
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO << name;
#endif

   if (name == QMediaPlayerControl_Key) {
      return m_control;
   }

   if (name == QMetaDataReaderControl_iid) {
      return m_playerMetaDataControl;
   }

   if (m_enableRenderControl && (name == QVideoRendererControl_iid)) {
      if (!m_videoOutput) {
         m_videoOutput = new AVFVideoRendererControl(this);
      }

      m_session->setVideoOutput(qobject_cast<AVFVideoOutput *>(m_videoOutput));
      return m_videoOutput;
   }

   if (name == QVideoWidgetControl_iid) {
      if (!m_videoOutput) {
         m_videoOutput = new AVFVideoWidgetControl(this);
      }

      m_session->setVideoOutput(qobject_cast<AVFVideoOutput *>(m_videoOutput));
      return m_videoOutput;
   }

   if (name == QVideoWindowControl_iid) {
      if (! m_videoOutput) {
         m_videoOutput = new AVFVideoWindowControl(this);
      }

      m_session->setVideoOutput(qobject_cast<AVFVideoOutput *>(m_videoOutput));
      return m_videoOutput;
   }

   return nullptr;
}

void AVFMediaPlayerService::releaseControl(QMediaControl *control)
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO << control;
#endif

   if (m_videoOutput == control) {
      AVFVideoRendererControl *renderControl = qobject_cast<AVFVideoRendererControl *>(m_videoOutput);

      if (renderControl) {
         renderControl->setSurface(nullptr);
      }

      m_videoOutput = nullptr;
      m_session->setVideoOutput(nullptr);

      delete control;
   }
}
