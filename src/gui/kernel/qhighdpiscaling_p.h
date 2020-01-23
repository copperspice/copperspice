/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QHIGHDPISCALING_P_H
#define QHIGHDPISCALING_P_H

#include <qglobal.h>
#include <qmargins.h>
#include <qmath.h>
#include <qrect.h>
#include <qvector.h>
#include <qregion.h>
#include <qscreen.h>
#include <qwindow.h>

// emerald   #include <qloggingcategory.h>
// emerald - Q_DECLARE_LOGGING_CATEGORY(lcScaling);

class QScreen;
class QPlatformScreen;

typedef QPair<qreal, qreal> QDpi;

class Q_GUI_EXPORT QHighDpiScaling
{

 public:
   static void initHighDpiScaling();
   static void updateHighDpiScaling();
   static void setGlobalFactor(qreal factor);
   static void setScreenFactor(QScreen *window, qreal factor);

   static bool isActive() {
      return m_active;
   }
   static qreal factor(const QWindow *window);
   static qreal factor(const QScreen *screen);
   static qreal factor(const QPlatformScreen *platformScreen);
   static QPoint origin(const QScreen *screen);
   static QPoint origin(const QPlatformScreen *platformScreen);
   static QPoint mapPositionFromNative(const QPoint &pos, const QPlatformScreen *platformScreen);
   static QPoint mapPositionToNative(const QPoint &pos, const QPlatformScreen *platformScreen);
   static QDpi logicalDpi();

 private:
   static qreal screenSubfactor(const QPlatformScreen *screen);

   static qreal m_factor;
   static bool m_active;
   static bool m_usePixelDensity;
   static bool m_globalScalingActive;
   static bool m_pixelDensityScalingActive;
   static bool m_screenFactorSet;
   static QDpi m_logicalDpi;
};

// Coordinate system conversion functions:
// QHighDpi::fromNativePixels   : from physical(screen/backing) to logical pixels
// QHighDpi::toNativePixels     : from logical to physical pixels

