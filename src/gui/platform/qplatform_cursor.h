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

#ifndef QPlatform_Cursor_H
#define QPlatform_Cursor_H

#include <qcursor.h>
#include <qlist.h>
#include <qimage.h>
#include <qmouseevent.h>
#include <qweakpointer.h>
#include <qobject.h>
#include <qplatform_screen.h>

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

   QPoint hotspot() const {
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
   static QList<QPlatformCursor *> getInstances();
};

class Q_GUI_EXPORT QPlatformCursor : public QObject
{

 public:
   QPlatformCursor();

   // input methods
   virtual void pointerEvent(const QMouseEvent &event) {
      (void) event;
   }

   virtual void changeCursor(QCursor *windowCursor, QWindow *window) = 0;
   virtual QPoint pos() const;
   virtual void setPos(const QPoint &pos);

 protected:
   QScopedPointer<QPlatformCursorPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QPlatformCursor)

   friend void cs_internal_set_cursor(QWidget *w, bool force);
   friend class QApplicationPrivate;
};

#endif
