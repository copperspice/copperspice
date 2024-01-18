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

#ifndef QCAMERAZOOMCONTROL_H
#define QCAMERAZOOMCONTROL_H

#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraZoomControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraZoomControl)

 public:
   ~QCameraZoomControl();

   virtual qreal maximumOpticalZoom() const = 0;
   virtual qreal maximumDigitalZoom() const = 0;

   virtual qreal requestedOpticalZoom() const = 0;
   virtual qreal requestedDigitalZoom() const = 0;
   virtual qreal currentOpticalZoom() const = 0;
   virtual qreal currentDigitalZoom() const = 0;

   virtual void zoomTo(qreal optical, qreal digital) = 0;

   MULTI_CS_SIGNAL_1(Public, void maximumOpticalZoomChanged(qreal opticalZoom))
   MULTI_CS_SIGNAL_2(maximumOpticalZoomChanged, opticalZoom)

   MULTI_CS_SIGNAL_1(Public, void maximumDigitalZoomChanged(qreal digitalZoom))
   MULTI_CS_SIGNAL_2(maximumDigitalZoomChanged, digitalZoom)

   MULTI_CS_SIGNAL_1(Public, void requestedOpticalZoomChanged(qreal opticalZoom))
   MULTI_CS_SIGNAL_2(requestedOpticalZoomChanged, opticalZoom)

   MULTI_CS_SIGNAL_1(Public, void requestedDigitalZoomChanged(qreal digitalZoom))
   MULTI_CS_SIGNAL_2(requestedDigitalZoomChanged, digitalZoom)

   MULTI_CS_SIGNAL_1(Public, void currentOpticalZoomChanged(qreal opticalZoom))
   MULTI_CS_SIGNAL_2(currentOpticalZoomChanged, opticalZoom)

   MULTI_CS_SIGNAL_1(Public, void currentDigitalZoomChanged(qreal digitalZoom))
   MULTI_CS_SIGNAL_2(currentDigitalZoomChanged, digitalZoom)

 protected:
   explicit QCameraZoomControl(QObject *parent = nullptr);
};

#define QCameraZoomControl_iid "com.copperspice.CS.cameraZoomControl/1.0"
CS_DECLARE_INTERFACE(QCameraZoomControl, QCameraZoomControl_iid)

#endif
