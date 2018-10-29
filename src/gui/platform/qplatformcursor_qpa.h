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

#ifndef QPlatformCursor_QPA_H
#define QPlatformCursor_QPA_H

#include <QtCore/QList>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtCore/QWeakPointer>
#include <QtCore/QObject>
#include <QtGui/QPlatformScreen>
#include <QtGui/QCursor>

QT_BEGIN_NAMESPACE

class QPlatformCursor;

class Q_GUI_EXPORT QPlatformCursorImage
{

 public:
   QPlatformCursorImage(const uchar *data, const uchar *mask, int width, int height, int hotX, int hotY) {
      set(data, mask, width, height, hotX, hotY);
   }
   QImage *image() {
      return &cursorImage;
   }
   QPoint hotspot() {
      return hot;
   }
   void set(const uchar *data, const uchar *mask, int width, int height, int hotX, int hotY);
   void set(const QImage &image, int hx, int hy);
   void set(Qt::CursorShape);

 private:
   static void createSystemCursor(int id);
   QImage cursorImage;
   QPoint hot;
};

class QPlatformCursorPrivate
{

 public:
   static QList<QWeakPointer<QPlatformCursor> > getInstances() {
      return instances;
   }
   static QList<QWeakPointer<QPlatformCursor> > instances;
};

class Q_GUI_EXPORT QPlatformCursor : public QObject
{

 public:
   QPlatformCursor(QPlatformScreen *);

   // input methods
   virtual void pointerEvent(const QMouseEvent &event) {
      Q_UNUSED(event);
   }
   virtual void changeCursor(QCursor *widgetCursor, QWidget *widget) = 0;
   virtual QPoint pos() const;
   virtual void setPos(const QPoint &pos);

 protected:
   QPlatformScreen *screen;  // Where to request an update

 private:
   Q_DECLARE_PRIVATE(QPlatformCursor);
   friend void qt_qpa_set_cursor(QWidget *w, bool force);
   friend class QApplicationPrivate;
};

QT_END_NAMESPACE

#endif // QGRAPHICSSYSTEMCURSOR_H
