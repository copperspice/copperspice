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

#ifndef QMEDIAGAPLESSPLAYBACKCONTROL_H
#define QMEDIAGAPLESSPLAYBACKCONTROL_H

#include <qstring.h>
#include <qmediacontrol.h>
#include <qmediacontent.h>

class Q_MULTIMEDIA_EXPORT QMediaGaplessPlaybackControl : public QMediaControl
{
    MULTI_CS_OBJECT(QMediaGaplessPlaybackControl)

public:
    virtual ~QMediaGaplessPlaybackControl();

    virtual QMediaContent nextMedia() const = 0;
    virtual void setNextMedia(const QMediaContent &media) = 0;

    virtual bool isCrossfadeSupported() const = 0;
    virtual qreal crossfadeTime() const = 0;
    virtual void setCrossfadeTime(qreal crossfadeTime) = 0;

    MULTI_CS_SIGNAL_1(Public, void crossfadeTimeChanged(qreal crossfadeTime))
    MULTI_CS_SIGNAL_2(crossfadeTimeChanged,crossfadeTime)
    MULTI_CS_SIGNAL_1(Public, void nextMediaChanged(const QMediaContent & media))
    MULTI_CS_SIGNAL_2(nextMediaChanged,media)
    MULTI_CS_SIGNAL_1(Public, void advancedToNextMedia())
    MULTI_CS_SIGNAL_2(advancedToNextMedia)

protected:
    explicit QMediaGaplessPlaybackControl(QObject *parent = nullptr);
};

#define QMediaGaplessPlaybackControl_iid "com.copperspice.CS.mediaGaplessPlaybackControl/1.0"
CS_DECLARE_INTERFACE(QMediaGaplessPlaybackControl, QMediaGaplessPlaybackControl_iid)

#endif
