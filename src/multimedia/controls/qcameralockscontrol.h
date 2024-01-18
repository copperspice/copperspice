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

#ifndef QCAMERALOCKSCONTROL_H
#define QCAMERALOCKSCONTROL_H

#include <qcamera.h>
#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraLocksControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraLocksControl)

 public:
   ~QCameraLocksControl();

   virtual QCamera::LockTypes supportedLocks() const = 0;

   virtual QCamera::LockStatus lockStatus(QCamera::LockType lockType) const = 0;

   virtual void searchAndLock(QCamera::LockTypes locks) = 0;
   virtual void unlock(QCamera::LockTypes locks) = 0;

   MULTI_CS_SIGNAL_1(Public, void lockStatusChanged(QCamera::LockType lockType, QCamera::LockStatus status, QCamera::LockChangeReason reason))
   MULTI_CS_SIGNAL_2(lockStatusChanged, lockType, status, reason)

 protected:
   explicit QCameraLocksControl(QObject *parent = nullptr);
};

#define QCameraLocksControl_iid "com.copperspice.CS.cameraLocksControl/1.0"
CS_DECLARE_INTERFACE(QCameraLocksControl, QCameraLocksControl_iid)

#endif

