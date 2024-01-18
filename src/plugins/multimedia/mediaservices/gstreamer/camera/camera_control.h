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

#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#include <qhash.h>
#include <qcameracontrol.h>
#include <camera_session.h>

class CamerabinResourcePolicy;

class CameraBinControl : public QCameraControl
{
   CS_OBJECT(CameraBinControl)

   CS_PROPERTY_READ(viewfinderColorSpaceConversion, viewfinderColorSpaceConversion)
   CS_PROPERTY_WRITE(viewfinderColorSpaceConversion, setViewfinderColorSpaceConversion)

 public:
   CameraBinControl( CameraBinSession *session );
   virtual ~CameraBinControl();

   bool isValid() const {
      return true;
   }

   QCamera::State state() const override;
   void setState(QCamera::State state) override;

   QCamera::Status status() const override;

   QCamera::CaptureModes captureMode() const override;
   void setCaptureMode(QCamera::CaptureModes mode) override;

   bool isCaptureModeSupported(QCamera::CaptureModes mode) const override;
   bool canChangeProperty(PropertyChangeType changeType, QCamera::Status status) const override;
   bool viewfinderColorSpaceConversion() const;

   CamerabinResourcePolicy *resourcePolicy() {
      return m_resourcePolicy;
   }

   CS_SLOT_1(Public, void reloadLater())
   CS_SLOT_2(reloadLater)

   CS_SLOT_1(Public, void setViewfinderColorSpaceConversion(bool enabled))
   CS_SLOT_2(setViewfinderColorSpaceConversion)

 private:
   CS_SLOT_1(Private, void delayedReload())
   CS_SLOT_2(delayedReload)

   CS_SLOT_1(Private, void handleResourcesGranted())
   CS_SLOT_2(handleResourcesGranted)

   CS_SLOT_1(Private, void handleResourcesLost())
   CS_SLOT_2(handleResourcesLost)

   CS_SLOT_1(Private, void handleBusyChanged(bool busy))
   CS_SLOT_2(handleBusyChanged)

   CS_SLOT_1(Private, void handleCameraError(int error, const QString &errorString))
   CS_SLOT_2(handleCameraError)

   void updateSupportedResolutions(const QString &device);

   CameraBinSession *m_session;
   QCamera::State m_state;
   CamerabinResourcePolicy *m_resourcePolicy;

   bool m_reloadPending;
};

#endif