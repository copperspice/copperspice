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

#include <avfcameraserviceplugin.h>
#include <avfcameraservice.h>
#include <avfcamerasession.h>
#include <qdebug.h>
#include <qmediaservice_provider_plugin.h>
#include <string.h>

CS_PLUGIN_REGISTER(AVFServicePlugin)

AVFServicePlugin::AVFServicePlugin()
{
}

QMediaService *AVFServicePlugin::create(const QString &key)
{
    if (key == Q_MEDIASERVICE_CAMERA)
        return new AVFCameraService;
    else
        qWarning() << "AVFoundation camera plugin, unsupported key:" << key;

    return nullptr;
}

void AVFServicePlugin::release(QMediaService *service)
{
    delete service;
}

QString AVFServicePlugin::defaultDevice(const QString &service) const
{
    if (service == Q_MEDIASERVICE_CAMERA) {
        int i = AVFCameraSession::defaultCameraIndex();

        if (i != -1)
            return AVFCameraSession::availableCameraDevices().at(i).deviceId;
    }

    return QByteArray();
}

QList<QString> AVFServicePlugin::devices(const QString &service) const
{
    QList<QString> devs;

    if (service == Q_MEDIASERVICE_CAMERA) {
        const QList<AVFCameraInfo> &cameras = AVFCameraSession::availableCameraDevices();

        for (const AVFCameraInfo &info : cameras)
            devs.append(info.deviceId);
    }

    return devs;
}

QString AVFServicePlugin::deviceDescription(const QString &service, const QString &device)
{
    if (service == Q_MEDIASERVICE_CAMERA)
        return AVFCameraSession::cameraDeviceInfo(device).description;

    return QString();
}

QCamera::Position AVFServicePlugin::cameraPosition(const QString &device) const
{
    return AVFCameraSession::cameraDeviceInfo(device).position;
}

int AVFServicePlugin::cameraOrientation(const QString &device) const
{
    return AVFCameraSession::cameraDeviceInfo(device).orientation;
}

