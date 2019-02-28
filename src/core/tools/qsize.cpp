/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qsize.h>
#include <qdatastream.h>
#include <qdebug_p.h>

void QSize::transpose()
{
   int tmp = wd;
   wd = ht;
   ht = tmp;
}

QSize QSize::scaled(const QSize &s, Qt::AspectRatioMode mode) const
{
   if (mode == Qt::IgnoreAspectRatio || wd == 0 || ht == 0) {
     return s;

   } else {
      bool useHeight;
      qint64 rw = qint64(s.ht) * qint64(wd) / qint64(ht);

      if (mode == Qt::KeepAspectRatio) {
         useHeight = (rw <= s.wd);
      } else { // mode == Qt::KeepAspectRatioByExpanding
         useHeight = (rw >= s.wd);
      }

      if (useHeight) {
            return QSize(rw, s.ht);
      } else {
            return QSize(s.wd,
                         qint32(qint64(s.wd) * qint64(ht) / qint64(wd)));
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
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "QSize(";
    QtDebugUtils::formatQSize(dbg, s);
    dbg << ')';
    return dbg;
}

void QSizeF::transpose()
{
   qreal tmp = wd;
   wd = ht;
   ht = tmp;
}
QSizeF QSizeF::scaled(const QSizeF &s, Qt::AspectRatioMode mode) const
{
   if (mode == Qt::IgnoreAspectRatio || qIsNull(wd) || qIsNull(ht)) {
        return s;
   } else {
      bool useHeight;
      qreal rw = s.ht * wd / ht;

      if (mode == Qt::KeepAspectRatio) {
         useHeight = (rw <= s.wd);
      } else { // mode == Qt::KeepAspectRatioByExpanding
         useHeight = (rw >= s.wd);
      }

      if (useHeight) {
            return QSizeF(rw, s.ht);
      } else {
            return QSizeF(s.wd, s.wd * ht / wd);
      }
   }
}

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

QDebug operator<<(QDebug dbg, const QSizeF &s)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "QSizeF(";
    QtDebugUtils::formatQSize(dbg, s);
    dbg << ')';
    return dbg;
}

