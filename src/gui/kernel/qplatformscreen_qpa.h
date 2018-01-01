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

#ifndef QPLATFORMSCREEN_QPA_H
#define QPLATFORMSCREEN_QPA_H

#include <QtCore/qmetatype.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qvariant.h>
#include <QtCore/qrect.h>
#include <QtCore/qobject.h>
#include <QtGui/qcursor.h>
#include <QtGui/qimage.h>
#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QPlatformScreen : public QObject
{
   GUI_CS_OBJECT(QPlatformScreen)

 public:
   virtual ~QPlatformScreen() { }

   virtual QRect geometry() const = 0;
   virtual QRect availableGeometry() const {
      return geometry();
   }
   virtual int depth() const = 0;
   virtual QImage::Format format() const = 0;
   virtual QSize physicalSize() const;

   //jl: should setDirty be removed.
   virtual void setDirty(const QRect &) {}
   virtual QWidget *topLevelAt(const QPoint &point) const;

   //jl: should this function be in QPlatformIntegration
   //jl: maybe screenForWidget is a better name?
   static QPlatformScreen *platformScreenForWidget(const QWidget *widget);
};

QT_END_NAMESPACE

#endif // QPLATFORMSCREEN_H
