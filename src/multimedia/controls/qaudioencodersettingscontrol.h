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

#ifndef QAUDIOENCODERSETTINGSCONTROL_H
#define QAUDIOENCODERSETTINGSCONTROL_H

#include <qlist.h>
#include <qstring.h>
#include <qpair.h>
#include <qmediacontrol.h>
#include <qmediarecorder.h>

class QStringList;
class QAudioFormat;

class Q_MULTIMEDIA_EXPORT QAudioEncoderSettingsControl : public QMediaControl
{
    MULTI_CS_OBJECT(QAudioEncoderSettingsControl)

public:
    virtual ~QAudioEncoderSettingsControl();

    virtual QStringList supportedAudioCodecs() const = 0;
    virtual QString codecDescription(const QString &codecName) const = 0;

    virtual QList<int> supportedSampleRates(const QAudioEncoderSettings &settings, bool *continuous = nullptr) const = 0;

    virtual QAudioEncoderSettings audioSettings() const = 0;
    virtual void setAudioSettings(const QAudioEncoderSettings &settings) = 0;

protected:
    explicit QAudioEncoderSettingsControl(QObject *parent = nullptr);
};

#define QAudioEncoderSettingsControl_iid "com.copperspice.CS.audioEncoderSettingsControl/1.0"
CS_DECLARE_INTERFACE(QAudioEncoderSettingsControl, QAudioEncoderSettingsControl_iid)

#endif