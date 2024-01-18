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

#include "private/qdeclarativestringconverters_p.h"

#include <QtGui/qcolor.h>
#include <QtGui/qvector3d.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdatetime.h>

QT_BEGIN_NAMESPACE

static uchar fromHex(const uchar c, const uchar c2)
{
   uchar rv = 0;
   if (c >= '0' && c <= '9') {
      rv += (c - '0') * 16;
   } else if (c >= 'A' && c <= 'F') {
      rv += (c - 'A' + 10) * 16;
   } else if (c >= 'a' && c <= 'f') {
      rv += (c - 'a' + 10) * 16;
   }

   if (c2 >= '0' && c2 <= '9') {
      rv += (c2 - '0');
   } else if (c2 >= 'A' && c2 <= 'F') {
      rv += (c2 - 'A' + 10);
   } else if (c2 >= 'a' && c2 <= 'f') {
      rv += (c2 - 'a' + 10);
   }

   return rv;
}

static uchar fromHex(const QString &s, int idx)
{
   uchar c  = s.at(idx).toLatin1();
   uchar c2 = s.at(idx + 1).toLatin1();
   return fromHex(c, c2);
}

QVariant QDeclarativeStringConverters::variantFromString(const QString &s)
{
   if (s.isEmpty()) {
      return QVariant(s);
   }
   bool ok = false;
   QRectF r = rectFFromString(s, &ok);
   if (ok) {
      return QVariant(r);
   }
   QColor c = colorFromString(s, &ok);
   if (ok) {
      return QVariant(c);
   }
   QPointF p = pointFFromString(s, &ok);
   if (ok) {
      return QVariant(p);
   }
   QSizeF sz = sizeFFromString(s, &ok);
   if (ok) {
      return QVariant(sz);
   }
   QVector3D v = vector3DFromString(s, &ok);
   if (ok) {
      return QVariant::fromValue(v);
   }

   return QVariant(s);
}

namespace {
int qRoundDouble(double d)
{
   return d >= double(0.0) ? int(d + double(0.5)) : int(d - int(d - 1) + double(0.5)) + int(d - 1);
}
}
QVariant QDeclarativeStringConverters::variantFromString(const QString &s, int preferredType, bool *ok)
{
   switch (preferredType) {
      case QMetaType::Int:
         return QVariant(int(qRoundDouble(s.toDouble(ok))));
      case QMetaType::UInt:
         return QVariant(uint(qRoundDouble(s.toDouble(ok))));
      case QMetaType::QColor:
         return QVariant::fromValue(colorFromString(s, ok));
#ifndef QT_NO_DATESTRING
      case QMetaType::QDate:
         return QVariant::fromValue(dateFromString(s, ok));
      case QMetaType::QTime:
         return QVariant::fromValue(timeFromString(s, ok));
      case QMetaType::QDateTime:
         return QVariant::fromValue(dateTimeFromString(s, ok));
#endif // QT_NO_DATESTRING
      case QMetaType::QPointF:
         return QVariant::fromValue(pointFFromString(s, ok));
      case QMetaType::QPoint:
         return QVariant::fromValue(pointFFromString(s, ok).toPoint());
      case QMetaType::QSizeF:
         return QVariant::fromValue(sizeFFromString(s, ok));
      case QMetaType::QSize:
         return QVariant::fromValue(sizeFFromString(s, ok).toSize());
      case QMetaType::QRectF:
         return QVariant::fromValue(rectFFromString(s, ok));
      case QMetaType::QRect:
         return QVariant::fromValue(rectFFromString(s, ok).toRect());
      case QMetaType::QVector3D:
         return QVariant::fromValue(vector3DFromString(s, ok));
      default:
         if (ok) {
            *ok = false;
         }
         return QVariant();
   }
}

QColor QDeclarativeStringConverters::colorFromString(const QString &s, bool *ok)
{
   if (s.length() == 9 && s.startsWith(QLatin1Char('#'))) {
      uchar a = fromHex(s, 1);
      uchar r = fromHex(s, 3);
      uchar g = fromHex(s, 5);
      uchar b = fromHex(s, 7);
      if (ok) {
         *ok = true;
      }
      return QColor(r, g, b, a);
   } else {
      QColor rv(s);
      if (ok) {
         *ok = rv.isValid();
      }
      return rv;
   }
}

