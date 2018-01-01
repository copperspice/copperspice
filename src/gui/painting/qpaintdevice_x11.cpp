/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qpaintdevice.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qbitmap.h>
#include <qapplication.h>
#include <qt_x11_p.h>
#include <qx11info_x11.h>

QT_BEGIN_NAMESPACE


Drawable Q_GUI_EXPORT qt_x11Handle(const QPaintDevice *pd)
{
   if (!pd) {
      return 0;
   }

   if (pd->devType() == QInternal::Widget) {
      return static_cast<const QWidget *>(pd)->handle();
   } else if (pd->devType() == QInternal::Pixmap) {
      return static_cast<const QPixmap *>(pd)->handle();
   }

   return 0;
}

/*!
    \relates QPaintDevice

    Returns the QX11Info structure for the \a pd paint device. 0 is
    returned if it can't be obtained.
*/
const Q_GUI_EXPORT QX11Info *qt_x11Info(const QPaintDevice *pd)
{
   if (!pd) {
      return 0;
   }
   if (pd->devType() == QInternal::Widget) {
      return &static_cast<const QWidget *>(pd)->x11Info();
   } else if (pd->devType() == QInternal::Pixmap) {
      return &static_cast<const QPixmap *>(pd)->x11Info();
   }
   return 0;
}

QT_END_NAMESPACE
