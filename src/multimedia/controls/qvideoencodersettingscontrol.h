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

#ifndef QVIDEOENCODERSETTINGSCONTROL_H
#define QVIDEOENCODERSETTINGSCONTROL_H

#include <qmediacontrol.h>
#include <qmediarecorder.h>

#include <qpair.h>
#include <qsize.h>
#include <qstring.h>

class QByteArray;
class QStringList;

class Q_MULTIMEDIA_EXPORT QVideoEncoderSettingsControl : public QMediaControl
{
    MULTI_CS_OBJECT(QVideoEncoderSettingsControl)

public:
    virtual ~QVideoEncoderSettingsControl();

    virtual QList<QSize> supportedResolutions(const QVideoEncoderSettings &settings, bool *continuous = nullptr) const = 0;
    virtual QList<qreal> supportedFrameRates(const QVideoEncoderSettings &settings, bool *continuous = nullptr) const = 0;

    virtual QStringList supportedVideoCodecs() const = 0;
    virtual QString videoCodecDescription(const QString &codecName) const = 0;

    virtual QVideoEncoderSettings videoSettings() const = 0;
    virtual void setVideoSettings(const QVideoEncoderSettings &settings) = 0;

protected:
    explicit QVideoEncoderSettingsControl(QObject *parent = nullptr);
};

#define QVideoEncoderSettingsControl_iid "com.copperspice.CS.videoEncoderSettingsControl/1.0"
CS_DECLARE_INTERFACE(QVideoEncoderSettingsControl, QVideoEncoderSettingsControl_iid)

#endif
