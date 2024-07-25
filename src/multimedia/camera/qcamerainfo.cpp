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

#include <qcamerainfo.h>

#include <qcamerainfocontrol.h>
#include <qvideodeviceselectorcontrol.h>

#include <qcamera_p.h>
#include <qmediaserviceprovider_p.h>

class QCameraInfoPrivate
{
 public:
   QCameraInfoPrivate()
      : isNull(true), position(QCamera::UnspecifiedPosition), orientation(0)
   {
   }

   bool isNull;
   QString deviceName;
   QString description;
   QCamera::Position position;
   int orientation;
};

QCameraInfo::QCameraInfo(const QCamera &camera)
   : d(new QCameraInfoPrivate)
{
   const QVideoDeviceSelectorControl *deviceControl = camera.d_func()->deviceControl;
   if (deviceControl && deviceControl->deviceCount() > 0) {
      const int selectedDevice = deviceControl->selectedDevice();
      d->deviceName = deviceControl->deviceName(selectedDevice);
      d->description = deviceControl->deviceDescription(selectedDevice);
      d->isNull = false;
   }

   const QCameraInfoControl *infoControl = camera.d_func()->infoControl;
   if (infoControl) {
      d->position = infoControl->cameraPosition(d->deviceName);
      d->orientation = infoControl->cameraOrientation(d->deviceName);
      d->isNull = false;
   }
}

QCameraInfo::QCameraInfo(const QString &name)
   : d(new QCameraInfoPrivate)
{
   if (! name.empty()) {
      QMediaServiceProvider *provider = QMediaServiceProvider::defaultServiceProvider();
      const QByteArray service(Q_MEDIASERVICE_CAMERA);

      if (provider->devices(service).contains(name)) {
         d->deviceName  = name;
         d->description = provider->deviceDescription(service, name);
         d->position    = provider->cameraPosition(name);
         d->orientation = provider->cameraOrientation(name);
         d->isNull = false;
      }
   }
}

QCameraInfo::QCameraInfo(const QCameraInfo &other)
   : d(other.d)
{
}

QCameraInfo::~QCameraInfo()
{
}

bool QCameraInfo::operator==(const QCameraInfo &other) const
{
   if (d == other.d) {
      return true;
   }

   return (d->deviceName == other.d->deviceName
           && d->description == other.d->description
           && d->position == other.d->position
           && d->orientation == other.d->orientation);
}

bool QCameraInfo::isNull() const
{
   return d->isNull;
}

QString QCameraInfo::deviceName() const
{
   return d->deviceName;
}

QString QCameraInfo::description() const
{
   return d->description;
}

QCamera::Position QCameraInfo::position() const
{
   return d->position;
}

int QCameraInfo::orientation() const
{
   return d->orientation;
}

QCameraInfo QCameraInfo::defaultCamera()
{
   return QCameraInfo(QMediaServiceProvider::defaultServiceProvider()->defaultDevice(Q_MEDIASERVICE_CAMERA));
}

QList<QCameraInfo> QCameraInfo::availableCameras(QCamera::Position position)
{
   QList<QCameraInfo> cameras;

   const QMediaServiceProvider *provider = QMediaServiceProvider::defaultServiceProvider();
   const QByteArray service(Q_MEDIASERVICE_CAMERA);

   const QList<QString> devices = provider->devices(service);

   for (const auto &name : devices) {
      if (position == QCamera::UnspecifiedPosition || position == provider->cameraPosition(name)) {
         cameras.append(QCameraInfo(name));
      }
   }

   return cameras;
}

QCameraInfo &QCameraInfo::operator=(const QCameraInfo &other)
{
   d = other.d;
   return *this;
}

QDebug operator<<(QDebug d, const QCameraInfo &camera)
{
   auto &metaObj = QCamera::staticMetaObject();

   d.maybeSpace() << QString("QCameraInfo(deviceName = %1, position = %2, orientation = %3)")
                  .formatArg(camera.deviceName())
                  .formatArg(metaObj.enumerator(metaObj.indexOfEnumerator("Position")).valueToKey(camera.position()))
                  .formatArg(camera.orientation());

   return d.space();
}
