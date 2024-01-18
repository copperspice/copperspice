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

#include <qtouchdevice.h>
#include <qtouchdevice_p.h>

#include <QCoreApplication>
#include <qdebug.h>
#include <QList>
#include <QMutex>

#include <qdebug_p.h>

QTouchDevice::QTouchDevice()
   : d(new QTouchDevicePrivate)
{
}

QTouchDevice::~QTouchDevice()
{
   delete d;
}

QTouchDevice::DeviceType QTouchDevice::type() const
{
   return d->type;
}


QTouchDevice::Capabilities QTouchDevice::capabilities() const
{
   return d->caps;
}

int QTouchDevice::maximumTouchPoints() const
{
   return d->maxTouchPoints;
}

QString QTouchDevice::name() const
{
   return d->name;
}

void QTouchDevice::setType(DeviceType devType)
{
   d->type = devType;
}

void QTouchDevice::setCapabilities(Capabilities caps)
{
   d->caps = caps;
}

void QTouchDevice::setMaximumTouchPoints(int max)
{
   d->maxTouchPoints = max;
}

void QTouchDevice::setName(const QString &name)
{
   d->name = name;
}

using TouchDevices = QList<const QTouchDevice *>;

static TouchDevices *deviceList()
{
   static TouchDevices retval;
   return &retval;
}

static QMutex devicesMutex;

static void cleanupDevicesList()
{
   QMutexLocker lock(&devicesMutex);

   for (auto item : *deviceList()) {
      delete item;
   }

   deviceList()->clear();
}


QList<const QTouchDevice *> QTouchDevice::devices()
{
   QMutexLocker lock(&devicesMutex);
   return *deviceList();
}

bool QTouchDevicePrivate::isRegistered(const QTouchDevice *dev)
{
   QMutexLocker locker(&devicesMutex);
   return deviceList()->contains(dev);
}

void QTouchDevicePrivate::registerDevice(const QTouchDevice *dev)
{
   QMutexLocker lock(&devicesMutex);
   if (deviceList()->isEmpty()) {
      qAddPostRoutine(cleanupDevicesList);
   }
   deviceList()->append(dev);
}

void QTouchDevicePrivate::unregisterDevice(const QTouchDevice *dev)
{
   QMutexLocker lock(&devicesMutex);
   bool wasRemoved = deviceList()->removeOne(dev);

   if (wasRemoved && deviceList()->isEmpty()) {
      qRemovePostRoutine(cleanupDevicesList);
   }
}

QDebug operator<<(QDebug debug, const QTouchDevice *device)
{
   QDebugStateSaver saver(debug);
   debug.nospace();
   debug.noquote();

   debug << "QTouchDevice(";

   if (device) {
      debug << '"' << device->name() << "\", type=";

      QtDebugUtils::formatQEnum(debug, device->type());
      debug << ", capabilities=";

      QtDebugUtils::formatQFlags(debug, device->capabilities());
      debug << ", maximumTouchPoints=" << device->maximumTouchPoints();

   } else {
      debug << '0';
   }

   debug << ')';

   return debug;
}
