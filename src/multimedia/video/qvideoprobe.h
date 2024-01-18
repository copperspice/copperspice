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

#ifndef QVIDEOPROBE_H
#define QVIDEOPROBE_H

#include <qobject.h>
#include <qvideoframe.h>

class QMediaObject;
class QMediaRecorder;
class QVideoProbePrivate;

class Q_MULTIMEDIA_EXPORT QVideoProbe : public QObject
{
    MULTI_CS_OBJECT(QVideoProbe)

public:
    explicit QVideoProbe(QObject *parent = nullptr);
    ~QVideoProbe();

    bool setSource(QMediaObject *source);
    bool setSource(QMediaRecorder *source);

    bool isActive() const;

    MULTI_CS_SIGNAL_1(Public, void videoFrameProbed(const QVideoFrame & videoFrame))
    MULTI_CS_SIGNAL_2(videoFrameProbed, videoFrame)

    MULTI_CS_SIGNAL_1(Public, void flush())
    MULTI_CS_SIGNAL_2(flush)

private:
    QVideoProbePrivate *d;
};

#endif
