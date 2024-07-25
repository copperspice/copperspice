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

#include <qcameraexposure.h>

#include <qcamera.h>
#include <qcameraexposurecontrol.h>
#include <qcameraflashcontrol.h>
#include <qdebug.h>
#include <qmetaobject.h>

#include <qmediaobject_p.h>

class QCameraExposurePrivate
{
 public:
   void initControls();
   QCameraExposure *q_ptr;

   template <typename T>
   T actualExposureParameter(QCameraExposureControl::ExposureParameter parameter, const T &defaultValue) const;

   template <typename T>
   T requestedExposureParameter(QCameraExposureControl::ExposureParameter parameter, const T &defaultValue) const;

   template <typename T>
   void setExposureParameter(QCameraExposureControl::ExposureParameter parameter, const T &value);

   void resetExposureParameter(QCameraExposureControl::ExposureParameter parameter);

   QCamera *camera;
   QCameraExposureControl *exposureControl;
   QCameraFlashControl *flashControl;

   void _q_exposureParameterChanged(int parameter);
   void _q_exposureParameterRangeChanged(int parameter);

 private:
   Q_DECLARE_NON_CONST_PUBLIC(QCameraExposure)
};

void QCameraExposurePrivate::initControls()
{
   Q_Q(QCameraExposure);

   QMediaService *service = camera->service();
   exposureControl = nullptr;
   flashControl    = nullptr;

   if (service) {
      exposureControl = dynamic_cast<QCameraExposureControl *>(service->requestControl(QCameraExposureControl_iid));
      flashControl    = dynamic_cast<QCameraFlashControl *>(service->requestControl(QCameraFlashControl_iid));
   }

   if (exposureControl) {
      q->connect(exposureControl, &QCameraExposureControl::actualValueChanged,    q, &QCameraExposure::_q_exposureParameterChanged);
      q->connect(exposureControl, &QCameraExposureControl::parameterRangeChanged, q, &QCameraExposure::_q_exposureParameterRangeChanged);
   }

   if (flashControl) {
      q->connect(flashControl, &QCameraFlashControl::flashReady, q, &QCameraExposure::flashReady);
   }
}

template <typename T>
T QCameraExposurePrivate::actualExposureParameter(QCameraExposureControl::ExposureParameter parameter,
      const T &defaultValue) const
{
   QVariant value = exposureControl ? exposureControl->actualValue(parameter) : QVariant();

   return value.isValid() ? value.value<T>() : defaultValue;
}

template <typename T>
T QCameraExposurePrivate::requestedExposureParameter(QCameraExposureControl::ExposureParameter parameter,
      const T &defaultValue) const
{
   QVariant value = exposureControl ? exposureControl->requestedValue(parameter) : QVariant();

   return value.isValid() ? value.value<T>() : defaultValue;
}

template <typename T>
void QCameraExposurePrivate::setExposureParameter(QCameraExposureControl::ExposureParameter parameter, const T &value)
{
   if (exposureControl) {
      exposureControl->setValue(parameter, QVariant::fromValue<T>(value));
   }
}

void QCameraExposurePrivate::resetExposureParameter(QCameraExposureControl::ExposureParameter parameter)
{
   if (exposureControl) {
      exposureControl->setValue(parameter, QVariant());
   }
}

void QCameraExposurePrivate::_q_exposureParameterChanged(int parameter)
{
   Q_Q(QCameraExposure);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "Exposure parameter changed:"
         << QCameraExposureControl::ExposureParameter(parameter)
         << exposureControl->actualValue(QCameraExposureControl::ExposureParameter(parameter));
#endif

   switch (parameter) {
      case QCameraExposureControl::ISO:
         emit q->isoSensitivityChanged(q->isoSensitivity());
         break;

      case QCameraExposureControl::Aperture:
         emit q->apertureChanged(q->aperture());
         break;

      case QCameraExposureControl::ShutterSpeed:
         emit q->shutterSpeedChanged(q->shutterSpeed());
         break;

      case QCameraExposureControl::ExposureCompensation:
         emit q->exposureCompensationChanged(q->exposureCompensation());
         break;
   }
}

