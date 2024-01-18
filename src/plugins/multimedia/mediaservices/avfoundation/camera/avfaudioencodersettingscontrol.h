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

#ifndef AVFAUDIOENCODERSETTINGSCONTROL_H
#define AVFAUDIOENCODERSETTINGSCONTROL_H

#include <qaudioencodersettingscontrol.h>

@class NSDictionary;
@class AVCaptureAudioDataOutput;

class AVFCameraService;

class AVFAudioEncoderSettingsControl : public QAudioEncoderSettingsControl
{
 public:
   explicit AVFAudioEncoderSettingsControl(AVFCameraService *service);

   QStringList supportedAudioCodecs() const override;
   QString codecDescription(const QString &codecName) const override;
   QList<int> supportedSampleRates(const QAudioEncoderSettings &settings, bool *continuous = nullptr) const override;
   QAudioEncoderSettings audioSettings() const override;
   void setAudioSettings(const QAudioEncoderSettings &settings) override;

   NSDictionary *applySettings();
   void unapplySettings();

 private:
   AVFCameraService *m_service;

   QAudioEncoderSettings m_requestedSettings;
   QAudioEncoderSettings m_actualSettings;
};

#endif
