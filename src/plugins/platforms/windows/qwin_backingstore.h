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

#ifndef QWINDOWSBACKINGSTORE_H
#define QWINDOWSBACKINGSTORE_H

#include <qwin_additional.h>
#include <qplatform_backingstore.h>
#include <qscopedpointer.h>

class QWindowsWindow;
class QWindowsNativeImage;

class QWindowsBackingStore : public QPlatformBackingStore
{
 public:
   QWindowsBackingStore(QWindow *window);

   QWindowsBackingStore(const QWindowsBackingStore &) = delete;
   QWindowsBackingStore &operator=(const QWindowsBackingStore &) = delete;

   ~QWindowsBackingStore();

   QPaintDevice *paintDevice() override;
   void flush(QWindow *window, const QRegion &region, const QPoint &offset) override;
   void resize(const QSize &size, const QRegion &r) override;
   bool scroll(const QRegion &area, int dx, int dy) override;
   void beginPaint(const QRegion &) override;

   HDC getDC() const;

#ifndef QT_NO_OPENGL
   QImage toImage() const override;
#endif

 private:
   QScopedPointer<QWindowsNativeImage> m_image;
   bool m_alphaNeedsFill;
};

#endif
