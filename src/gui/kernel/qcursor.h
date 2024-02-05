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

#ifndef QCURSOR_H
#define QCURSOR_H

#include <qpoint.h>
#include <qvariant.h>
#include <qwindowdefs.h>

class QScreen;

#ifdef QT_NO_CURSOR
// fake class, used on touchscreen devices

class Q_GUI_EXPORT QCursor
{
 public:
   static QPoint pos();
   static QPoint pos(const QScreen *screen);

   static void setPos(int x, int y);

   static void setPos(QScreen *screen, int x, int y);

   static void setPos(const QPoint &p) {
      setPos(p.x(), p.y());
   }

 private:
   QCursor();
};

#endif


#ifndef QT_NO_CURSOR

class QCursorData;
class QBitmap;
class QPixmap;

class Q_GUI_EXPORT QCursor
{
 public:
   QCursor();
   QCursor(Qt::CursorShape shape);
   QCursor(const QBitmap &bitmap, const QBitmap &mask, int hotX = -1, int hotY = -1);
   QCursor(const QPixmap &pixmap, int hotX = -1, int hotY = -1);
   QCursor(const QCursor &other);

   QCursor(QCursor &&other) : d(other.d) {
      other.d = nullptr;
   }

   ~QCursor();

   QCursor &operator=(const QCursor &cursor);

   QCursor &operator=(QCursor &&other) {
      qSwap(d, other.d);
      return *this;
   }

   operator QVariant() const;

   Qt::CursorShape shape() const;
   void setShape(Qt::CursorShape newShape);

   const QBitmap *bitmap() const;
   const QBitmap *mask() const;
   QPixmap pixmap() const;
   QPoint hotSpot() const;

   static QPoint pos();
   static QPoint pos(const QScreen *screen);
   static void setPos(int x, int y);

   static void setPos(QScreen *screen, int x, int y);

   static void setPos(const QPoint &p) {
      setPos(p.x(), p.y());
   }

   static void setPos(QScreen *screen, const QPoint &p) {
      setPos(screen, p.x(), p.y());
   }

 private:
   QCursorData *d;

};

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &outS, const QCursor &cursor);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &inS, QCursor &cursor);

template <>
inline bool CustomType_T<QCursor>::compare(const CustomType &other) const {
   auto ptr = dynamic_cast<const CustomType_T<QCursor>*>(&other);

   if (ptr != nullptr) {
      return m_value.shape() == (ptr->m_value).shape();
   }

   return false;
}

#endif // QT_NO_CURSOR

#endif
