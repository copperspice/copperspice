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

#ifndef QCAMERAIMAGEPROCESSINGCONTROL_H
#define QCAMERAIMAGEPROCESSINGCONTROL_H

#include <qcamera.h>
#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraImageProcessingControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraImageProcessingControl)

   MULTI_CS_ENUM(ProcessingParameter)

 public:
   ~QCameraImageProcessingControl();

   enum ProcessingParameter {
      WhiteBalancePreset,
      ColorTemperature,
      Contrast,
      Saturation,
      Brightness,
      Sharpening,
      Denoising,
      ContrastAdjustment,
      SaturationAdjustment,
      BrightnessAdjustment,
      SharpeningAdjustment,
      DenoisingAdjustment,
      ColorFilter,
      ExtendedParameter = 1000
   };

   virtual bool isParameterSupported(ProcessingParameter) const = 0;
   virtual bool isParameterValueSupported(ProcessingParameter parameter, const QVariant &value) const = 0;
   virtual QVariant parameter(ProcessingParameter parameter) const = 0;
   virtual void setParameter(ProcessingParameter parameter, const QVariant &value) = 0;

 protected:
   explicit QCameraImageProcessingControl(QObject *parent = nullptr);
};

#define QCameraImageProcessingControl_iid "com.copperspice.CS.cameraImageProcessingControl/1.0"
CS_DECLARE_INTERFACE(QCameraImageProcessingControl, QCameraImageProcessingControl_iid)

#endif

