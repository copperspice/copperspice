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

#ifndef QSCREEN_P_H
#define QSCREEN_P_H

#include <qscreen.h>
#include <qplatform_screen.h>

#include <qhighdpiscaling_p.h>

class QScreenPrivate
{
   Q_DECLARE_PUBLIC(QScreen)

 public:
   QScreenPrivate()
      : platformScreen(nullptr), orientationUpdateMask(Qt::EmptyFlag)
   {
   }

   void setPlatformScreen(QPlatformScreen *screen);

   void updateHighDpi() {
      geometry          = platformScreen->deviceIndependentGeometry();
      availableGeometry = QHighDpi::fromNative(platformScreen->availableGeometry(),
               QHighDpiScaling::factor(platformScreen), geometry.topLeft());
   }

   void updatePrimaryOrientation();

   QPlatformScreen *platformScreen;

   Qt::ScreenOrientations orientationUpdateMask;
   Qt::ScreenOrientation  orientation;
   Qt::ScreenOrientation  filteredOrientation;
   Qt::ScreenOrientation  primaryOrientation;

   QRect geometry;
   QRect availableGeometry;
   QDpi logicalDpi;
   qreal refreshRate;

 protected:
   QScreen *q_ptr;

};

#endif
