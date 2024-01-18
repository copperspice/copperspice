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

#ifndef AVFAUDIOINPUTSELECTORCONTROL_H
#define AVFAUDIOINPUTSELECTORCONTROL_H

#include <qaudioinputselectorcontrol.h>
#include <qstringlist.h>

#import <AVFoundation/AVFoundation.h>

class AVFCameraSession;
class AVFCameraService;

class AVFAudioInputSelectorControl : public QAudioInputSelectorControl
{
   CS_OBJECT(AVFAudioInputSelectorControl)

 public:
   AVFAudioInputSelectorControl(AVFCameraService *service, QObject *parent = nullptr);
   ~AVFAudioInputSelectorControl();

   QList<QString> availableInputs() const override;
   QString inputDescription(const QString &name) const override;
   QString defaultInput() const override;
   QString activeInput() const override;

   CS_SLOT_1(Public, void setActiveInput(const QString &name) override)
   CS_SLOT_2(setActiveInput)

   //device changed since the last createCaptureDevice()
   bool isDirty() const {
      return m_dirty;
   }
   AVCaptureDevice *createCaptureDevice();

 private:
   QString m_activeInput;
   bool m_dirty;
   QString m_defaultDevice;
   QStringList m_devices;
   QMap<QString, QString> m_deviceDescriptions;
};

#endif
