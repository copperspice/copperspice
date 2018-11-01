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

#ifndef QSCREEN_H
#define QSCREEN_H

#include <qlist.h>
#include <qobject.h>
#include <qrect.h>
#include <qsize.h>
#include <qsizef.h>
#include <qtransform.h>
#include <qnamespace.h>

class QPlatformScreen;
class QScreenPrivate;
//emerald    class QWindow;
class QRect;
class QPixmap;
class QDebug;

class Q_GUI_EXPORT QScreen : public QObject
{
    CS_OBJECT(QScreen)
    Q_DECLARE_PRIVATE(QScreen)

    CS_PROPERTY_READ(name, name)
    CS_PROPERTY_CONSTANT(name)
    CS_PROPERTY_READ(depth, depth)
    CS_PROPERTY_CONSTANT(depth)
    CS_PROPERTY_READ(size, size)
    CS_PROPERTY_NOTIFY(size, geometryChanged)
    CS_PROPERTY_READ(availableSize, availableSize)
    CS_PROPERTY_NOTIFY(availableSize, availableGeometryChanged)
    CS_PROPERTY_READ(virtualSize, virtualSize)
    CS_PROPERTY_NOTIFY(virtualSize, virtualGeometryChanged)
    CS_PROPERTY_READ(availableVirtualSize, availableVirtualSize)
    CS_PROPERTY_NOTIFY(availableVirtualSize, virtualGeometryChanged)
    CS_PROPERTY_READ(geometry, geometry)
    CS_PROPERTY_NOTIFY(geometry, geometryChanged)
    CS_PROPERTY_READ(availableGeometry, availableGeometry)
    CS_PROPERTY_NOTIFY(availableGeometry, availableGeometryChanged)
    CS_PROPERTY_READ(virtualGeometry, virtualGeometry)
    CS_PROPERTY_NOTIFY(virtualGeometry, virtualGeometryChanged)
    CS_PROPERTY_READ(availableVirtualGeometry, availableVirtualGeometry)
    CS_PROPERTY_NOTIFY(availableVirtualGeometry, virtualGeometryChanged)
    CS_PROPERTY_READ(physicalSize, physicalSize)
    CS_PROPERTY_NOTIFY(physicalSize, physicalSizeChanged)
    CS_PROPERTY_READ(physicalDotsPerInchX, physicalDotsPerInchX)
    CS_PROPERTY_NOTIFY(physicalDotsPerInchX, physicalDotsPerInchChanged)
    CS_PROPERTY_READ(physicalDotsPerInchY, physicalDotsPerInchY)
    CS_PROPERTY_NOTIFY(physicalDotsPerInchY, physicalDotsPerInchChanged)
    CS_PROPERTY_READ(physicalDotsPerInch, physicalDotsPerInch)
    CS_PROPERTY_NOTIFY(physicalDotsPerInch, physicalDotsPerInchChanged)
    CS_PROPERTY_READ(logicalDotsPerInchX, logicalDotsPerInchX)
    CS_PROPERTY_NOTIFY(logicalDotsPerInchX, logicalDotsPerInchChanged)
    CS_PROPERTY_READ(logicalDotsPerInchY, logicalDotsPerInchY)
    CS_PROPERTY_NOTIFY(logicalDotsPerInchY, logicalDotsPerInchChanged)
    CS_PROPERTY_READ(logicalDotsPerInch, logicalDotsPerInch)
    CS_PROPERTY_NOTIFY(logicalDotsPerInch, logicalDotsPerInchChanged)
    CS_PROPERTY_READ(devicePixelRatio, devicePixelRatio)
    CS_PROPERTY_NOTIFY(devicePixelRatio, physicalDotsPerInchChanged)
    CS_PROPERTY_READ(primaryOrientation, primaryOrientation)
    CS_PROPERTY_NOTIFY(primaryOrientation, primaryOrientationChanged)
    CS_PROPERTY_READ(orientation, orientation)
    CS_PROPERTY_NOTIFY(orientation, orientationChanged)
    CS_PROPERTY_READ(nativeOrientation, nativeOrientation)
    CS_PROPERTY_READ(refreshRate, refreshRate)
    CS_PROPERTY_NOTIFY(refreshRate, refreshRateChanged)

public:
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

    QPixmap grabWindow(WId window, int x = 0, int y = 0, int w = -1, int h = -1);

    qreal refreshRate() const;

    CS_SIGNAL_1(Public, void geometryChanged(const QRect & geometry))
    CS_SIGNAL_2(geometryChanged,geometry)

    CS_SIGNAL_1(Public, void availableGeometryChanged(const QRect & geometry))
    CS_SIGNAL_2(availableGeometryChanged,geometry)

    CS_SIGNAL_1(Public, void physicalSizeChanged(const QSizeF & size))
    CS_SIGNAL_2(physicalSizeChanged,size)

    CS_SIGNAL_1(Public, void physicalDotsPerInchChanged(qreal dpi))
    CS_SIGNAL_2(physicalDotsPerInchChanged,dpi)

    CS_SIGNAL_1(Public, void logicalDotsPerInchChanged(qreal dpi))
    CS_SIGNAL_2(logicalDotsPerInchChanged,dpi)

    CS_SIGNAL_1(Public, void virtualGeometryChanged(const QRect & rect))
    CS_SIGNAL_2(virtualGeometryChanged,rect)

    CS_SIGNAL_1(Public, void primaryOrientationChanged(Qt::ScreenOrientation orientation))
    CS_SIGNAL_2(primaryOrientationChanged,orientation)

    CS_SIGNAL_1(Public, void orientationChanged(Qt::ScreenOrientation orientation))
    CS_SIGNAL_2(orientationChanged,orientation)

    CS_SIGNAL_1(Public, void refreshRateChanged(qreal refreshRate))
    CS_SIGNAL_2(refreshRateChanged,refreshRate)

protected:
   QScopedPointer<QScreenPrivate> d_ptr;

private:
    explicit QScreen(QPlatformScreen *screen);

    Q_DISABLE_COPY(QScreen)

    friend class QGuiApplicationPrivate;
    friend class QPlatformIntegration;
    friend class QPlatformScreen;
    friend class QHighDpiScaling;
};

Q_GUI_EXPORT QDebug operator<<(QDebug, const QScreen *);

#endif

