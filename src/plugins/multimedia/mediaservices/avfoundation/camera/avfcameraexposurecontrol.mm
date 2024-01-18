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

#include "avfcameraexposurecontrol.h"
#include "avfcamerautility.h"
#include "avfcamerasession.h"
#include "avfcameraservice.h"
#include "avfcameradebug.h"

#include <qvariant.h>
#include <qpointer.h>
#include <qdebug.h>
#include <qpair.h>

#include <AVFoundation/AVFoundation.h>

#include <limits>

AVFCameraExposureControl::AVFCameraExposureControl(AVFCameraService *service)
    : m_service(service), m_session(nullptr)
{
    Q_ASSERT(service);
    m_session = m_service->session();
    Q_ASSERT(m_session);

    connect(m_session, SIGNAL(stateChanged(QCamera::State)), SLOT(cameraStateChanged()));
}

bool AVFCameraExposureControl::isParameterSupported(ExposureParameter parameter) const
{
   (void) parameter;
   return false;
}

QVariantList AVFCameraExposureControl::supportedParameterRange(ExposureParameter parameter, bool *continuous) const
{
   (void) parameter;
   (void) continuous;
   return QVariantList();
}

QVariant AVFCameraExposureControl::requestedValue(ExposureParameter parameter) const
{
   (void) parameter;
    return QVariant();
}

QVariant AVFCameraExposureControl::actualValue(ExposureParameter parameter) const
{
   (void) parameter;
   return QVariant();
}

bool AVFCameraExposureControl::setValue(ExposureParameter parameter, const QVariant &value)
{
   (void) parameter;
   (void) value;
   return false;
}

bool AVFCameraExposureControl::setExposureMode(const QVariant &value)
{
   (void) value;
   return false;
}

bool AVFCameraExposureControl::setExposureCompensation(const QVariant &value)
{
   (void) value;
   return false;
}

bool AVFCameraExposureControl::setShutterSpeed(const QVariant &value)
{
   (void) value;
   return false;
}

bool AVFCameraExposureControl::setISO(const QVariant &value)
{
   (void) value;
   return false;
}

void AVFCameraExposureControl::cameraStateChanged()
{
}

