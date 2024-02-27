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

#ifndef DSCAMERAIMAGEPROCESSINGCONTROL_H
#define DSCAMERAIMAGEPROCESSINGCONTROL_H

#include <qcamera.h>
#include <qcameraimageprocessingcontrol.h>

class DSCameraSession;

class DSCameraImageProcessingControl : public QCameraImageProcessingControl
{
   CS_OBJECT(DSCameraImageProcessingControl)

 public:
   DSCameraImageProcessingControl(DSCameraSession *session);
   virtual ~DSCameraImageProcessingControl();

   bool isParameterSupported(ProcessingParameter) const override;
   bool isParameterValueSupported(ProcessingParameter parameter, const QVariant &value) const override;
   QVariant parameter(ProcessingParameter parameter) const override;
   void setParameter(ProcessingParameter parameter, const QVariant &value) override;

 private:
   DSCameraSession *m_session;
};

#endif