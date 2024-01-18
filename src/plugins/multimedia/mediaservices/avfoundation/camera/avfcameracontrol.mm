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

#include "avfcameradebug.h"
#include "avfcameracontrol.h"
#include "avfcamerasession.h"
#include "avfcameraservice.h"

AVFCameraControl::AVFCameraControl(AVFCameraService *service, QObject *parent)
   : QCameraControl(parent), m_session(service->session()), m_state(QCamera::UnloadedState),
     m_lastStatus(QCamera::UnloadedStatus) , m_captureMode(QCamera::CaptureStillImage)
{
    (void) service;
    connect(m_session, SIGNAL(stateChanged(QCamera::State)), SLOT(updateStatus()));
    connect(this, &AVFCameraControl::captureModeChanged, m_session, &AVFCameraSession::onCaptureModeChanged);
}

AVFCameraControl::~AVFCameraControl()
{
}

QCamera::State AVFCameraControl::state() const
{
    return m_state;
}

void AVFCameraControl::setState(QCamera::State state)
{
    if (m_state == state)
        return;
    m_state = state;
    m_session->setState(state);

    Q_EMIT stateChanged(m_state);
    updateStatus();
}

QCamera::Status AVFCameraControl::status() const
{
    static QCamera::Status statusTable[3][3] = {
        { QCamera::UnloadedStatus, QCamera::UnloadingStatus, QCamera::StoppingStatus }, //Unloaded state
        { QCamera::LoadingStatus,  QCamera::LoadedStatus,    QCamera::StoppingStatus }, //Loaded state
        { QCamera::LoadingStatus,  QCamera::StartingStatus,  QCamera::ActiveStatus } //ActiveState
    };

    return statusTable[m_state][m_session->state()];
}

void AVFCameraControl::updateStatus()
{
    QCamera::Status newStatus = status();

    if (m_lastStatus != newStatus) {
        qDebugCamera() << "Camera status changed: " << m_lastStatus << " -> " << newStatus;
        m_lastStatus = newStatus;
        Q_EMIT statusChanged(m_lastStatus);
    }
}

QCamera::CaptureModes AVFCameraControl::captureMode() const
{
    return m_captureMode;
}

void AVFCameraControl::setCaptureMode(QCamera::CaptureModes mode)
{
    if (m_captureMode == mode)
        return;

    if (!isCaptureModeSupported(mode)) {
        Q_EMIT error(QCamera::NotSupportedFeatureError, tr("Requested capture mode is not supported"));
        return;
    }

    m_captureMode = mode;
    Q_EMIT captureModeChanged(mode);
}

bool AVFCameraControl::isCaptureModeSupported(QCamera::CaptureModes mode) const
{
    //all the capture modes are supported, including QCamera::CaptureStillImage | QCamera::CaptureVideo
    return (mode & (QCamera::CaptureStillImage | QCamera::CaptureVideo)) == mode;
}

bool AVFCameraControl::canChangeProperty(QCameraControl::PropertyChangeType changeType, QCamera::Status status) const
{
    (void) changeType;
    (void) status;

    return true;
}
