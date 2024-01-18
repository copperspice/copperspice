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

#ifndef AVFCAMERAVIEWFINDERSETTINGSCONTROL_H
#define AVFCAMERAVIEWFINDERSETTINGSCONTROL_H

#include <qcameraviewfindersettingscontrol.h>
#include <qcameraviewfindersettings.h>
#include <qvideoframe.h>
#include <qpointer.h>
#include <qglobal.h>
#include <qsize.h>

@class AVCaptureDevice;
@class AVCaptureVideoDataOutput;
@class AVCaptureConnection;
@class AVCaptureDeviceFormat;

class AVFCameraSession;
class AVFCameraService;

class AVFCameraViewfinderSettingsControl2 : public QCameraViewfinderSettingsControl2
{
   CS_OBJECT(AVFCameraViewfinderSettingsControl2)

 public:
   AVFCameraViewfinderSettingsControl2(AVFCameraService *service);

   QList<QCameraViewfinderSettings> supportedViewfinderSettings() const override;
   QCameraViewfinderSettings viewfinderSettings() const override;
   void setViewfinderSettings(const QCameraViewfinderSettings &settings) override;

   // "Converters":
   static QVideoFrame::PixelFormat QtPixelFormatFromCVFormat(unsigned avPixelFormat);
   static bool CVPixelFormatFromQtFormat(QVideoFrame::PixelFormat qtFormat, unsigned &conv);

 private:
   void setResolution(const QSize &resolution);
   void setFramerate(qreal minFPS, qreal maxFPS, bool useActive);
   void setPixelFormat(QVideoFrame::PixelFormat newFormat);
   AVCaptureDeviceFormat *findBestFormatMatch(const QCameraViewfinderSettings &settings) const;
   QVector<QVideoFrame::PixelFormat> viewfinderPixelFormats() const;
   bool convertPixelFormatIfSupported(QVideoFrame::PixelFormat format, unsigned &avfFormat) const;
   bool applySettings(const QCameraViewfinderSettings &settings);
   QCameraViewfinderSettings requestedSettings() const;

   AVCaptureConnection *videoConnection() const;

   AVFCameraService *m_service;
   QCameraViewfinderSettings m_settings;

   friend class AVFCameraSession;
   friend class AVFCameraViewfinderSettingsControl;
};

class AVFCameraViewfinderSettingsControl : public QCameraViewfinderSettingsControl
{
   CS_OBJECT(AVFCameraViewfinderSettingsControl)

 public:
   AVFCameraViewfinderSettingsControl(AVFCameraService *service);

   bool isViewfinderParameterSupported(ViewfinderParameter parameter) const override;
   QVariant viewfinderParameter(ViewfinderParameter parameter) const override;
   void setViewfinderParameter(ViewfinderParameter parameter, const QVariant &value) override;

 private:
   void setResolution(const QVariant &resolution);
   void setAspectRatio(const QVariant &aspectRatio);
   void setFrameRate(const QVariant &fps, bool max);
   void setPixelFormat(const QVariant &pf);
   bool initSettingsControl() const;

   AVFCameraService *m_service;
   mutable QPointer<AVFCameraViewfinderSettingsControl2> m_settingsControl;
};

#endif