void QCameraExposurePrivate::_q_exposureParameterRangeChanged(int parameter)
{
   Q_Q(QCameraExposure);

   switch (parameter) {
      case QCameraExposureControl::Aperture:
         emit q->apertureRangeChanged();
         break;

      case QCameraExposureControl::ShutterSpeed:
         emit q->shutterSpeedRangeChanged();
         break;
   }
}

QCameraExposure::QCameraExposure(QCamera *parent):
   QObject(parent), d_ptr(new QCameraExposurePrivate)
{
   Q_D(QCameraExposure);
   d->camera = parent;
   d->q_ptr = this;
   d->initControls();
}

QCameraExposure::~QCameraExposure()
{
   Q_D(QCameraExposure);

   if (d->exposureControl) {
      d->camera->service()->releaseControl(d->exposureControl);
   }

   delete d;
}

bool QCameraExposure::isAvailable() const
{
   return d_func()->exposureControl != nullptr;
}

QCameraExposure::FlashModes QCameraExposure::flashMode() const
{
   return d_func()->flashControl ? d_func()->flashControl->flashMode() : QCameraExposure::FlashOff;
}

void QCameraExposure::setFlashMode(QCameraExposure::FlashModes mode)
{
   if (d_func()->flashControl) {
      d_func()->flashControl->setFlashMode(mode);
   }
}

bool QCameraExposure::isFlashModeSupported(QCameraExposure::FlashModes mode) const
{
   return d_func()->flashControl ? d_func()->flashControl->isFlashModeSupported(mode) : false;
}

bool QCameraExposure::isFlashReady() const
{
   return d_func()->flashControl ? d_func()->flashControl->isFlashReady() : false;
}

QCameraExposure::ExposureMode QCameraExposure::exposureMode() const
{
   return d_func()->actualExposureParameter<QCameraExposure::ExposureMode>(QCameraExposureControl::ExposureMode, QCameraExposure::ExposureAuto);
}

void QCameraExposure::setExposureMode(QCameraExposure::ExposureMode mode)
{
   d_func()->setExposureParameter<QCameraExposure::ExposureMode>(QCameraExposureControl::ExposureMode, mode);
}

bool QCameraExposure::isExposureModeSupported(QCameraExposure::ExposureMode mode) const
{
   if (! d_func()->exposureControl) {
      return false;
   }

   bool continuous = false;

   return d_func()->exposureControl->supportedParameterRange(QCameraExposureControl::ExposureMode, &continuous)
          .contains(QVariant::fromValue<QCameraExposure::ExposureMode>(mode));
}

double QCameraExposure::exposureCompensation() const
{
   return d_func()->actualExposureParameter<double>(QCameraExposureControl::ExposureCompensation, 0.0);
}

void QCameraExposure::setExposureCompensation(double value)
{
   d_func()->setExposureParameter<double>(QCameraExposureControl::ExposureCompensation, value);
}

QCameraExposure::MeteringMode QCameraExposure::meteringMode() const
{
   return d_func()->actualExposureParameter<QCameraExposure::MeteringMode>(QCameraExposureControl::MeteringMode, QCameraExposure::MeteringMatrix);
}

void QCameraExposure::setMeteringMode(QCameraExposure::MeteringMode mode)
{
   d_func()->setExposureParameter<QCameraExposure::MeteringMode>(QCameraExposureControl::MeteringMode, mode);
}

QPointF QCameraExposure::spotMeteringPoint() const
{
   return d_func()->exposureControl ? d_func()->exposureControl->actualValue(QCameraExposureControl::SpotMeteringPoint).toPointF() : QPointF();
}

void QCameraExposure::setSpotMeteringPoint(const QPointF &point)
{
   if (d_func()->exposureControl) {
      d_func()->exposureControl->setValue(QCameraExposureControl::SpotMeteringPoint, point);
   }
}

bool QCameraExposure::isMeteringModeSupported(QCameraExposure::MeteringMode mode) const
{
   if (!d_func()->exposureControl) {
      return false;
   }

   bool continuous = false;
   return d_func()->exposureControl->supportedParameterRange(QCameraExposureControl::MeteringMode, &continuous)
          .contains(QVariant::fromValue<QCameraExposure::MeteringMode>(mode));
}

