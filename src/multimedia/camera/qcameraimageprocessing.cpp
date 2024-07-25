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

#include <qcameraimageprocessing.h>

#include <qcameracontrol.h>
#include <qcameraexposurecontrol.h>
#include <qcamerafocuscontrol.h>
#include <qcameraimagecapturecontrol.h>
#include <qcameraimageprocessingcontrol.h>
#include <qdebug.h>
#include <qmediarecordercontrol.h>
#include <qvideodeviceselectorcontrol.h>

#include <qmediaobject_p.h>

class QCameraImageProcessingFakeControl : public QCameraImageProcessingControl
{
 public:
   QCameraImageProcessingFakeControl(QObject *parent)
      : QCameraImageProcessingControl(parent)
   { }

   bool isParameterSupported(ProcessingParameter) const override {
      return false;
   }

   bool isParameterValueSupported(ProcessingParameter, const QVariant &) const override {
      return false;
   }

   QVariant parameter(ProcessingParameter) const override {
      return QVariant();
   }

   void setParameter(ProcessingParameter, const QVariant &) override {
   }
};

class QCameraImageProcessingPrivate : public QMediaObjectPrivate
{
   Q_DECLARE_NON_CONST_PUBLIC(QCameraImageProcessing)

 public:
   void initControls();

   QCameraImageProcessing *q_ptr;

   QCamera *camera;
   QCameraImageProcessingControl *imageControl;
   bool available;
};

void QCameraImageProcessingPrivate::initControls()
{
   imageControl = nullptr;

   QMediaService *service = camera->service();
   if (service) {
      imageControl = dynamic_cast<QCameraImageProcessingControl *>(service->requestControl(QCameraImageProcessingControl_iid));
   }

   available = (imageControl != nullptr);

   if (!imageControl) {
      imageControl = new QCameraImageProcessingFakeControl(q_ptr);
   }
}

QCameraImageProcessing::QCameraImageProcessing(QCamera *camera):
   QObject(camera), d_ptr(new QCameraImageProcessingPrivate)
{
   Q_D(QCameraImageProcessing);
   d->camera = camera;
   d->q_ptr = this;
   d->initControls();
}

QCameraImageProcessing::~QCameraImageProcessing()
{
   delete d_ptr;
}

bool QCameraImageProcessing::isAvailable() const
{
   return d_func()->available;
}

QCameraImageProcessing::WhiteBalanceMode QCameraImageProcessing::whiteBalanceMode() const
{
   return d_func()->imageControl->parameter(QCameraImageProcessingControl::WhiteBalancePreset)
          .value<QCameraImageProcessing::WhiteBalanceMode>();
}

void QCameraImageProcessing::setWhiteBalanceMode(QCameraImageProcessing::WhiteBalanceMode mode)
{
   d_func()->imageControl->setParameter(
      QCameraImageProcessingControl::WhiteBalancePreset,
      QVariant::fromValue<QCameraImageProcessing::WhiteBalanceMode>(mode));
}

bool QCameraImageProcessing::isWhiteBalanceModeSupported(QCameraImageProcessing::WhiteBalanceMode mode) const
{
   return d_func()->imageControl->isParameterValueSupported(
             QCameraImageProcessingControl::WhiteBalancePreset,
             QVariant::fromValue<QCameraImageProcessing::WhiteBalanceMode>(mode));

}

qreal QCameraImageProcessing::manualWhiteBalance() const
{
   return d_func()->imageControl->parameter(QCameraImageProcessingControl::ColorTemperature).toReal();
}

void QCameraImageProcessing::setManualWhiteBalance(qreal colorTemperature)
{
   d_func()->imageControl->setParameter(
      QCameraImageProcessingControl::ColorTemperature,
      QVariant(colorTemperature));
}

qreal QCameraImageProcessing::contrast() const
{
   return d_func()->imageControl->parameter(QCameraImageProcessingControl::ContrastAdjustment).toReal();
}

void QCameraImageProcessing::setContrast(qreal value)
{
   d_func()->imageControl->setParameter(QCameraImageProcessingControl::ContrastAdjustment, QVariant(value));
}

qreal QCameraImageProcessing::saturation() const
{
   return d_func()->imageControl->parameter(QCameraImageProcessingControl::SaturationAdjustment).toReal();
}

void QCameraImageProcessing::setSaturation(qreal value)
{
   d_func()->imageControl->setParameter(QCameraImageProcessingControl::SaturationAdjustment,
                                        QVariant(value));
}

qreal QCameraImageProcessing::sharpeningLevel() const
{
   return d_func()->imageControl->parameter(QCameraImageProcessingControl::SharpeningAdjustment).toReal();
}

void QCameraImageProcessing::setSharpeningLevel(qreal level)
{
   d_func()->imageControl->setParameter(QCameraImageProcessingControl::SharpeningAdjustment, QVariant(level));
}

qreal QCameraImageProcessing::denoisingLevel() const
{
   return d_func()->imageControl->parameter(QCameraImageProcessingControl::DenoisingAdjustment).toReal();
}

void QCameraImageProcessing::setDenoisingLevel(qreal level)
{
   d_func()->imageControl->setParameter(QCameraImageProcessingControl::DenoisingAdjustment, QVariant(level));
}

QCameraImageProcessing::ColorFilter QCameraImageProcessing::colorFilter() const
{
   return d_func()->imageControl->parameter(QCameraImageProcessingControl::ColorFilter)
          .value<QCameraImageProcessing::ColorFilter>();
}

void QCameraImageProcessing::setColorFilter(QCameraImageProcessing::ColorFilter filter)
{
   d_func()->imageControl->setParameter(
      QCameraImageProcessingControl::ColorFilter,
      QVariant::fromValue<QCameraImageProcessing::ColorFilter>(filter));
}

bool QCameraImageProcessing::isColorFilterSupported(QCameraImageProcessing::ColorFilter filter) const
{
   return d_func()->imageControl->isParameterValueSupported(
             QCameraImageProcessingControl::ColorFilter,
             QVariant::fromValue<QCameraImageProcessing::ColorFilter>(filter));

}