#ifndef QT_NO_DATESTRING
QDate QDeclarativeStringConverters::dateFromString(const QString &s, bool *ok)
{
   QDate d = QDate::fromString(s, Qt::ISODate);
   if (ok) {
      *ok =  d.isValid();
   }
   return d;
}

QTime QDeclarativeStringConverters::timeFromString(const QString &s, bool *ok)
{
   QTime t = QTime::fromString(s, Qt::ISODate);
   if (ok) {
      *ok = t.isValid();
   }
   return t;
}

QDateTime QDeclarativeStringConverters::dateTimeFromString(const QString &s, bool *ok)
{
   QDateTime d = QDateTime::fromString(s, Qt::ISODate);
   if (ok) {
      *ok =  d.isValid();
   }
   return d;
}
#endif // QT_NO_DATESTRING

//expects input of "x,y"
QPointF QDeclarativeStringConverters::pointFFromString(const QString &s, bool *ok)
{
   if (s.count(QLatin1Char(',')) != 1) {
      if (ok) {
         *ok = false;
      }
      return QPointF();
   }

   bool xGood, yGood;
   int index = s.indexOf(QLatin1Char(','));
   qreal xCoord = s.left(index).toDouble(&xGood);
   qreal yCoord = s.mid(index + 1).toDouble(&yGood);
   if (!xGood || !yGood) {
      if (ok) {
         *ok = false;
      }
      return QPointF();
   }

   if (ok) {
      *ok = true;
   }
   return QPointF(xCoord, yCoord);
}

//expects input of "widthxheight"
QSizeF QDeclarativeStringConverters::sizeFFromString(const QString &s, bool *ok)
{
   if (s.count(QLatin1Char('x')) != 1) {
      if (ok) {
         *ok = false;
      }
      return QSizeF();
   }

   bool wGood, hGood;
   int index = s.indexOf(QLatin1Char('x'));
   qreal width = s.left(index).toDouble(&wGood);
   qreal height = s.mid(index + 1).toDouble(&hGood);
   if (!wGood || !hGood) {
      if (ok) {
         *ok = false;
      }
      return QSizeF();
   }

   if (ok) {
      *ok = true;
   }
   return QSizeF(width, height);
}

//expects input of "x,y,widthxheight" //### use space instead of second comma?
QRectF QDeclarativeStringConverters::rectFFromString(const QString &s, bool *ok)
{
   if (s.count(QLatin1Char(',')) != 2 || s.count(QLatin1Char('x')) != 1) {
      if (ok) {
         *ok = false;
      }
      return QRectF();
   }

   bool xGood, yGood, wGood, hGood;
   int index = s.indexOf(QLatin1Char(','));
   qreal x = s.left(index).toDouble(&xGood);
   int index2 = s.indexOf(QLatin1Char(','), index + 1);
   qreal y = s.mid(index + 1, index2 - index - 1).toDouble(&yGood);
   index = s.indexOf(QLatin1Char('x'), index2 + 1);
   qreal width = s.mid(index2 + 1, index - index2 - 1).toDouble(&wGood);
   qreal height = s.mid(index + 1).toDouble(&hGood);
   if (!xGood || !yGood || !wGood || !hGood) {
      if (ok) {
         *ok = false;
      }
      return QRectF();
   }

   if (ok) {
      *ok = true;
   }
   return QRectF(x, y, width, height);
}

//expects input of "x,y,z"
QVector3D QDeclarativeStringConverters::vector3DFromString(const QString &s, bool *ok)
{
   if (s.count(QLatin1Char(',')) != 2) {
      if (ok) {
         *ok = false;
      }
      return QVector3D();
   }

   bool xGood, yGood, zGood;
   int index = s.indexOf(QLatin1Char(','));
   int index2 = s.indexOf(QLatin1Char(','), index + 1);
   qreal xCoord = s.left(index).toDouble(&xGood);
   qreal yCoord = s.mid(index + 1, index2 - index - 1).toDouble(&yGood);
   qreal zCoord = s.mid(index2 + 1).toDouble(&zGood);
   if (!xGood || !yGood || !zGood) {
      if (ok) {
         *ok = false;
      }
      return QVector3D();
   }

   if (ok) {
      *ok = true;
   }
   return QVector3D(xCoord, yCoord, zCoord);
}

QT_END_NAMESPACE
