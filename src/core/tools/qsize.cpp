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
      } else {
         // mode == Qt::KeepAspectRatioByExpanding
         useHeight = (rw >= s.wd);
      }

      if (useHeight) {
         return QSize(rw, s.ht);
      } else {
         return QSize(s.wd, qint32(qint64(s.wd) * qint64(ht) / qint64(wd)));
      }
   }
}

QDataStream &operator<<(QDataStream &stream, const QSize &size)
{
   stream << (qint32)size.width() << (qint32)size.height();

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QSize &size)
{
   qint32 w, h;
   stream >> w;
   size.rwidth() = w;

   stream >> h;
   size.rheight() = h;

   return stream;
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

QDataStream &operator<<(QDataStream &stream, const QSizeF &sizeF)
{
   stream << double(sizeF.width()) << double(sizeF.height());

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QSizeF &sizeF)
{
   double w, h;
   stream >> w;
   stream >> h;

   sizeF.setWidth(qreal(w));
   sizeF.setHeight(qreal(h));

   return stream;
}

QDebug operator<<(QDebug dbg, const QSizeF &size)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   dbg << "QSizeF(";

   QtDebugUtils::formatQSize(dbg, size);
   dbg << ')';

   return dbg;
}
