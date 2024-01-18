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

#ifndef CAMERABINIMAGEPROCESSINGCONTROL_H
#define CAMERABINIMAGEPROCESSINGCONTROL_H

#include <qcamera.h>
#include <qcameraimageprocessingcontrol.h>

#include <gst/gst.h>
#include <glib.h>

#ifdef HAVE_GST_PHOTOGRAPHY
# include <gst/interfaces/photography.h>

# if !GST_CHECK_VERSION(1,0,0)
typedef GstWhiteBalanceMode GstPhotographyWhiteBalanceMode;
typedef GstColourToneMode GstPhotographyColorToneMode;
# endif

#endif

#ifdef USE_V4L
class CameraBinV4LImageProcessing;
#endif

class CameraBinSession;

class CameraBinImageProcessing : public QCameraImageProcessingControl
{
   CS_OBJECT(CameraBinImageProcessing)

 public:
   CameraBinImageProcessing(CameraBinSession *session);
   virtual ~CameraBinImageProcessing();

   QCameraImageProcessing::WhiteBalanceMode whiteBalanceMode() const;
   bool setWhiteBalanceMode(QCameraImageProcessing::WhiteBalanceMode mode);
   bool isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceMode mode) const;

   bool isParameterSupported(ProcessingParameter) const override;
   bool isParameterValueSupported(ProcessingParameter parameter, const QVariant &value) const override;
   QVariant parameter(ProcessingParameter parameter) const override;
   void setParameter(ProcessingParameter parameter, const QVariant &value) override;

#ifdef HAVE_GST_PHOTOGRAPHY
   void lockWhiteBalance();
   void unlockWhiteBalance();
#endif

 private:
   bool setColorBalanceValue(const QString &channel, qreal value);
   void updateColorBalanceValues();

   CameraBinSession *m_session;
   QMap<QCameraImageProcessingControl::ProcessingParameter, int> m_values;

#ifdef HAVE_GST_PHOTOGRAPHY
   QMap<GstPhotographyWhiteBalanceMode, QCameraImageProcessing::WhiteBalanceMode> m_mappedWbValues;
   QMap<QCameraImageProcessing::ColorFilter, GstPhotographyColorToneMode> m_filterMap;
#endif

   QCameraImageProcessing::WhiteBalanceMode m_whiteBalanceMode;

#ifdef USE_V4L
   CameraBinV4LImageProcessing *m_v4lImageControl;
#endif
};

#endif
