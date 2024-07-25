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

#ifndef QCAMERA_P_H
#define QCAMERA_P_H

#include <qcamera.h>

#include <qmediaobject_p.h>
#include <qvideosurfaceoutput_p.h>

class QCameraControl;
class QCameraInfoControl;
class QCameraLocksControl;
class QCameraViewfinderSettingsControl2;
class QCameraViewfinderSettingsControl;
class QMediaServiceProvider;
class QVideoDeviceSelectorControl;

class QCameraPrivate : public QMediaObjectPrivate
{
 public:
   QCameraPrivate()
      : QMediaObjectPrivate(), provider(nullptr), control(nullptr), deviceControl(nullptr),
        locksControl(nullptr), infoControl(nullptr), viewfinderSettingsControl(nullptr),
        viewfinderSettingsControl2(nullptr), cameraExposure(nullptr), cameraFocus(nullptr),
        imageProcessing(nullptr), viewfinder(nullptr), capture(nullptr), state(QCamera::UnloadedState),
        error(QCamera::NoError), requestedLocks(QCamera::NoLock), lockStatus(QCamera::Unlocked),
        lockChangeReason(QCamera::UserRequest), supressLockChangedSignal(false), restartPending(false)
   {
   }

   void clear();
   void init();
   void initControls();

   QMediaServiceProvider *provider;

   QCameraControl *control;
   QVideoDeviceSelectorControl *deviceControl;
   QCameraLocksControl *locksControl;
   QCameraInfoControl *infoControl;
   QCameraViewfinderSettingsControl *viewfinderSettingsControl;
   QCameraViewfinderSettingsControl2 *viewfinderSettingsControl2;

   QCameraExposure *cameraExposure;
   QCameraFocus *cameraFocus;
   QCameraImageProcessing *imageProcessing;

   QObject *viewfinder;
   QObject *capture;

   QCamera::State state;

   QCamera::Error error;
   QString errorString;

   QCamera::LockTypes requestedLocks;

   QCamera::LockStatus lockStatus;
   QCamera::LockChangeReason lockChangeReason;
   bool supressLockChangedSignal;

   bool restartPending;

   QVideoSurfaceOutput surfaceViewfinder;

   void _q_error(int error, const QString &errorString);
   void unsetError() {
      error = QCamera::NoError;
      errorString.clear();
   }

   void setState(QCamera::State);

   void _q_updateLockStatus(QCamera::LockType, QCamera::LockStatus, QCamera::LockChangeReason);
   void _q_updateState(QCamera::State newState);
   void _q_preparePropertyChange(int changeType);
   void _q_restartCamera();
   void updateLockStatus();

 private:
   Q_DECLARE_NON_CONST_PUBLIC(QCamera)
};

#endif
