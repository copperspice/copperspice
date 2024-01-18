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

#ifndef QBACKINGSTORE_H
#define QBACKINGSTORE_H

#include <qrect.h>
#include <qwindow.h>
#include <qregion.h>

class QRegion;
class QRect;
class QPoint;
class QImage;
class QBackingStorePrivate;
class QPlatformBackingStore;

class Q_GUI_EXPORT QBackingStore
{
 public:
   explicit QBackingStore(QWindow *window);
   ~QBackingStore();

   QWindow *window() const;

   QPaintDevice *paintDevice();

   // 'window' can be a child window, in which case 'region' is in child window coordinates and
   // offset is the (child) window's offset in relation to the window surface.
   void flush(const QRegion &region, QWindow *window = nullptr, const QPoint &offset = QPoint());

   void resize(const QSize &size);
   QSize size() const;

   bool scroll(const QRegion &area, int dx, int dy);

   void beginPaint(const QRegion &region);
   void endPaint();

   void setStaticContents(const QRegion &region);
   QRegion staticContents() const;
   bool hasStaticContents() const;

   QPlatformBackingStore *handle() const;

 private:
   QScopedPointer<QBackingStorePrivate> d_ptr;
};

#endif
