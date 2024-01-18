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

#include <avfmediacontainercontrol.h>
#include <qstringlist.h>

#include <AVFoundation/AVMediaFormat.h>

struct ContainerInfo
{
    QString description;
    NSString *fileType;

    ContainerInfo()
      : fileType(nil)
    {
    }

    ContainerInfo(const QString &desc, NSString *type)
        : description(desc), fileType(type)
    {
    }
};

using SupportedContainers = QMap<QString, ContainerInfo>;

static SupportedContainers *containers()
{
   static SupportedContainers retval;
   return &retval;
}

AVFMediaContainerControl::AVFMediaContainerControl(AVFCameraService *)
    : QMediaContainerControl(), m_format("mov") // .mov is the default container format on Apple platforms
{
    if (containers()->isEmpty()) {
        containers()->insert("mov", ContainerInfo("QuickTime movie file format", AVFileTypeQuickTimeMovie));
        containers()->insert("mp4", ContainerInfo("MPEG-4 file format", AVFileTypeMPEG4));
        containers()->insert("m4v", ContainerInfo("iTunes video file format", AVFileTypeAppleM4V));
    }
}

QStringList AVFMediaContainerControl::supportedContainers() const
{
    return containers()->keys();
}

QString AVFMediaContainerControl::containerFormat() const
{
    return m_format;
}

void AVFMediaContainerControl::setContainerFormat(const QString &format)
{
    if (! containers()->contains(format)) {
        qWarning("Unsupported container format: '%s'", csPrintable(format));
        return;
    }

    m_format = format;
}

QString AVFMediaContainerControl::containerDescription(const QString &formatMimeType) const
{
    return containers()->value(formatMimeType).description;
}

NSString *AVFMediaContainerControl::fileType() const
{
    return containers()->value(m_format).fileType;
}