int QCameraExposure::isoSensitivity() const
{
   return d_func()->actualExposureParameter<int>(QCameraExposureControl::ISO, -1);
}

int QCameraExposure::requestedIsoSensitivity() const
{
   return d_func()->requestedExposureParameter<int>(QCameraExposureControl::ISO, -1);
}

QList<int> QCameraExposure::supportedIsoSensitivities(bool *continuous) const
{
   QList<int> res;
   QCameraExposureControl *control = d_func()->exposureControl;

   bool tmp = false;
   if (!continuous) {
      continuous = &tmp;
   }

   if (!control) {
      return res;
   }

   for (const QVariant &value : control->supportedParameterRange(QCameraExposureControl::ISO, continuous)) {
      bool ok = false;
      int intValue = value.toInt(&ok);

      if (ok) {
         res.append(intValue);
      } else {
         qWarning() << "Incompatible ISO value type, int is expected";
      }
   }

   return res;
}

void QCameraExposure::setManualIsoSensitivity(int iso)
{
   d_func()->setExposureParameter<int>(QCameraExposureControl::ISO, iso);
}

void QCameraExposure::setAutoIsoSensitivity()
{
   d_func()->resetExposureParameter(QCameraExposureControl::ISO);
}

double QCameraExposure::aperture() const
{
   return d_func()->actualExposureParameter<double>(QCameraExposureControl::Aperture, -1.0);
}

double QCameraExposure::requestedAperture() const
{
   return d_func()->requestedExposureParameter<double>(QCameraExposureControl::Aperture, -1.0);
}

QList<double> QCameraExposure::supportedApertures(bool *continuous) const
{
   QList<double> res;
   QCameraExposureControl *control = d_func()->exposureControl;

   bool tmp = false;
   if (!continuous) {
      continuous = &tmp;
   }

   if (!control) {
      return res;
   }

   for (const QVariant &value : control->supportedParameterRange(QCameraExposureControl::Aperture, continuous)) {
      bool ok = false;
      double realValue = value.toReal(&ok);

      if (ok) {
         res.append(realValue);
      } else {
         qWarning() << "Incompatible aperture value type, double is expected";
      }
   }

   return res;
}

void QCameraExposure::setManualAperture(double aperture)
{
   d_func()->setExposureParameter<double>(QCameraExposureControl::Aperture, aperture);
}

void QCameraExposure::setAutoAperture()
{
   d_func()->resetExposureParameter(QCameraExposureControl::Aperture);
}

double QCameraExposure::shutterSpeed() const
{
   return d_func()->actualExposureParameter<double>(QCameraExposureControl::ShutterSpeed, -1.0);
}

double QCameraExposure::requestedShutterSpeed() const
{
   return d_func()->requestedExposureParameter<double>(QCameraExposureControl::ShutterSpeed, -1.0);
}

QList<double> QCameraExposure::supportedShutterSpeeds(bool *continuous) const
{
   QList<double> res;
   QCameraExposureControl *control = d_func()->exposureControl;

   bool tmp = false;
   if (!continuous) {
      continuous = &tmp;
   }

   if (!control) {
      return res;
   }

   for (const QVariant &value : control->supportedParameterRange(QCameraExposureControl::ShutterSpeed, continuous)) {
      bool ok = false;
      double realValue = value.toReal(&ok);

      if (ok) {
         res.append(realValue);
      } else {
         qWarning() << "Incompatible shutter speed value type, double is expected";
      }
   }

   return res;
}

void QCameraExposure::setManualShutterSpeed(double seconds)
{
   d_func()->setExposureParameter<double>(QCameraExposureControl::ShutterSpeed, seconds);
}

void QCameraExposure::setAutoShutterSpeed()
{
   d_func()->resetExposureParameter(QCameraExposureControl::ShutterSpeed);
}

void QCameraExposure::_q_exposureParameterChanged(int value)
{
   Q_D(QCameraExposure);
   d->_q_exposureParameterChanged(value);
}

void QCameraExposure::_q_exposureParameterRangeChanged(int value)
{
   Q_D(QCameraExposure);
   d->_q_exposureParameterRangeChanged(value);
}