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

#include <qinputdevicemanager_p.h>

QInputDeviceManager::QInputDeviceManager(QObject *parent)
   : QObject(parent), d_ptr(new QInputDeviceManagerPrivate)
{
   d_ptr->q_ptr = this;
}

int QInputDeviceManager::deviceCount(DeviceType type) const
{
   Q_D(const QInputDeviceManager);
   return d->deviceCount(type);
}

int QInputDeviceManagerPrivate::deviceCount(QInputDeviceManager::DeviceType type) const
{
   return m_deviceCount.value(type);
}

void QInputDeviceManagerPrivate::setDeviceCount(QInputDeviceManager::DeviceType type, int count)
{
   Q_Q(QInputDeviceManager);
   if (m_deviceCount.value(type) != count) {
      m_deviceCount[type] = count;
      emit q->deviceListChanged(type);
   }
}

void QInputDeviceManager::setCursorPos(const QPoint &pos)
{
   emit cursorPositionChangeRequested(pos);
}

