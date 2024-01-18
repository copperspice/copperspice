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

#ifndef QGSTREAMERPLAYERCONTROL_H
#define QGSTREAMERPLAYERCONTROL_H

#include <qobject.h>
#include <qstack.h>
#include <qaudioformat.h>
#include <qaudiobuffer.h>
#include <qaudiodecoder.h>
#include <qaudiodecodercontrol.h>

#include <limits.h>

class QGstreamerAudioDecoderSession;
class QGstreamerAudioDecoderService;

class QGstreamerAudioDecoderControl : public QAudioDecoderControl
{
   CS_OBJECT(QGstreamerAudioDecoderControl)

 public:
   QGstreamerAudioDecoderControl(QGstreamerAudioDecoderSession *session, QObject *parent = nullptr);
   ~QGstreamerAudioDecoderControl();

   QAudioDecoder::State state() const override;

   QString sourceFilename() const override;
   void setSourceFilename(const QString &fileName) override;

   QIODevice *sourceDevice() const override;
   void setSourceDevice(QIODevice *device) override;

   void start() override;
   void stop() override;

   QAudioFormat audioFormat() const override;
   void setAudioFormat(const QAudioFormat &format) override;

   QAudioBuffer read() override;
   bool bufferAvailable() const override;

   qint64 position() const override;
   qint64 duration() const override;

 private:
   QGstreamerAudioDecoderSession *m_session;
};

#endif
