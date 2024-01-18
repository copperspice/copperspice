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

   qreal maximumOpticalZoom() const override;
   qreal maximumDigitalZoom() const override;

   qreal requestedOpticalZoom() const override;
   qreal requestedDigitalZoom() const override;
   qreal currentOpticalZoom() const override;
   qreal currentDigitalZoom() const override;

   void zoomTo(qreal optical, qreal digital) override;

 private:
   CameraBinSession *m_session;
   qreal m_requestedOpticalZoom;
   qreal m_requestedDigitalZoom;
};

#endif
