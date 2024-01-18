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

#ifndef AVFVIDEOENCODERSETTINGSCONTROL_H
#define AVFVIDEOENCODERSETTINGSCONTROL_H

#include <qvideoencodersettingscontrol.h>

#include <avfcamerautility.h>

#import <AVFoundation/AVFoundation.h>

@class NSDictionary;

class AVFCameraService;

class AVFVideoEncoderSettingsControl : public QVideoEncoderSettingsControl
{
   CS_OBJECT(AVFVideoEncoderSettingsControl)

 public:
   explicit AVFVideoEncoderSettingsControl(AVFCameraService *service);

   QList<QSize> supportedResolutions(const QVideoEncoderSettings &requestedVideoSettings,
            bool *continuous = nullptr) const override;

   QList<qreal> supportedFrameRates(const QVideoEncoderSettings &requestedVideoSettings,
            bool *continuous = nullptr) const override;

   QStringList supportedVideoCodecs() const override;
   QString videoCodecDescription(const QString &codecName) const override;

   QVideoEncoderSettings videoSettings() const override;
   void setVideoSettings(const QVideoEncoderSettings &requestedVideoSettings) override;

   NSDictionary *applySettings(AVCaptureConnection *connection);
   void unapplySettings(AVCaptureConnection *connection);

 private:
   AVFCameraService *m_service;

   QVideoEncoderSettings m_requestedSettings;
   QVideoEncoderSettings m_actualSettings;

   AVCaptureDeviceFormat *m_restoreFormat;
   AVFPSRange m_restoreFps;
};

#endif
