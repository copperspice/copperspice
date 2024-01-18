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

#include <qgstreameraudiodecodercontrol.h>

#include <qdir.h>
#include <qsocketnotifier.h>
#include <qurl.h>
#include <qdebug.h>

#include <qgstreameraudiodecodersession.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

QGstreamerAudioDecoderControl::QGstreamerAudioDecoderControl(QGstreamerAudioDecoderSession *session, QObject *parent)
   : QAudioDecoderControl(parent), m_session(session)
{
   connect(m_session, &QGstreamerAudioDecoderSession::bufferAvailableChanged, this, &QGstreamerAudioDecoderControl::bufferAvailableChanged);
   connect(m_session, &QGstreamerAudioDecoderSession::bufferReady,            this, &QGstreamerAudioDecoderControl::bufferReady);
   connect(m_session, &QGstreamerAudioDecoderSession::error,                  this, &QGstreamerAudioDecoderControl::error);
   connect(m_session, &QGstreamerAudioDecoderSession::formatChanged,          this, &QGstreamerAudioDecoderControl::formatChanged);
   connect(m_session, &QGstreamerAudioDecoderSession::sourceChanged,          this, &QGstreamerAudioDecoderControl::sourceChanged);
   connect(m_session, &QGstreamerAudioDecoderSession::stateChanged,           this, &QGstreamerAudioDecoderControl::stateChanged);
   connect(m_session, &QGstreamerAudioDecoderSession::finished,               this, &QGstreamerAudioDecoderControl::finished);
   connect(m_session, &QGstreamerAudioDecoderSession::positionChanged,        this, &QGstreamerAudioDecoderControl::positionChanged);
   connect(m_session, &QGstreamerAudioDecoderSession::durationChanged,        this, &QGstreamerAudioDecoderControl::durationChanged);
}

QGstreamerAudioDecoderControl::~QGstreamerAudioDecoderControl()
{
}

QAudioDecoder::State QGstreamerAudioDecoderControl::state() const
{
   return m_session->pendingState();
}

QString QGstreamerAudioDecoderControl::sourceFilename() const
{
   return m_session->sourceFilename();
}

void QGstreamerAudioDecoderControl::setSourceFilename(const QString &fileName)
{
   m_session->setSourceFilename(fileName);
}

QIODevice *QGstreamerAudioDecoderControl::sourceDevice() const
{
   return m_session->sourceDevice();
}

void QGstreamerAudioDecoderControl::setSourceDevice(QIODevice *device)
{
   m_session->setSourceDevice(device);
}

void QGstreamerAudioDecoderControl::start()
{
   m_session->start();
}

void QGstreamerAudioDecoderControl::stop()
{
   m_session->stop();
}

QAudioFormat QGstreamerAudioDecoderControl::audioFormat() const
{
   return m_session->audioFormat();
}

void QGstreamerAudioDecoderControl::setAudioFormat(const QAudioFormat &format)
{
   m_session->setAudioFormat(format);
}

QAudioBuffer QGstreamerAudioDecoderControl::read()
{
   return m_session->read();
}

bool QGstreamerAudioDecoderControl::bufferAvailable() const
{
   return m_session->bufferAvailable();
}

qint64 QGstreamerAudioDecoderControl::position() const
{
   return m_session->position();
}

qint64 QGstreamerAudioDecoderControl::duration() const
{
   return m_session->duration();
}


