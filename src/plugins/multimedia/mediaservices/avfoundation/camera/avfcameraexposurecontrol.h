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

#ifndef AVFCAMERAEXPOSURECONTROL_H
#define AVFCAMERAEXPOSURECONTROL_H

#include <qcameraexposurecontrol.h>
#include <qcameraexposure.h>
#include <qglobal.h>

class AVFCameraSession;
class AVFCameraService;

class AVFCameraExposureControl : public QCameraExposureControl
{
   CS_OBJECT(AVFCameraExposureControl)

 public:
   AVFCameraExposureControl(AVFCameraService *service);

   bool isParameterSupported(ExposureParameter parameter) const override;
   QVariantList supportedParameterRange(ExposureParameter parameter, bool *continuous) const override;

   QVariant requestedValue(ExposureParameter parameter) const override;
   QVariant actualValue(ExposureParameter parameter) const override;
   bool setValue(ExposureParameter parameter, const QVariant &value) override;

 private:
   CS_SLOT_1(Private, void cameraStateChanged())
   CS_SLOT_2(cameraStateChanged)

   AVFCameraService *m_service;
   AVFCameraSession *m_session;

   QVariant m_requestedMode;
   QVariant m_requestedCompensation;
   QVariant m_requestedShutterSpeed;
   QVariant m_requestedISO;

   // Aux setters
   bool setExposureMode(const QVariant &value);
   bool setExposureCompensation(const QVariant &value);
   bool setShutterSpeed(const QVariant &value);
   bool setISO(const QVariant &value);
};

#endif
