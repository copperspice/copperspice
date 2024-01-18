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

#include <camera_zoom.h>
#include <camera_session.h>

#define ZOOM_PROPERTY "zoom"
#define MAX_ZOOM_PROPERTY "max-zoom"

CameraBinZoom::CameraBinZoom(CameraBinSession *session)
   : QCameraZoomControl(session), m_session(session), m_requestedOpticalZoom(1.0), m_requestedDigitalZoom(1.0)
{
}

CameraBinZoom::~CameraBinZoom()
{
}

qreal CameraBinZoom::maximumOpticalZoom() const
{
   return 1.0;
}

qreal CameraBinZoom::maximumDigitalZoom() const
{
   gfloat zoomFactor = 1.0;
   g_object_get(GST_BIN(m_session->cameraBin()), MAX_ZOOM_PROPERTY, &zoomFactor, NULL);
   return zoomFactor;
}

qreal CameraBinZoom::requestedDigitalZoom() const
{
   return m_requestedDigitalZoom;
}

qreal CameraBinZoom::requestedOpticalZoom() const
{
   return m_requestedOpticalZoom;
}

qreal CameraBinZoom::currentOpticalZoom() const
{
   return 1.0;
}

qreal CameraBinZoom::currentDigitalZoom() const
{
   gfloat zoomFactor = 1.0;
   g_object_get(GST_BIN(m_session->cameraBin()), ZOOM_PROPERTY, &zoomFactor, NULL);
   return zoomFactor;
}

void CameraBinZoom::zoomTo(qreal optical, qreal digital)
{
   qreal oldDigitalZoom = currentDigitalZoom();

   if (m_requestedDigitalZoom != digital) {
      m_requestedDigitalZoom = digital;
      emit requestedDigitalZoomChanged(digital);
   }

   if (m_requestedOpticalZoom != optical) {
      m_requestedOpticalZoom = optical;
      emit requestedOpticalZoomChanged(optical);
   }

   digital = qBound(qreal(1.0), digital, maximumDigitalZoom());
   g_object_set(GST_BIN(m_session->cameraBin()), ZOOM_PROPERTY, digital, NULL);

   qreal newDigitalZoom = currentDigitalZoom();
   if (!qFuzzyCompare(oldDigitalZoom, newDigitalZoom)) {
      emit currentDigitalZoomChanged(digital);
   }
}

