/***********************************************************************
*
* Copyright (c) 2012-2022 Barbara Geller
* Copyright (c) 2012-2022 Ansel Sermersheim
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

#ifndef CAMERABINZOOMCONTROL_H
#define CAMERABINZOOMCONTROL_H

#include <qcamerazoomcontrol.h>

class CameraBinSession;

class CameraBinZoom  : public QCameraZoomControl
{
   CS_OBJECT(CameraBinZoom)

 public:
   CameraBinZoom(CameraBinSession *session);
   virtual ~CameraBinZoom();

   qreal maximumOpticalZoom() const;
   qreal maximumDigitalZoom() const;

   qreal requestedOpticalZoom() const;
   qreal requestedDigitalZoom() const;
   qreal currentOpticalZoom() const;
   qreal currentDigitalZoom() const;

   void zoomTo(qreal optical, qreal digital);

 private:
   CameraBinSession *m_session;
   qreal m_requestedOpticalZoom;
   qreal m_requestedDigitalZoom;
};

#endif // CAMERABINZOOMCONTROL_H
