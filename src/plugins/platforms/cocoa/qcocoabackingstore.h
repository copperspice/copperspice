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

#ifndef QBACKINGSTORE_COCOA_H
#define QBACKINGSTORE_COCOA_H

#include <Cocoa/Cocoa.h>

#include "qcocoawindow.h"
#include "qnsview.h"

#include <qplatform_backingstore.h>

class QCocoaBackingStore : public QPlatformBackingStore
{
 public:
   QCocoaBackingStore(QWindow *window);
   ~QCocoaBackingStore();

   QPaintDevice *paintDevice() override;
   void flush(QWindow *widget, const QRegion &region, const QPoint &offset) override;

#ifndef QT_NO_OPENGL
   QImage toImage() const override;
#else
   QImage toImage() const; // No QPlatformBackingStore::toImage() for NO_OPENGL builds.
#endif

   void resize (const QSize &size, const QRegion &) override;
   bool scroll(const QRegion &area, int dx, int dy) override;
   void beginPaint(const QRegion &region) override;
   qreal getBackingStoreDevicePixelRatio();

 private:
   QImage m_qImage;
   QSize m_requestedSize;
};

#endif
