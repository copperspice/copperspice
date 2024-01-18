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

#ifndef CAMERABINEXPOSURECONTROL_MAEMO_H
#define CAMERABINEXPOSURECONTROL_MAEMO_H

#include <qcamera.h>
#include <qcameraexposurecontrol.h>

#include <gst/gst.h>
#include <glib.h>

class CameraBinSession;

class Q_MULTIMEDIA_EXPORT CameraBinExposure : public QCameraExposureControl
{
   CS_OBJECT(CameraBinExposure)

 public:
   CameraBinExposure(CameraBinSession *session);
   virtual ~CameraBinExposure();

   bool isParameterSupported(ExposureParameter parameter) const;
   QVariantList supportedParameterRange(ExposureParameter parameter, bool *continuous) const;

   QVariant requestedValue(ExposureParameter parameter) const;
   QVariant actualValue(ExposureParameter parameter) const;
   bool setValue(ExposureParameter parameter, const QVariant &value);

 private:
   CameraBinSession *m_session;
   QHash<ExposureParameter, QVariant> m_requestedValues;
};

#endif
