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

#ifndef QIMAGEENCODERCONTROL_H
#define QIMAGEENCODERCONTROL_H

#include <qsize.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmediacontrol.h>
#include <qmediarecorder.h>
#include <qmediaencodersettings.h>

class QByteArray;

class Q_MULTIMEDIA_EXPORT QImageEncoderControl : public QMediaControl
{
    MULTI_CS_OBJECT(QImageEncoderControl)

public:
    virtual ~QImageEncoderControl();

    virtual QStringList supportedImageCodecs() const = 0;
    virtual QString imageCodecDescription(const QString &codecName) const = 0;

    virtual QList<QSize> supportedResolutions(const QImageEncoderSettings &settings,
                                              bool *continuous = nullptr) const = 0;

    virtual QImageEncoderSettings imageSettings() const = 0;
    virtual void setImageSettings(const QImageEncoderSettings &settings) = 0;

protected:
    explicit QImageEncoderControl(QObject *parent = nullptr);
};

#define QImageEncoderControl_iid "com.copperspice.CS.imageEncoderControl/1.0"
CS_DECLARE_INTERFACE(QImageEncoderControl, QImageEncoderControl_iid)

#endif
