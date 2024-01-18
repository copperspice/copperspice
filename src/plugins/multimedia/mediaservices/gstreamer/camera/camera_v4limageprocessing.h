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

#ifndef CAMERA_V4LIMAGEPROCESSING_H
#define CAMERA_V4LIMAGEPROCESSING_H

#include <qcamera.h>
#include <qcameraimageprocessingcontrol.h>

class CameraBinSession;

class CameraBinV4LImageProcessing : public QCameraImageProcessingControl
{
   CS_OBJECT(CameraBinV4LImageProcessing)

 public:
   CameraBinV4LImageProcessing(CameraBinSession *session);
   virtual ~CameraBinV4LImageProcessing();

   bool isParameterSupported(ProcessingParameter) const;
   bool isParameterValueSupported(ProcessingParameter parameter, const QVariant &value) const;
   QVariant parameter(ProcessingParameter parameter) const;
   void setParameter(ProcessingParameter parameter, const QVariant &value);

 public :
   CS_SLOT_1(Public, void updateParametersInfo(QCamera::Status cameraStatus))
   CS_SLOT_2(updateParametersInfo)

 private:
   struct SourceParameterValueInfo {
      SourceParameterValueInfo()
         : cid(0) {
      }

      qint32 defaultValue;
      qint32 minimumValue;
      qint32 maximumValue;
      quint32 cid; // V4L control id
   };

   static qreal scaledImageProcessingParameterValue(
      qint32 sourceValue, const SourceParameterValueInfo &sourceValueInfo);
   static qint32 sourceImageProcessingParameterValue(
      qreal scaledValue, const SourceParameterValueInfo &valueRange);
 private:
   CameraBinSession *m_session;
   QMap<ProcessingParameter, SourceParameterValueInfo> m_parametersInfo;
};

#endif
