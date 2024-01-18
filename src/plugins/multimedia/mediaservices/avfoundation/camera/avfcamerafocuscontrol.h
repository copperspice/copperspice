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

#ifndef AVFCAMERAFOCUSCONTROL_H
#define AVFCAMERAFOCUSCONTROL_H

#include <qscopedpointer.h>
#include <qglobal.h>
#include <qcamerafocuscontrol.h>

@class AVCaptureDevice;

class AVFCameraService;
class AVFCameraSession;

class AVFCameraFocusControl : public QCameraFocusControl
{
   CS_OBJECT(AVFCameraFocusControl)

 public:
   explicit AVFCameraFocusControl(AVFCameraService *service);

   QCameraFocus::FocusModes focusMode() const override;
   void setFocusMode(QCameraFocus::FocusModes mode) override;
   bool isFocusModeSupported(QCameraFocus::FocusModes mode) const override;

   QCameraFocus::FocusPointMode focusPointMode() const override;
   void setFocusPointMode(QCameraFocus::FocusPointMode mode) override;
   bool isFocusPointModeSupported(QCameraFocus::FocusPointMode mode) const override;
   QPointF customFocusPoint() const override;
   void setCustomFocusPoint(const QPointF &point) override;

   QCameraFocusZoneList focusZones() const override;

 private:
   CS_SLOT_1(Private, void cameraStateChanged())
   CS_SLOT_2(cameraStateChanged)

   AVFCameraSession *m_session;
   QCameraFocus::FocusModes m_focusMode;
   QCameraFocus::FocusPointMode m_focusPointMode;
   QPointF m_customFocusPoint;
   QPointF m_actualFocusPoint;
};

#endif
