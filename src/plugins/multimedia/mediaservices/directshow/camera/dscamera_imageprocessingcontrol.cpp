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

#include <dscamera_imageprocessingcontrol.h>
#include <dscamera_session.h>

DSCameraImageProcessingControl::DSCameraImageProcessingControl(DSCameraSession *session)
   : QCameraImageProcessingControl(session), m_session(session)
{
}

DSCameraImageProcessingControl::~DSCameraImageProcessingControl()
{
}

bool DSCameraImageProcessingControl::isParameterSupported(
   QCameraImageProcessingControl::ProcessingParameter parameter) const
{
   return m_session->isImageProcessingParameterSupported(parameter);
}

bool DSCameraImageProcessingControl::isParameterValueSupported(
   QCameraImageProcessingControl::ProcessingParameter parameter,
   const QVariant &value) const
{
   return m_session->isImageProcessingParameterValueSupported(parameter, value);
}

QVariant DSCameraImageProcessingControl::parameter(
   QCameraImageProcessingControl::ProcessingParameter parameter) const
{
   return m_session->imageProcessingParameter(parameter);
}

void DSCameraImageProcessingControl::setParameter(QCameraImageProcessingControl::ProcessingParameter parameter,
      const QVariant &value)
{
   m_session->setImageProcessingParameter(parameter, value);
}

