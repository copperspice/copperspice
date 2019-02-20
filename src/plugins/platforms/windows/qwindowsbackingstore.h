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

#ifndef QWINDOWSBACKINGSTORE_H
#define QWINDOWSBACKINGSTORE_H

#include "qtwindows_additional.h"

#include <qplatform_backingstore.h>
#include <QScopedPointer>

class QWindowsWindow;
class QWindowsNativeImage;

class QWindowsBackingStore : public QPlatformBackingStore
{
   Q_DISABLE_COPY(QWindowsBackingStore)

 public:
   QWindowsBackingStore(QWindow *window);
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
