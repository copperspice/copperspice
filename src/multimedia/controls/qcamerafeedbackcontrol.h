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

#ifndef QCAMERAFEEDBACKCONTROL_H
#define QCAMERAFEEDBACKCONTROL_H

#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qcamera.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraFeedbackControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraFeedbackControl)

 public:
   enum EventType {
      ViewfinderStarted = 1,
      ViewfinderStopped,
      ImageCaptured,
      ImageSaved,
      ImageError,
      RecordingStarted,
      RecordingInProgress,
      RecordingStopped,
      AutoFocusInProgress,
      AutoFocusLocked,
      AutoFocusFailed
   };

   ~QCameraFeedbackControl();

   virtual bool isEventFeedbackLocked(EventType event) const = 0;

   virtual bool isEventFeedbackEnabled(EventType event) const = 0;

   virtual bool setEventFeedbackEnabled(EventType event, bool enabled) = 0;
   virtual void resetEventFeedback(EventType event) = 0;

   virtual bool setEventFeedbackSound(EventType event, const QString &filePath) = 0;

 protected:
   explicit QCameraFeedbackControl(QObject *parent = nullptr);
};

#define QCameraFeedbackControl_iid "com.copperspice.CS.cameraFeedBackControl/1.0"
CS_DECLARE_INTERFACE(QCameraFeedbackControl, QCameraFeedbackControl_iid)

#endif