namespace QHighDpi {

inline QPointF fromNative(const QPointF &pos, qreal scaleFactor, const QPointF &origin)
{
   return (pos - origin) / scaleFactor + origin;
}

inline QPointF toNative(const QPointF &pos, qreal scaleFactor, const QPointF &origin)
{
   return (pos - origin) * scaleFactor + origin;
}

inline QPoint fromNative(const QPoint &pos, qreal scaleFactor, const QPoint &origin)
{
   return (pos - origin) / scaleFactor + origin;
}

inline QPoint toNative(const QPoint &pos, qreal scaleFactor, const QPoint &origin)
{
   return (pos - origin) * scaleFactor + origin;
}

inline QPoint fromNative(const QPoint &pos, qreal scaleFactor)
{
   return pos / scaleFactor;
}

inline QPoint toNative(const QPoint &pos, qreal scaleFactor)
{
   return pos * scaleFactor;
}

inline QSize fromNative(const QSize &size, qreal scaleFactor)
{
   return size / scaleFactor; // TODO: should we round up?
}

inline QSize toNative(const QSize &size, qreal scaleFactor)
{
   return size * scaleFactor;
}

inline QSizeF fromNative(const QSizeF &size, qreal scaleFactor)
{
   return size / scaleFactor;
}

inline QSizeF toNative(const QSizeF &size, qreal scaleFactor)
{
   return size * scaleFactor;
}

inline QRect fromNative(const QRect &rect, qreal scaleFactor, const QPoint &origin)
{
   return QRect(fromNative(rect.topLeft(), scaleFactor, origin), fromNative(rect.size(), scaleFactor));
}

inline QRect toNative(const QRect &rect, qreal scaleFactor, const QPoint &origin)
{
   return QRect(toNative(rect.topLeft(), scaleFactor, origin), toNative(rect.size(), scaleFactor));

}

inline QRect fromNative(const QRect &rect, const QScreen *screen, const QPoint &screenOrigin)
{
   return fromNative(rect, QHighDpiScaling::factor(screen), screenOrigin);
}

inline QRect fromNativeScreenGeometry(const QRect &nativeScreenGeometry, const QScreen *screen)
{
   return QRect(nativeScreenGeometry.topLeft(),
         fromNative(nativeScreenGeometry.size(), QHighDpiScaling::factor(screen)));
}

inline QPoint fromNativeLocalPosition(const QPoint &pos, const QWindow *window)
{
   const qreal scaleFactor = QHighDpiScaling::factor(window);
   return pos / scaleFactor;
}

inline QPoint toNativeLocalPosition(const QPoint &pos, const QWindow *window)
{
   const qreal scaleFactor = QHighDpiScaling::factor(window);
   return pos * scaleFactor;
}

inline QPointF fromNativeLocalPosition(const QPointF &pos, const QWindow *window)
{
   const qreal scaleFactor = QHighDpiScaling::factor(window);
   return pos / scaleFactor;
}

inline QPointF toNativeLocalPosition(const QPointF &pos, const QWindow *window)
{
   const qreal scaleFactor = QHighDpiScaling::factor(window);
   return pos * scaleFactor;
}

inline QRect fromNativePixels(const QRect &pixelRect, const QPlatformScreen *platformScreen)
{
   const qreal scaleFactor = QHighDpiScaling::factor(platformScreen);
   const QPoint origin = QHighDpiScaling::origin(platformScreen);
   return QRect(fromNative(pixelRect.topLeft(), scaleFactor, origin),
         fromNative(pixelRect.size(), scaleFactor));
}

inline QRect toNativePixels(const QRect &pointRect, const QPlatformScreen *platformScreen)
{
   const qreal scaleFactor = QHighDpiScaling::factor(platformScreen);
   const QPoint origin = QHighDpiScaling::origin(platformScreen);
   return QRect(toNative(pointRect.topLeft(), scaleFactor, origin),
         toNative(pointRect.size(), scaleFactor));
}

inline QRect fromNativePixels(const QRect &pixelRect, const QScreen *screen)
{
   const qreal scaleFactor = QHighDpiScaling::factor(screen);
   const QPoint origin = QHighDpiScaling::origin(screen);
   return QRect(fromNative(pixelRect.topLeft(), scaleFactor, origin),
         fromNative(pixelRect.size(), scaleFactor));
}

inline QRect toNativePixels(const QRect &pointRect, const QScreen *screen)
{
   const qreal scaleFactor = QHighDpiScaling::factor(screen);
   const QPoint origin = QHighDpiScaling::origin(screen);
   return QRect(toNative(pointRect.topLeft(), scaleFactor, origin),
         toNative(pointRect.size(), scaleFactor));
}

inline QRect fromNativePixels(const QRect &pixelRect, const QWindow *window)
{
   if (window && window->isTopLevel() && window->screen()) {
      return fromNativePixels(pixelRect, window->screen());
   } else {
      const qreal scaleFactor = QHighDpiScaling::factor(window);
      return QRect(pixelRect.topLeft() / scaleFactor, fromNative(pixelRect.size(), scaleFactor));
   }
}

inline QRectF toNativePixels(const QRectF &pointRect, const QScreen *screen)
{
   const qreal scaleFactor = QHighDpiScaling::factor(screen);
   const QPoint origin = QHighDpiScaling::origin(screen);
   return QRectF(toNative(pointRect.topLeft(), scaleFactor, origin),
         toNative(pointRect.size(), scaleFactor));
}

inline QRect toNativePixels(const QRect &pointRect, const QWindow *window)
{
   if (window && window->isTopLevel() && window->screen()) {
      return toNativePixels(pointRect, window->screen());
   } else {
      const qreal scaleFactor = QHighDpiScaling::factor(window);
      return QRect(pointRect.topLeft() * scaleFactor, toNative(pointRect.size(), scaleFactor));
   }
}

inline QRectF fromNativePixels(const QRectF &pixelRect, const QScreen *screen)
{
   const qreal scaleFactor = QHighDpiScaling::factor(screen);
   const QPoint origin = QHighDpiScaling::origin(screen);
   return QRectF(fromNative(pixelRect.topLeft(), scaleFactor, origin),
         fromNative(pixelRect.size(), scaleFactor));
}

inline QRectF fromNativePixels(const QRectF &pixelRect, const QWindow *window)
{
   if (window && window->isTopLevel() && window->screen()) {
      return fromNativePixels(pixelRect, window->screen());
   } else {
      const qreal scaleFactor = QHighDpiScaling::factor(window);
      return QRectF(pixelRect.topLeft() / scaleFactor, pixelRect.size() / scaleFactor);
   }
}

inline QRectF toNativePixels(const QRectF &pointRect, const QWindow *window)
{
   if (window && window->isTopLevel() && window->screen()) {
      return toNativePixels(pointRect, window->screen());
   } else {
      const qreal scaleFactor = QHighDpiScaling::factor(window);
      return QRectF(pointRect.topLeft() * scaleFactor, pointRect.size() * scaleFactor);
   }
}

inline QSize fromNativePixels(const QSize &pixelSize, const QWindow *window)
{
   return pixelSize / QHighDpiScaling::factor(window);
}

inline QSize toNativePixels(const QSize &pointSize, const QWindow *window)
{
   return pointSize * QHighDpiScaling::factor(window);
}

inline QSizeF fromNativePixels(const QSizeF &pixelSize, const QWindow *window)
{
   return pixelSize / QHighDpiScaling::factor(window);
}

inline QSizeF toNativePixels(const QSizeF &pointSize, const QWindow *window)
{
   return pointSize * QHighDpiScaling::factor(window);
}

inline QPoint fromNativePixels(const QPoint &pixelPoint, const QScreen *screen)
{
   return fromNative(pixelPoint, QHighDpiScaling::factor(screen), QHighDpiScaling::origin(screen));
}

inline QPoint fromNativePixels(const QPoint &pixelPoint, const QWindow *window)
{
   if (window && window->isTopLevel() && window->screen()) {
      return fromNativePixels(pixelPoint, window->screen());
   } else {
      return pixelPoint / QHighDpiScaling::factor(window);
   }
}

inline QPoint toNativePixels(const QPoint &pointPoint, const QScreen *screen)
{
   return toNative(pointPoint, QHighDpiScaling::factor(screen), QHighDpiScaling::origin(screen));
}

inline QPoint toNativePixels(const QPoint &pointPoint, const QWindow *window)
{
   if (window && window->isTopLevel() && window->screen()) {
      return toNativePixels(pointPoint, window->screen());
   } else {
      return pointPoint * QHighDpiScaling::factor(window);
   }
}

inline QPointF fromNativePixels(const QPointF &pixelPoint, const QScreen *screen)
{
   return fromNative(pixelPoint, QHighDpiScaling::factor(screen), QHighDpiScaling::origin(screen));
}

inline QPointF fromNativePixels(const QPointF &pixelPoint, const QWindow *window)
{
   if (window && window->isTopLevel() && window->screen()) {
      return fromNativePixels(pixelPoint, window->screen());
   } else {
      return pixelPoint / QHighDpiScaling::factor(window);
   }
}

inline QPointF toNativePixels(const QPointF &pointPoint, const QScreen *screen)
{
   return toNative(pointPoint, QHighDpiScaling::factor(screen), QHighDpiScaling::origin(screen));
}

inline QPointF toNativePixels(const QPointF &pointPoint, const QWindow *window)
{
   if (window && window->isTopLevel() && window->screen()) {
      return toNativePixels(pointPoint, window->screen());
   } else {
      return pointPoint * QHighDpiScaling::factor(window);
   }
}

inline QMargins fromNativePixels(const QMargins &pixelMargins, const QWindow *window)
{
   const qreal scaleFactor = QHighDpiScaling::factor(window);
   return QMargins(pixelMargins.left() / scaleFactor, pixelMargins.top() / scaleFactor,
         pixelMargins.right() / scaleFactor, pixelMargins.bottom() / scaleFactor);
}

inline QMargins toNativePixels(const QMargins &pointMargins, const QWindow *window)
{
   const qreal scaleFactor = QHighDpiScaling::factor(window);
   return QMargins(pointMargins.left() * scaleFactor, pointMargins.top() * scaleFactor,
         pointMargins.right() * scaleFactor, pointMargins.bottom() * scaleFactor);
}

inline QRegion fromNativeLocalRegion(const QRegion &pixelRegion, const QWindow *window)
{
   if (!QHighDpiScaling::isActive()) {
      return pixelRegion;
   }

   qreal scaleFactor = QHighDpiScaling::factor(window);
   QRegion pointRegion;
   for (const QRect &rect : pixelRegion.rects()) {
      pointRegion += QRect(fromNative(rect.topLeft(), scaleFactor),
            fromNative(rect.size(), scaleFactor));
   }
   return pointRegion;
}

// When mapping expose events to Qt rects: round top/left towards the origin and
// bottom/right away from the origin, making sure that we cover the whole window.
inline QRegion fromNativeLocalExposedRegion(const QRegion &pixelRegion, const QWindow *window)
{
   if (!QHighDpiScaling::isActive()) {
      return pixelRegion;
   }

   const qreal scaleFactor = QHighDpiScaling::factor(window);
   QRegion pointRegion;
   for (const QRect &rect : pixelRegion.rects()) {
      const QPointF topLeftP = QPointF(rect.topLeft()) / scaleFactor;
      const QPointF bottomRightP = QPointF(rect.bottomRight()) / scaleFactor;
      pointRegion += QRect(QPoint(qFloor(topLeftP.x()), qFloor(topLeftP.y())),
            QPoint(qCeil(bottomRightP.x()), qCeil(bottomRightP.y())));
   }
   return pointRegion;
}

inline QRegion toNativeLocalRegion(const QRegion &pointRegion, const QWindow *window)
{
   if (!QHighDpiScaling::isActive()) {
      return pointRegion;
   }

   qreal scaleFactor = QHighDpiScaling::factor(window);
   QRegion pixelRegon;
   for (const QRect &rect : pointRegion.rects()) {
      pixelRegon += QRect(toNative(rect.topLeft(), scaleFactor), toNative(rect.size(), scaleFactor));
   }
   return pixelRegon;
}

// Any T that has operator/()
template <typename T>
T fromNativePixels(const T &pixelValue, const QWindow *window)
{
   if (!QHighDpiScaling::isActive()) {
      return pixelValue;
   }

   return pixelValue / QHighDpiScaling::factor(window);

}

//##### ?????
template <typename T>
T fromNativePixels(const T &pixelValue, const QScreen *screen)
{
   if (!QHighDpiScaling::isActive()) {
      return pixelValue;
   }

   return pixelValue / QHighDpiScaling::factor(screen);

}

// Any T that has operator*()
template <typename T>
T toNativePixels(const T &pointValue, const QWindow *window)
{
   if (!QHighDpiScaling::isActive()) {
      return pointValue;
   }

   return pointValue * QHighDpiScaling::factor(window);
}

template <typename T>
T toNativePixels(const T &pointValue, const QScreen *screen)
{
   if (!QHighDpiScaling::isActive()) {
      return pointValue;
   }

   return pointValue * QHighDpiScaling::factor(screen);
}

// Any QVector<T> where T has operator/()
template <typename T>
QVector<T> fromNativePixels(const QVector<T> &pixelValues, const QWindow *window)
{
   if (!QHighDpiScaling::isActive()) {
      return pixelValues;
   }

   QVector<T> pointValues;
   for (const T &pixelValue : pixelValues) {
      pointValues.append(pixelValue / QHighDpiScaling::factor(window));
   }
   return pointValues;
}

// Any QVector<T> where T has operator*()
template <typename T>
QVector<T> toNativePixels(const QVector<T> &pointValues, const QWindow *window)
{
   if (!QHighDpiScaling::isActive()) {
      return pointValues;
   }

   QVector<T> pixelValues;
   for (const T &pointValue : pointValues) {
      pixelValues.append(pointValue * QHighDpiScaling::factor(window));
   }
   return pixelValues;
}

} // namespace QHighDpi

#endif
