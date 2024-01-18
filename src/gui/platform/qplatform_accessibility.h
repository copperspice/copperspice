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

#ifndef QPLATFORM_ACCESSIBILITY_H
#define QPLATFORM_ACCESSIBILITY_H

#include <qobject.h>

#ifndef QT_NO_ACCESSIBILITY

#include <qaccessible.h>

class Q_GUI_EXPORT QPlatformAccessibility
{
 public:
   QPlatformAccessibility();

   virtual ~QPlatformAccessibility();
   virtual void notifyAccessibilityUpdate(QAccessibleEvent *event);
   virtual void setRootObject(QObject *o);
   virtual void initialize();
   virtual void cleanup();

   inline bool isActive() const {
      return m_active;
   }
   void setActive(bool active);

 private:
   bool m_active;
};

#endif // QT_NO_ACCESSIBILITY

#endif
