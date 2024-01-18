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

#ifndef QSCREEN_H
#define QSCREEN_H

#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qrect.h>
#include <qsize.h>
#include <qsizef.h>
#include <qtransform.h>

#include <qscopedpointer.h>

class QPlatformScreen;
class QScreenPrivate;
class QWindow;
class QRect;
class QPixmap;
class QDebug;

class Q_GUI_EXPORT QScreen : public QObject
{
   GUI_CS_OBJECT(QScreen)

   GUI_CS_PROPERTY_READ(name, name)
   GUI_CS_PROPERTY_CONSTANT(name)

   GUI_CS_PROPERTY_READ(depth, depth)
   GUI_CS_PROPERTY_CONSTANT(depth)

   GUI_CS_PROPERTY_READ(size, size)
   GUI_CS_PROPERTY_NOTIFY(size, geometryChanged)

   GUI_CS_PROPERTY_READ(availableSize, availableSize)
   GUI_CS_PROPERTY_NOTIFY(availableSize, availableGeometryChanged)

   GUI_CS_PROPERTY_READ(virtualSize, virtualSize)
   GUI_CS_PROPERTY_NOTIFY(virtualSize, virtualGeometryChanged)

   GUI_CS_PROPERTY_READ(availableVirtualSize, availableVirtualSize)
   GUI_CS_PROPERTY_NOTIFY(availableVirtualSize, virtualGeometryChanged)

   GUI_CS_PROPERTY_READ(geometry, geometry)
   GUI_CS_PROPERTY_NOTIFY(geometry, geometryChanged)

   GUI_CS_PROPERTY_READ(availableGeometry, availableGeometry)
   GUI_CS_PROPERTY_NOTIFY(availableGeometry, availableGeometryChanged)

   GUI_CS_PROPERTY_READ(virtualGeometry, virtualGeometry)
   GUI_CS_PROPERTY_NOTIFY(virtualGeometry, virtualGeometryChanged)

   GUI_CS_PROPERTY_READ(availableVirtualGeometry, availableVirtualGeometry)
   GUI_CS_PROPERTY_NOTIFY(availableVirtualGeometry, virtualGeometryChanged)

   GUI_CS_PROPERTY_READ(physicalSize, physicalSize)
   GUI_CS_PROPERTY_NOTIFY(physicalSize, physicalSizeChanged)

   GUI_CS_PROPERTY_READ(physicalDotsPerInchX, physicalDotsPerInchX)
   GUI_CS_PROPERTY_NOTIFY(physicalDotsPerInchX, physicalDotsPerInchChanged)

   GUI_CS_PROPERTY_READ(physicalDotsPerInchY, physicalDotsPerInchY)
   GUI_CS_PROPERTY_NOTIFY(physicalDotsPerInchY, physicalDotsPerInchChanged)

   GUI_CS_PROPERTY_READ(physicalDotsPerInch, physicalDotsPerInch)
   GUI_CS_PROPERTY_NOTIFY(physicalDotsPerInch, physicalDotsPerInchChanged)

   GUI_CS_PROPERTY_READ(logicalDotsPerInchX, logicalDotsPerInchX)
   GUI_CS_PROPERTY_NOTIFY(logicalDotsPerInchX, logicalDotsPerInchChanged)

   GUI_CS_PROPERTY_READ(logicalDotsPerInchY, logicalDotsPerInchY)
   GUI_CS_PROPERTY_NOTIFY(logicalDotsPerInchY, logicalDotsPerInchChanged)

   GUI_CS_PROPERTY_READ(logicalDotsPerInch, logicalDotsPerInch)
   GUI_CS_PROPERTY_NOTIFY(logicalDotsPerInch, logicalDotsPerInchChanged)

   GUI_CS_PROPERTY_READ(devicePixelRatio, devicePixelRatio)
   GUI_CS_PROPERTY_NOTIFY(devicePixelRatio, physicalDotsPerInchChanged)

   GUI_CS_PROPERTY_READ(primaryOrientation, primaryOrientation)
   GUI_CS_PROPERTY_NOTIFY(primaryOrientation, primaryOrientationChanged)

