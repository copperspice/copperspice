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

#ifndef QPEN_H
#define QPEN_H

#include <QtGui/qcolor.h>
#include <QtGui/qbrush.h>



class QVariant;
class QPenPrivate;
class QBrush;
class QPen;

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPen &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPen &);


class Q_GUI_EXPORT QPen
{
 public:
   QPen();
   QPen(Qt::PenStyle);
   QPen(const QColor &color);
   QPen(const QBrush &brush, qreal width, Qt::PenStyle s = Qt::SolidLine,
      Qt::PenCapStyle c = Qt::SquareCap, Qt::PenJoinStyle j = Qt::BevelJoin);


   QPen(const QPen &pen);

   QPen(QPen &&other) : d(other.d) {
      other.d = nullptr;
   }

   ~QPen();

   QPen &operator=(const QPen &pen);

   QPen &operator=(QPen &&other) {
      qSwap(d, other.d);
      return *this;
   }

   void swap(QPen &other) {
      qSwap(d, other.d);
   }

   Qt::PenStyle style() const;
   void setStyle(Qt::PenStyle);

   QVector<qreal> dashPattern() const;
   void setDashPattern(const QVector<qreal> &pattern);

   qreal dashOffset() const;
   void setDashOffset(qreal doffset);

   qreal miterLimit() const;
   void setMiterLimit(qreal limit);

   qreal widthF() const;
   void setWidthF(qreal width);

   int width() const;
   void setWidth(int width);

   QColor color() const;
   void setColor(const QColor &color);

   QBrush brush() const;
   void setBrush(const QBrush &brush);

   bool isSolid() const;

   Qt::PenCapStyle capStyle() const;
   void setCapStyle(Qt::PenCapStyle pcs);

   Qt::PenJoinStyle joinStyle() const;
   void setJoinStyle(Qt::PenJoinStyle pcs);

   bool isCosmetic() const;
   void setCosmetic(bool cosmetic);

   bool operator==(const QPen &p) const;

   bool operator!=(const QPen &p) const {
      return !(operator==(p));
   }
   operator QVariant() const;

   bool isDetached();

   typedef QPenPrivate *DataPtr;

   inline DataPtr &data_ptr() {
      return d;
   }

 private:
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPen &);
   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPen &);

   void detach();
   class QPenPrivate *d;
};


Q_GUI_EXPORT QDebug operator<<(QDebug, const QPen &);

#endif
