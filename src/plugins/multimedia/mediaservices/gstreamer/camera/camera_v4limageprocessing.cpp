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

#include <camera_v4limageprocessing.h>
#include <camera_session.h>
#include <qdebug.h>

#include <qcore_unix_p.h>

#include <linux/videodev2.h>

CameraBinV4LImageProcessing::CameraBinV4LImageProcessing(CameraBinSession *session)
   : QCameraImageProcessingControl(session), m_session(session)
{
}

CameraBinV4LImageProcessing::~CameraBinV4LImageProcessing()
{
}

bool CameraBinV4LImageProcessing::isParameterSupported(
   ProcessingParameter parameter) const
{
   return m_parametersInfo.contains(parameter);
}

bool CameraBinV4LImageProcessing::isParameterValueSupported(
   ProcessingParameter parameter, const QVariant &value) const
{
   QMap<ProcessingParameter, SourceParameterValueInfo>::const_iterator sourceValueInfo =
      m_parametersInfo.constFind(parameter);
   if (sourceValueInfo == m_parametersInfo.constEnd()) {
      return false;
   }

   switch (parameter) {

      case QCameraImageProcessingControl::WhiteBalancePreset: {
         const QCameraImageProcessing::WhiteBalanceMode checkedValue =
            value.value<QCameraImageProcessing::WhiteBalanceMode>();
         const QCameraImageProcessing::WhiteBalanceMode firstAllowedValue =
            (*sourceValueInfo).minimumValue ? QCameraImageProcessing::WhiteBalanceAuto
            : QCameraImageProcessing::WhiteBalanceManual;
         const QCameraImageProcessing::WhiteBalanceMode secondAllowedValue =
            (*sourceValueInfo).maximumValue ? QCameraImageProcessing::WhiteBalanceAuto
            : QCameraImageProcessing::WhiteBalanceManual;
         if (checkedValue != firstAllowedValue
               && checkedValue != secondAllowedValue) {
            return false;
         }
      }
      break;

      case QCameraImageProcessingControl::ColorTemperature: {
         const qint32 checkedValue = value.toInt();
         if (checkedValue < (*sourceValueInfo).minimumValue
               || checkedValue > (*sourceValueInfo).maximumValue) {
            return false;
         }
      }
      break;

      case QCameraImageProcessingControl::ContrastAdjustment: // falling back
      case QCameraImageProcessingControl::SaturationAdjustment: // falling back
      case QCameraImageProcessingControl::BrightnessAdjustment: // falling back
      case QCameraImageProcessingControl::SharpeningAdjustment: {
         const qint32 sourceValue = sourceImageProcessingParameterValue(
                                       value.toReal(), (*sourceValueInfo));
         if (sourceValue < (*sourceValueInfo).minimumValue
               || sourceValue > (*sourceValueInfo).maximumValue) {
            return false;
         }
      }
      break;

      default:
         return false;
   }

   return true;
}

QVariant CameraBinV4LImageProcessing::parameter(
   ProcessingParameter parameter) const
{
   QMap<ProcessingParameter, SourceParameterValueInfo>::const_iterator sourceValueInfo =
      m_parametersInfo.constFind(parameter);
   if (sourceValueInfo == m_parametersInfo.constEnd()) {
      qWarning() << "Unable to get the parameter value: the parameter is not supported.";
      return QVariant();
   }

   const QString deviceName = m_session->device();
   const int fd = qt_safe_open(deviceName.toLocal8Bit().constData(), O_RDONLY);
   if (fd == -1) {
      qWarning() << "Unable to open the camera" << deviceName
                 << "for read to get the parameter value:" << qt_error_string(errno);
      return QVariant();
   }

   struct v4l2_control control;
   ::memset(&control, 0, sizeof(control));
   control.id = (*sourceValueInfo).cid;

   const bool ret = (::ioctl(fd, VIDIOC_G_CTRL, &control) == 0);

   qt_safe_close(fd);

   if (!ret) {
      qWarning() << "Unable to get the parameter value:" << qt_error_string(errno);
      return QVariant();
   }

   switch (parameter) {

      case QCameraImageProcessingControl::WhiteBalancePreset:
         return QVariant::fromValue<QCameraImageProcessing::WhiteBalanceMode>(
                   control.value ? QCameraImageProcessing::WhiteBalanceAuto
                   : QCameraImageProcessing::WhiteBalanceManual);

      case QCameraImageProcessingControl::ColorTemperature:
         return QVariant::fromValue<qint32>(control.value);

      case QCameraImageProcessingControl::ContrastAdjustment: // falling back
      case QCameraImageProcessingControl::SaturationAdjustment: // falling back
      case QCameraImageProcessingControl::BrightnessAdjustment: // falling back
      case QCameraImageProcessingControl::SharpeningAdjustment: {
         return scaledImageProcessingParameterValue(
                   control.value, (*sourceValueInfo));
      }

      default:
         return QVariant();
   }
}

