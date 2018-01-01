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

#ifndef QWINDOWSURFACE_MAC_P_H
#define QWINDOWSURFACE_MAC_P_H

#include <qglobal.h>
#include <qwindowsurface_p.h>

QT_BEGIN_NAMESPACE

class QPaintDevice;
class QPoint;
class QRegion;
class QWidget;
struct QMacWindowSurfacePrivate;

class QMacWindowSurface : public QWindowSurface
{
 public:
   QMacWindowSurface(QWidget *widget);
   ~QMacWindowSurface();

   QPaintDevice *paintDevice();
   void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
   void setGeometry(const QRect &rect);
   bool scroll(const QRegion &area, int dx, int dy);

 private:
   QMacWindowSurfacePrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QWINDOWSURFACE_MAC_P_H
