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

#include <qsize.h>
#include <qdatastream.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

void QSize::transpose()
{
   int tmp = wd;
   wd = ht;
   ht = tmp;
}

void QSize::scale(const QSize &s, Qt::AspectRatioMode mode)
{
   if (mode == Qt::IgnoreAspectRatio || wd == 0 || ht == 0) {
      wd = s.wd;
      ht = s.ht;
   } else {
      bool useHeight;
      qint64 rw = qint64(s.ht) * qint64(wd) / qint64(ht);

      if (mode == Qt::KeepAspectRatio) {
         useHeight = (rw <= s.wd);
      } else { // mode == Qt::KeepAspectRatioByExpanding
         useHeight = (rw >= s.wd);
      }

      if (useHeight) {
         wd = rw;
         ht = s.ht;
      } else {
         ht = qint32(qint64(s.wd) * qint64(ht) / qint64(wd));
         wd = s.wd;
      }
   }
}

QDataStream &operator<<(QDataStream &s, const QSize &sz)
{
   s << (qint32)sz.width() << (qint32)sz.height();

   return s;
}

QDataStream &operator>>(QDataStream &s, QSize &sz)
{
   qint32 w, h;
   s >> w;
   sz.rwidth() = w;
   s >> h;
   sz.rheight() = h;

   return s;
}

QDebug operator<<(QDebug dbg, const QSize &s)
{
   dbg.nospace() << "QSize(" << s.width() << ", " << s.height() << ')';
   return dbg.space();
}

void QSizeF::transpose()
{
   qreal tmp = wd;
   wd = ht;
   ht = tmp;
}
void QSizeF::scale(const QSizeF &s, Qt::AspectRatioMode mode)
{
   if (mode == Qt::IgnoreAspectRatio || qIsNull(wd) || qIsNull(ht)) {
      wd = s.wd;
      ht = s.ht;
   } else {
      bool useHeight;
      qreal rw = s.ht * wd / ht;

      if (mode == Qt::KeepAspectRatio) {
         useHeight = (rw <= s.wd);
      } else { // mode == Qt::KeepAspectRatioByExpanding
         useHeight = (rw >= s.wd);
      }

      if (useHeight) {
         wd = rw;
         ht = s.ht;
      } else {
         ht = s.wd * ht / wd;
         wd = s.wd;
      }
   }
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const QSizeF &sz)
{
   s << double(sz.width()) << double(sz.height());
   return s;
}

QDataStream &operator>>(QDataStream &s, QSizeF &sz)
{
   double w, h;
   s >> w;
   s >> h;
   sz.setWidth(qreal(w));
   sz.setHeight(qreal(h));
   return s;
}
#endif

QDebug operator<<(QDebug dbg, const QSizeF &s)
{
   dbg.nospace() << "QSizeF(" << s.width() << ", " << s.height() << ')';
   return dbg.space();
}

QT_END_NAMESPACE