void CameraBinV4LImageProcessing::setParameter(
   ProcessingParameter parameter, const QVariant &value)
{
   QMap<ProcessingParameter, SourceParameterValueInfo>::const_iterator sourceValueInfo =
      m_parametersInfo.constFind(parameter);
   if (sourceValueInfo == m_parametersInfo.constEnd()) {
      qWarning() << "Unable to set the parameter value: the parameter is not supported.";
      return;
   }

   const QString deviceName = m_session->device();
   const int fd = qt_safe_open(deviceName.toLocal8Bit().constData(), O_WRONLY);
   if (fd == -1) {
      qWarning() << "Unable to open the camera" << deviceName
                 << "for write to set the parameter value:" << qt_error_string(errno);
      return;
   }

   struct v4l2_control control;
   ::memset(&control, 0, sizeof(control));
   control.id = (*sourceValueInfo).cid;

   switch (parameter) {

      case QCameraImageProcessingControl::WhiteBalancePreset: {
         const QCameraImageProcessing::WhiteBalanceMode m =
            value.value<QCameraImageProcessing::WhiteBalanceMode>();
         if (m != QCameraImageProcessing::WhiteBalanceAuto
               && m != QCameraImageProcessing::WhiteBalanceManual) {
            qt_safe_close(fd);
            return;
         }

         control.value = (m == QCameraImageProcessing::WhiteBalanceAuto) ? true : false;
      }
      break;

      case QCameraImageProcessingControl::ColorTemperature:
         control.value = value.toInt();
         break;

      case QCameraImageProcessingControl::ContrastAdjustment: // falling back
      case QCameraImageProcessingControl::SaturationAdjustment: // falling back
      case QCameraImageProcessingControl::BrightnessAdjustment: // falling back
      case QCameraImageProcessingControl::SharpeningAdjustment:
         control.value = sourceImageProcessingParameterValue(
                            value.toReal(), (*sourceValueInfo));
         break;

      default:
         qt_safe_close(fd);
         return;
   }

   if (::ioctl(fd, VIDIOC_S_CTRL, &control) != 0) {
      qWarning() << "Unable to set the parameter value:" << qt_error_string(errno);
   }

   qt_safe_close(fd);
}

void CameraBinV4LImageProcessing::updateParametersInfo(
   QCamera::Status cameraStatus)
{
   if (cameraStatus == QCamera::UnloadedStatus) {
      m_parametersInfo.clear();
   } else if (cameraStatus == QCamera::LoadedStatus) {
      const QString deviceName = m_session->device();
      const int fd = qt_safe_open(deviceName.toLocal8Bit().constData(), O_RDONLY);
      if (fd == -1) {
         qWarning() << "Unable to open the camera" << deviceName
                    << "for read to query the parameter info:" << qt_error_string(errno);
         return;
      }

      static const struct SupportedParameterEntry {
         quint32 cid;
         QCameraImageProcessingControl::ProcessingParameter parameter;
      } supportedParametersEntries[] = {
         { V4L2_CID_AUTO_WHITE_BALANCE, QCameraImageProcessingControl::WhiteBalancePreset },
         { V4L2_CID_WHITE_BALANCE_TEMPERATURE, QCameraImageProcessingControl::ColorTemperature },
         { V4L2_CID_CONTRAST, QCameraImageProcessingControl::ContrastAdjustment },
         { V4L2_CID_SATURATION, QCameraImageProcessingControl::SaturationAdjustment },
         { V4L2_CID_BRIGHTNESS, QCameraImageProcessingControl::BrightnessAdjustment },
         { V4L2_CID_SHARPNESS, QCameraImageProcessingControl::SharpeningAdjustment }
      };

      for (int i = 0; i < int(sizeof(supportedParametersEntries) / sizeof(SupportedParameterEntry)); ++i) {
         struct v4l2_queryctrl queryControl;
         ::memset(&queryControl, 0, sizeof(queryControl));
         queryControl.id = supportedParametersEntries[i].cid;

         if (::ioctl(fd, VIDIOC_QUERYCTRL, &queryControl) != 0) {
            qWarning() << "Unable to query the parameter info:" << qt_error_string(errno);
            continue;
         }

         SourceParameterValueInfo sourceValueInfo;
         sourceValueInfo.cid = queryControl.id;
         sourceValueInfo.defaultValue = queryControl.default_value;
         sourceValueInfo.maximumValue = queryControl.maximum;
         sourceValueInfo.minimumValue = queryControl.minimum;

         m_parametersInfo.insert(supportedParametersEntries[i].parameter, sourceValueInfo);
      }

      qt_safe_close(fd);
   }
}

qreal CameraBinV4LImageProcessing::scaledImageProcessingParameterValue(
   qint32 sourceValue, const SourceParameterValueInfo &sourceValueInfo)
{
   if (sourceValue == sourceValueInfo.defaultValue) {
      return 0.0f;
   } else if (sourceValue < sourceValueInfo.defaultValue) {
      return ((sourceValue - sourceValueInfo.minimumValue)
              / qreal(sourceValueInfo.defaultValue - sourceValueInfo.minimumValue))
             + (-1.0f);
   } else {
      return ((sourceValue - sourceValueInfo.defaultValue)
              / qreal(sourceValueInfo.maximumValue - sourceValueInfo.defaultValue));
   }
}

qint32 CameraBinV4LImageProcessing::sourceImageProcessingParameterValue(
   qreal scaledValue, const SourceParameterValueInfo &valueRange)
{
   if (qFuzzyIsNull(scaledValue)) {
      return valueRange.defaultValue;
   } else if (scaledValue < 0.0f) {
      return ((scaledValue - (-1.0f)) * (valueRange.defaultValue - valueRange.minimumValue))
             + valueRange.minimumValue;
   } else {
      return (scaledValue * (valueRange.maximumValue - valueRange.defaultValue))
             + valueRange.defaultValue;
   }
}
