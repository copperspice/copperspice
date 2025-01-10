/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef DSIMAGECAPTURECONTROL_H
#define DSIMAGECAPTURECONTROL_H

#include <qcameraimagecapturecontrol.h>
#include <dscamera_session.h>

class DSImageCaptureControl : public QCameraImageCaptureControl
{
   CS_OBJECT(DSImageCaptureControl)

 public:
   DSImageCaptureControl(DSCameraSession *session);
   ~DSImageCaptureControl();

   bool isReadyForCapture() const override;
   int capture(const QString &fileName) override;

   QCameraImageCapture::DriveMode driveMode() const override;
   void setDriveMode(QCameraImageCapture::DriveMode mode) override;

   void cancelCapture() override {
   }

 private:
   DSCameraSession *m_session;
};

#endif
