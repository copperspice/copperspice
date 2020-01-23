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

#ifndef QREGION_H
#define QREGION_H

#include <qatomic.h>
#include <qrect.h>
#include <qwindowdefs.h>

#include <qdatastream.h>

template <class T> class QVector;
class QVariant;
struct QRegionPrivate;


class QBitmap;

class Q_GUI_EXPORT QRegion
{
 public:
   enum RegionType { Rectangle, Ellipse };

   QRegion();
   QRegion(int x, int y, int w, int h, RegionType t = Rectangle);
   QRegion(const QRect &r, RegionType t = Rectangle);
   QRegion(const QPolygon &pa, Qt::FillRule fillRule = Qt::OddEvenFill);

   QRegion(const QRegion &region);
   QRegion(const QBitmap &bitmap);
   ~QRegion();
   QRegion &operator=(const QRegion &);

   inline QRegion &operator=(QRegion &&other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QRegion &other) {
      qSwap(d, other.d);
   }

   bool isEmpty() const;
   bool isNull() const;

   bool contains(const QPoint &p) const;
   bool contains(const QRect &r) const;

   void translate(int dx, int dy);
   inline void translate(const QPoint &p) {
      translate(p.x(), p.y());
   }

   QRegion translated(int dx, int dy) const;

   inline QRegion translated(const QPoint &p) const {
      return translated(p.x(), p.y());
   }

   QRegion united(const QRegion &r) const;
   QRegion united(const QRect &r) const;
   QRegion intersected(const QRegion &r) const;
   QRegion intersected(const QRect &r) const;

   QRegion subtracted(const QRegion &r) const;
   QRegion xored(const QRegion &r) const;


   bool intersects(const QRegion &r) const;
   bool intersects(const QRect &r) const;

   QRect boundingRect() const;
   QVector<QRect> rects() const;

   void setRects(const QRect *rect, int num);
   int rectCount() const;

   QRegion operator|(const QRegion &r) const;
   QRegion operator+(const QRegion &r) const;
   QRegion operator+(const QRect &r) const;
   QRegion operator&(const QRegion &r) const;
   QRegion operator&(const QRect &r) const;
   QRegion operator-(const QRegion &r) const;
   QRegion operator^(const QRegion &r) const;

   QRegion &operator|=(const QRegion &r);
   QRegion &operator+=(const QRegion &r);
   QRegion &operator+=(const QRect &r);
   QRegion &operator&=(const QRegion &r);
   QRegion &operator&=(const QRect &r);
   QRegion &operator-=(const QRegion &r);
   QRegion &operator^=(const QRegion &r);

   bool operator==(const QRegion &r) const;
   inline bool operator!=(const QRegion &r) const {
      return !(operator==(r));
   }
   operator QVariant() const;

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QRegion &);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QRegion &);

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


Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QRegion &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QRegion &);


Q_GUI_EXPORT QDebug operator<<(QDebug, const QRegion &);



#endif // QREGION_H
