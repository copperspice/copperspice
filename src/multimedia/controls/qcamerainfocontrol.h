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

#ifndef QCAMERAINFOCONTROL_H
#define QCAMERAINFOCONTROL_H

#include <qcamera.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraInfoControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraInfoControl)

 public:
   virtual ~QCameraInfoControl();

   virtual QCamera::Position cameraPosition(const QString &deviceName) const = 0;
   virtual int cameraOrientation(const QString &deviceName) const = 0;

 protected:
   explicit QCameraInfoControl(QObject *parent = nullptr);
};

#define QCameraInfoControl_iid "com.copperspice.CS.cameraInfoControl/1.0"
CS_DECLARE_INTERFACE(QCameraInfoControl, QCameraInfoControl_iid)

#endif
