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

#ifndef QINPUTDEVICEMANAGER_P_H
#define QINPUTDEVICEMANAGER_P_H

#include <qobject.h>
#include <qpoint.h>

class QInputDeviceManagerPrivate;

class Q_GUI_EXPORT QInputDeviceManager : public QObject
{
   GUI_CS_OBJECT(QInputDeviceManager)
   Q_DECLARE_PRIVATE(QInputDeviceManager)

 public:
   enum DeviceType {
      DeviceTypeUnknown,
      DeviceTypePointer,
      DeviceTypeKeyboard,
      DeviceTypeTouch
   };

   QInputDeviceManager(QObject *parent = nullptr);

   int deviceCount(DeviceType type) const;

   void setCursorPos(const QPoint &pos);

   GUI_CS_SIGNAL_1(Public, void deviceListChanged(DeviceType type))
   GUI_CS_SIGNAL_2(deviceListChanged, type)

   GUI_CS_SIGNAL_1(Public, void cursorPositionChangeRequested(const QPoint &pos))
   GUI_CS_SIGNAL_2(cursorPositionChangeRequested, pos)

 protected:
   QScopedPointer<QInputDeviceManagerPrivate> d_ptr;
};

class Q_GUI_EXPORT QInputDeviceManagerPrivate
{
   Q_DECLARE_PUBLIC(QInputDeviceManager)

 public:
   static QInputDeviceManagerPrivate *get(QInputDeviceManager *mgr) {
      return mgr->d_func();
   }

   int deviceCount(QInputDeviceManager::DeviceType type) const;
   void setDeviceCount(QInputDeviceManager::DeviceType type, int count);

   QMap<QInputDeviceManager::DeviceType, int> m_deviceCount;

 protected:
   QInputDeviceManager *q_ptr;
};

#endif
