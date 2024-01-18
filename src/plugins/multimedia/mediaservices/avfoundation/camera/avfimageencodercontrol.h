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

#ifndef AVFIMAGEENCODERCONTROL_H
#define AVFIMAGEENCODERCONTROL_H

#include <qmediaencodersettings.h>
#include <qimageencodercontrol.h>

#include <qglobal.h>
#include <qstring.h>
#include <qlist.h>

@class AVCaptureDeviceFormat;

class AVFCameraService;

class AVFImageEncoderControl : public QImageEncoderControl
{
   CS_OBJECT(AVFImageEncoderControl)

 public:
   AVFImageEncoderControl(AVFCameraService *service);

   QStringList supportedImageCodecs() const override;
   QString imageCodecDescription(const QString &codecName) const override;
   QList<QSize> supportedResolutions(const QImageEncoderSettings &settings, bool *continuous) const override;
   QImageEncoderSettings imageSettings() const override;
   void setImageSettings(const QImageEncoderSettings &settings) override;

   QImageEncoderSettings requestedSettings() const;

 private:
   AVFCameraService *m_service;
   QImageEncoderSettings m_settings;

   bool applySettings();
   bool videoCaptureDeviceIsValid() const;

   friend class AVFCameraSession;
};

QSize qt_image_high_resolution(AVCaptureDeviceFormat *fomat);

#endif
