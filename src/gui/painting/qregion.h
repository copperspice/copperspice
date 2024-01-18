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

#ifndef QREGION_H
#define QREGION_H

#include <qatomic.h>
#include <qrect.h>
#include <qwindowdefs.h>
#include <qdatastream.h>

class QBitmap;
class QVariant;

struct QRegionPrivate;

template <class T>
class QVector;

class Q_GUI_EXPORT QRegion
{
 public:
   enum RegionType { Rectangle, Ellipse };

   QRegion();
   QRegion(int x, int y, int w, int h, RegionType t = Rectangle);
   QRegion(const QRect &rect, RegionType t = Rectangle);
   QRegion(const QPolygon &polygon, Qt::FillRule fillRule = Qt::OddEvenFill);

   QRegion(const QRegion &other);
   QRegion(const QBitmap &bitmap);

   ~QRegion();

   QRegion &operator=(const QRegion &other);

   inline QRegion &operator=(QRegion &&other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QRegion &other) {
      qSwap(d, other.d);
   }

   bool isEmpty() const;
   bool isNull() const;

   bool contains(const QPoint &point) const;
   bool contains(const QRect &rect) const;

   void translate(int dx, int dy);
   inline void translate(const QPoint &point) {
      translate(point.x(), point.y());
   }

   QRegion translated(int dx, int dy) const;

   inline QRegion translated(const QPoint &point) const {
      return translated(point.x(), point.y());
   }

   QRegion united(const QRegion &region) const;
   QRegion united(const QRect &rect) const;
   QRegion intersected(const QRegion &region) const;
   QRegion intersected(const QRect &rect) const;

   QRegion subtracted(const QRegion &region) const;
   QRegion xored(const QRegion &region) const;

   bool intersects(const QRegion &region) const;
   bool intersects(const QRect &rect) const;

   QRect boundingRect() const;
   QVector<QRect> rects() const;

   void setRects(const QRect *rects, int rectCount);
   int rectCount() const;

   QRegion operator|(const QRegion &other) const;
   QRegion operator+(const QRegion &other) const;
   QRegion operator+(const QRect &rect) const;
   QRegion operator&(const QRegion &other) const;
   QRegion operator&(const QRect &rect) const;
   QRegion operator-(const QRegion &other) const;
   QRegion operator^(const QRegion &other) const;

   QRegion &operator|=(const QRegion &other);
   QRegion &operator+=(const QRegion &other);
   QRegion &operator+=(const QRect &rect);
   QRegion &operator&=(const QRegion &other);
   QRegion &operator&=(const QRect &rect);
   QRegion &operator-=(const QRegion &other);
   QRegion &operator^=(const QRegion &other);

   bool operator==(const QRegion &other) const;

   inline bool operator!=(const QRegion &other) const {
      return !(operator==(other));
   }
   operator QVariant() const;

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QRegion &region);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QRegion &region);

 private:
   QRegion copy() const;   // helper of detach
   void detach();

   void exec(const QByteArray &ba, int ver = 0, QDataStream::ByteOrder byteOrder = QDataStream::BigEndian);

   struct QRegionData {
      QtPrivate::RefCount ref;
      QRegionPrivate *qt_rgn;

   };

   struct QRegionData *d;
   static const struct QRegionData shared_empty;
   static void cleanUp(QRegionData *x);

   friend bool qt_region_strictContains(const QRegion &region, const QRect &rect);
   friend struct QRegionPrivate;

};

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QRegion &region);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QRegion &region);

Q_GUI_EXPORT QDebug operator<<(QDebug, const QRegion &region);

#endif
