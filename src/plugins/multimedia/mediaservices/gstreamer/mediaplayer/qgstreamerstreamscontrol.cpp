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

#include <qgstreamerstreamscontrol.h>
#include <qgstreamerplayersession.h>

QGstreamerStreamsControl::QGstreamerStreamsControl(QGstreamerPlayerSession *session, QObject *parent)
   : QMediaStreamsControl(parent), m_session(session)
{
   connect(m_session, &QGstreamerPlayerSession::streamsChanged, this, &QGstreamerStreamsControl::streamsChanged);
}

QGstreamerStreamsControl::~QGstreamerStreamsControl()
{
}

int QGstreamerStreamsControl::streamCount()
{
   return m_session->streamCount();
}

QMediaStreamsControl::StreamType QGstreamerStreamsControl::streamType(int streamNumber)
{
   return m_session->streamType(streamNumber);
}

QVariant QGstreamerStreamsControl::metaData(int streamNumber, const QString &key)
{
   return m_session->streamProperties(streamNumber).value(key);
}

bool QGstreamerStreamsControl::isActive(int streamNumber)
{
   return streamNumber != -1 && streamNumber == m_session->activeStream(streamType(streamNumber));
}

void QGstreamerStreamsControl::setActive(int streamNumber, bool state)
{
   QMediaStreamsControl::StreamType type = m_session->streamType(streamNumber);
   if (type == QMediaStreamsControl::UnknownStream) {
      return;
   }

   if (state) {
      m_session->setActiveStream(type, streamNumber);
   } else {
      //only one active stream of certain type is supported
      if (m_session->activeStream(type) == streamNumber) {
         m_session->setActiveStream(type, -1);
      }
   }
}

