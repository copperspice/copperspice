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

#ifndef CAMERABINFOCUSCONTROL_H
#define CAMERABINFOCUSCONTROL_H

#include <qbasictimer.h>
#include <qcamera.h>
#include <qcamerafocuscontrol.h>
#include <qmutex.h>
#include <qvector.h>

#include <qgstreamerbufferprobe_p.h>

#include <gst/gst.h>
#include <glib.h>

class CameraBinSession;

class CameraBinFocus
   : public QCameraFocusControl
#if GST_CHECK_VERSION(1,0,0)
   , QGstreamerBufferProbe
#endif
{
   CS_OBJECT(CameraBinFocus)

 public:
   CameraBinFocus(CameraBinSession *session);
   virtual ~CameraBinFocus();

   QCameraFocus::FocusModes focusMode() const;
   void setFocusMode(QCameraFocus::FocusModes mode);
   bool isFocusModeSupported(QCameraFocus::FocusModes mode) const;

   QCameraFocus::FocusPointMode focusPointMode() const;
   void setFocusPointMode(QCameraFocus::FocusPointMode mode) ;
   bool isFocusPointModeSupported(QCameraFocus::FocusPointMode) const;
   QPointF customFocusPoint() const;
   void setCustomFocusPoint(const QPointF &point);

   QCameraFocusZoneList focusZones() const;

   void handleFocusMessage(GstMessage *);
   QCamera::LockStatus focusStatus() const {
      return m_focusStatus;
   }

 public:
   CS_SIGNAL_1(Public, void _q_focusStatusChanged(QCamera::LockStatus status, QCamera::LockChangeReason reason))
   CS_SIGNAL_2(_q_focusStatusChanged, status, reason)

 public :
   CS_SLOT_1(Public, void _q_startFocusing())
   CS_SLOT_2(_q_startFocusing)
   CS_SLOT_1(Public, void _q_stopFocusing())
   CS_SLOT_2(_q_stopFocusing)

   CS_SLOT_1(Public, void setViewfinderResolution(const QSize &resolution))
   CS_SLOT_2(setViewfinderResolution)

#if GST_CHECK_VERSION(1,0,0)
 protected:
   void timerEvent(QTimerEvent *event);
#endif

 private :
   CS_SLOT_1(Private, void _q_setFocusStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason))
   CS_SLOT_2(_q_setFocusStatus)
   CS_SLOT_1(Private, void _q_handleCameraStatusChange(QCamera::Status status))
   CS_SLOT_2(_q_handleCameraStatusChange)

#if GST_CHECK_VERSION(1,0,0)
   CS_SLOT_1(Private, void _q_updateFaces())
   CS_SLOT_2(_q_updateFaces)
#endif

 private:
   void resetFocusPoint();
   void updateRegionOfInterest(const QRectF &rectangle);
   void updateRegionOfInterest(const QVector<QRect> &rectangles);

#if GST_CHECK_VERSION(1,0,0)
   bool probeBuffer(GstBuffer *buffer);
#endif

   CameraBinSession *m_session;
   QCamera::Status m_cameraStatus;
   QCameraFocus::FocusModes m_focusMode;
   QCameraFocus::FocusPointMode m_focusPointMode;
   QCamera::LockStatus m_focusStatus;
   QCameraFocusZone::FocusZoneStatus m_focusZoneStatus;
   QPointF m_focusPoint;
   QRectF m_focusRect;
   QSize m_viewfinderResolution;
   QVector<QRect> m_faces;
   QVector<QRect> m_faceFocusRects;
   QBasicTimer m_faceResetTimer;
   mutable QMutex m_mutex;
};

#endif
