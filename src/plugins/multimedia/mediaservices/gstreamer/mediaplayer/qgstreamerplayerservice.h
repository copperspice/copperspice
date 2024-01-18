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

#ifndef QGSTREAMERPLAYERSERVICE_H
#define QGSTREAMERPLAYERSERVICE_H

#include <qobject.h>
#include <qiodevice.h>

#include <qmediaservice.h>

class QMediaPlayerControl;
class QMediaPlaylist;
class QMediaPlaylistNavigator;

class QGstreamerMetaData;
class QGstreamerPlayerControl;
class QGstreamerPlayerSession;
class QGstreamerMetaDataProvider;
class QGstreamerStreamsControl;
class QGstreamerVideoRenderer;
class QGstreamerVideoWindow;
class QGstreamerVideoWidgetControl;
class QGStreamerAvailabilityControl;
class QGstreamerAudioProbeControl;
class QGstreamerVideoProbeControl;

class QGstreamerPlayerService : public QMediaService
{
   CS_OBJECT(QGstreamerPlayerService)

 public:
   QGstreamerPlayerService(QObject *parent = nullptr);
   ~QGstreamerPlayerService();

   QMediaControl *requestControl(const QString &name) override;
   void releaseControl(QMediaControl *control) override;

 private:
   QGstreamerPlayerControl *m_control;
   QGstreamerPlayerSession *m_session;
   QGstreamerMetaDataProvider *m_metaData;
   QGstreamerStreamsControl *m_streamsControl;
   QGStreamerAvailabilityControl *m_availabilityControl;

   QGstreamerAudioProbeControl *m_audioProbeControl;
   QGstreamerVideoProbeControl *m_videoProbeControl;

   QMediaControl *m_videoOutput;
   QMediaControl *m_videoRenderer;
   QGstreamerVideoWindow *m_videoWindow;
   QGstreamerVideoWidgetControl *m_videoWidget;

   void increaseVideoRef();
   void decreaseVideoRef();
   int m_videoReferenceCount;
};

#endif
