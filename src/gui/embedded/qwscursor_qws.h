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

#ifndef QWSCURSOR_QWS_H
#define QWSCURSOR_QWS_H

#include <QtGui/qimage.h>
#include <QtGui/qregion.h>

QT_BEGIN_NAMESPACE

class QWSCursor
{
 public:
   QWSCursor() {}
   QWSCursor(const uchar *data, const uchar *mask, int width, int height, int hotX, int hotY) {
      set(data, mask, width, height, hotX, hotY);
   }

   void set(const uchar *data, const uchar *mask, int width, int height, int hotX, int hotY);

   QPoint hotSpot() const {
      return hot;
   }

   QImage &image() {
      return cursor;
   }

   static QWSCursor *systemCursor(int id);

 private:
   static void createSystemCursor(int id);
   void createDropShadow(int dropx, int dropy);

   QPoint hot;
   QImage cursor;
};

QT_END_NAMESPACE

#endif // QWSCURSOR_QWS_H
