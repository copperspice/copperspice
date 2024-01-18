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

#ifndef CAMERABINLOCKSCONTROL_H
#define CAMERABINLOCKSCONTROL_H

#include <qbasictimer.h>
#include <qcamera.h>
#include <qcameralockscontrol.h>

#include <gst/gst.h>
#include <glib.h>

class CameraBinSession;
class CameraBinFocus;

class CameraBinLocks  : public QCameraLocksControl
{
   CS_OBJECT(CameraBinLocks)

 public:
   CameraBinLocks(CameraBinSession *session);
   virtual ~CameraBinLocks();

   QCamera::LockTypes supportedLocks() const;

   QCamera::LockStatus lockStatus(QCamera::LockType lock) const;

   void searchAndLock(QCamera::LockTypes locks);
   void unlock(QCamera::LockTypes locks);

 protected:
#if GST_CHECK_VERSION(1, 2, 0)
   void timerEvent(QTimerEvent *event);
#endif

 private :
   CS_SLOT_1(Private, void updateFocusStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason))
   CS_SLOT_2(updateFocusStatus)

 private:
#if GST_CHECK_VERSION(1, 2, 0)
   bool isExposureLocked() const;
   void lockExposure(QCamera::LockChangeReason reason);
   void unlockExposure(QCamera::LockStatus status, QCamera::LockChangeReason reason);

   bool isWhiteBalanceLocked() const;
   void lockWhiteBalance(QCamera::LockChangeReason reason);
   void unlockWhiteBalance(QCamera::LockStatus status, QCamera::LockChangeReason reason);
#endif

   CameraBinSession *m_session;
   CameraBinFocus *m_focus;
   QBasicTimer m_lockTimer;
   QCamera::LockTypes m_pendingLocks;
};

#endif