   GUI_CS_PROPERTY_READ(orientation, orientation)
   GUI_CS_PROPERTY_NOTIFY(orientation, orientationChanged)

   GUI_CS_PROPERTY_READ(nativeOrientation, nativeOrientation)

   GUI_CS_PROPERTY_READ(refreshRate, refreshRate)
   GUI_CS_PROPERTY_NOTIFY(refreshRate, refreshRateChanged)

 public:
   QScreen(const QScreen &) = delete;
   QScreen &operator=(const QScreen &) = delete;

   ~QScreen();
   QPlatformScreen *handle() const;

   QString name() const;
   int depth() const;

   QSize size() const;
   QRect geometry() const;

   QSizeF physicalSize() const;

   qreal physicalDotsPerInchX() const;
   qreal physicalDotsPerInchY() const;
   qreal physicalDotsPerInch() const;

   qreal logicalDotsPerInchX() const;
   qreal logicalDotsPerInchY() const;
   qreal logicalDotsPerInch() const;

   qreal devicePixelRatio() const;

   QSize availableSize() const;
   QRect availableGeometry() const;

   QList<QScreen *> virtualSiblings() const;

   QSize virtualSize() const;
   QRect virtualGeometry() const;

   QSize availableVirtualSize() const;
   QRect availableVirtualGeometry() const;

   Qt::ScreenOrientation primaryOrientation() const;
   Qt::ScreenOrientation orientation() const;
   Qt::ScreenOrientation nativeOrientation() const;

   Qt::ScreenOrientations orientationUpdateMask() const;
   void setOrientationUpdateMask(Qt::ScreenOrientations mask);

   int angleBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b) const;
   QTransform transformBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &target) const;
   QRect mapBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &rect) const;

   bool isPortrait(Qt::ScreenOrientation orientation) const;
   bool isLandscape(Qt::ScreenOrientation orientation) const;

   QPixmap grabWindow(WId window, int x = 0, int y = 0, int width = -1, int height = -1);

   qreal refreshRate() const;

   GUI_CS_SIGNAL_1(Public, void geometryChanged(const QRect &geometry))
   GUI_CS_SIGNAL_2(geometryChanged, geometry)

   GUI_CS_SIGNAL_1(Public, void availableGeometryChanged(const QRect &geometry))
   GUI_CS_SIGNAL_2(availableGeometryChanged, geometry)

   GUI_CS_SIGNAL_1(Public, void physicalSizeChanged(const QSizeF &size))
   GUI_CS_SIGNAL_2(physicalSizeChanged, size)

   GUI_CS_SIGNAL_1(Public, void physicalDotsPerInchChanged(qreal dpi))
   GUI_CS_SIGNAL_2(physicalDotsPerInchChanged, dpi)

   GUI_CS_SIGNAL_1(Public, void logicalDotsPerInchChanged(qreal dpi))
   GUI_CS_SIGNAL_2(logicalDotsPerInchChanged, dpi)

   GUI_CS_SIGNAL_1(Public, void virtualGeometryChanged(const QRect &rect))
   GUI_CS_SIGNAL_2(virtualGeometryChanged, rect)

   GUI_CS_SIGNAL_1(Public, void primaryOrientationChanged(Qt::ScreenOrientation orientation))
   GUI_CS_SIGNAL_2(primaryOrientationChanged, orientation)

   GUI_CS_SIGNAL_1(Public, void orientationChanged(Qt::ScreenOrientation orientation))
   GUI_CS_SIGNAL_2(orientationChanged, orientation)

   GUI_CS_SIGNAL_1(Public, void refreshRateChanged(qreal refreshRate))
   GUI_CS_SIGNAL_2(refreshRateChanged, refreshRate)

 protected:
   QScopedPointer<QScreenPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QScreen)
   explicit QScreen(QPlatformScreen *screen);

   friend class QApplicationPrivate;
   friend class QPlatformIntegration;
   friend class QPlatformScreen;
   friend class QHighDpiScaling;
};

Q_GUI_EXPORT QDebug operator<<(QDebug, const QScreen *);

#endif

