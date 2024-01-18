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

#ifndef QCAMERAEXPOSURECONTROL_H
#define QCAMERAEXPOSURECONTROL_H

#include <qcameraexposure.h>
#include <qcamera.h>
#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraExposureControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraExposureControl)

   MULTI_CS_ENUM(ExposureParameter)

 public:
   ~QCameraExposureControl();

   enum ExposureParameter {
      ISO,
      Aperture,
      ShutterSpeed,
      ExposureCompensation,
      FlashPower,
      FlashCompensation,
      TorchPower,
      SpotMeteringPoint,
      ExposureMode,
      MeteringMode,
      ExtendedExposureParameter = 1000
   };

   virtual bool isParameterSupported(ExposureParameter parameter) const = 0;
   virtual QVariantList supportedParameterRange(ExposureParameter parameter, bool *continuous) const = 0;

   virtual QVariant requestedValue(ExposureParameter parameter) const = 0;
   virtual QVariant actualValue(ExposureParameter parameter) const = 0;
   virtual bool setValue(ExposureParameter parameter, const QVariant &value) = 0;

   MULTI_CS_SIGNAL_1(Public, void requestedValueChanged(int parameter))
   MULTI_CS_SIGNAL_2(requestedValueChanged, parameter)

   MULTI_CS_SIGNAL_1(Public, void actualValueChanged(int parameter))
   MULTI_CS_SIGNAL_2(actualValueChanged, parameter)

   MULTI_CS_SIGNAL_1(Public, void parameterRangeChanged(int parameter))
   MULTI_CS_SIGNAL_2(parameterRangeChanged, parameter)

 protected:
   explicit QCameraExposureControl(QObject *parent = nullptr);
};

#define QCameraExposureControl_iid  "com.copperspice.CS.cameraExposureControl/1.0"
CS_DECLARE_INTERFACE(QCameraExposureControl, QCameraExposureControl_iid)

#endif

