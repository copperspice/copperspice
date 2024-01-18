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

#ifndef QCAMERAVIEWFINDERSETTINGSCONTROL_H
#define QCAMERAVIEWFINDERSETTINGSCONTROL_H

#include <qmediacontrol.h>
#include <qcamera.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraViewfinderSettingsControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraViewfinderSettingsControl)

 public:
   enum ViewfinderParameter {
      Resolution,
      PixelAspectRatio,
      MinimumFrameRate,
      MaximumFrameRate,
      PixelFormat,
      UserParameter = 1000
   };

   ~QCameraViewfinderSettingsControl();

   virtual bool isViewfinderParameterSupported(ViewfinderParameter parameter) const = 0;
   virtual QVariant viewfinderParameter(ViewfinderParameter parameter) const = 0;
   virtual void setViewfinderParameter(ViewfinderParameter parameter, const QVariant &value) = 0;

 protected:
   explicit QCameraViewfinderSettingsControl(QObject *parent = nullptr);
};

class Q_MULTIMEDIA_EXPORT QCameraViewfinderSettingsControl2 : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraViewfinderSettingsControl2)

 public:
   virtual ~QCameraViewfinderSettingsControl2();

   virtual QList<QCameraViewfinderSettings> supportedViewfinderSettings() const = 0;

   virtual QCameraViewfinderSettings viewfinderSettings() const = 0;
   virtual void setViewfinderSettings(const QCameraViewfinderSettings &settings) = 0;

 protected:
   explicit QCameraViewfinderSettingsControl2(QObject *parent = nullptr);
};

#define QCameraViewfinderSettingsControl_iid "com.copperspice.CS.cameraViewfinderSettingsControl/1.0"
CS_DECLARE_INTERFACE(QCameraViewfinderSettingsControl, QCameraViewfinderSettingsControl_iid)

#define QCameraViewfinderSettingsControl2_iid "com.copperspice.CS.cameraViewfinderSettingsControl2/1.0"
CS_DECLARE_INTERFACE(QCameraViewfinderSettingsControl2, QCameraViewfinderSettingsControl2_iid)

#endif
