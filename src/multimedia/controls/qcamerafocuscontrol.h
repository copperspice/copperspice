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

#ifndef QCAMERAFOCUSCONTROL_H
#define QCAMERAFOCUSCONTROL_H

#include <qcamerafocus.h>
#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraFocusControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraFocusControl)

 public:
   ~QCameraFocusControl();

   virtual QCameraFocus::FocusModes focusMode() const = 0;
   virtual void setFocusMode(QCameraFocus::FocusModes mode) = 0;
   virtual bool isFocusModeSupported(QCameraFocus::FocusModes mode) const = 0;

   virtual QCameraFocus::FocusPointMode focusPointMode() const = 0;
   virtual void setFocusPointMode(QCameraFocus::FocusPointMode mode) = 0;
   virtual bool isFocusPointModeSupported(QCameraFocus::FocusPointMode mode) const = 0;
   virtual QPointF customFocusPoint() const = 0;
   virtual void setCustomFocusPoint(const QPointF &point) = 0;

   virtual QCameraFocusZoneList focusZones() const = 0;

   MULTI_CS_SIGNAL_1(Public, void focusModeChanged(QCameraFocus::FocusModes mode))
   MULTI_CS_SIGNAL_2(focusModeChanged, mode)

   MULTI_CS_SIGNAL_1(Public, void focusPointModeChanged(QCameraFocus::FocusPointMode mode))
   MULTI_CS_SIGNAL_2(focusPointModeChanged, mode)

   MULTI_CS_SIGNAL_1(Public, void customFocusPointChanged(const QPointF &point))
   MULTI_CS_SIGNAL_2(customFocusPointChanged, point)

   MULTI_CS_SIGNAL_1(Public, void focusZonesChanged())
   MULTI_CS_SIGNAL_2(focusZonesChanged)

 protected:
   explicit QCameraFocusControl(QObject *parent = nullptr);
};

#define QCameraFocusControl_iid "com.copperspice.CS.cameraFocusControl/1.0"
CS_DECLARE_INTERFACE(QCameraFocusControl, QCameraFocusControl_iid)

#endif

