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

#ifndef QAUDIOPROBE_H
#define QAUDIOPROBE_H

#include <qobject.h>
#include <qaudiobuffer.h>

class QMediaObject;
class QMediaRecorder;
class QAudioProbePrivate;

class Q_MULTIMEDIA_EXPORT QAudioProbe : public QObject
{
    MULTI_CS_OBJECT(QAudioProbe)

 public:
    explicit QAudioProbe(QObject *parent = nullptr);
    ~QAudioProbe();

    bool setSource(QMediaObject *source);
    bool setSource(QMediaRecorder *source);

    bool isActive() const;

    MULTI_CS_SIGNAL_1(Public, void audioBufferProbed(const QAudioBuffer & buffer))
    MULTI_CS_SIGNAL_2(audioBufferProbed, buffer)

    MULTI_CS_SIGNAL_1(Public, void flush())
    MULTI_CS_SIGNAL_2(flush)

 private:
    QAudioProbePrivate *d;
};

#endif
