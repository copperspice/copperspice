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

#ifndef QVOLATILEIMAGE_P_H
#define QVOLATILEIMAGE_P_H

#include <QtGui/qimage.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QVolatileImageData;

class Q_GUI_EXPORT QVolatileImage
{
 public:
   QVolatileImage();
   QVolatileImage(int w, int h, QImage::Format format);
   explicit QVolatileImage(const QImage &sourceImage);
   explicit QVolatileImage(void *nativeImage, void *nativeMask = 0);
   QVolatileImage(const QVolatileImage &other);
   ~QVolatileImage();
   QVolatileImage &operator=(const QVolatileImage &rhs);

   bool paintingActive() const;
   bool isNull() const;
   QImage::Format format() const;
   int width() const;
   int height() const;
   int bytesPerLine() const;
   int byteCount() const;
   int depth() const;
   bool hasAlphaChannel() const;
   void beginDataAccess() const;
   void endDataAccess(bool readOnly = false) const;
   uchar *bits();
   const uchar *constBits() const;
   bool ensureFormat(QImage::Format format);
   QImage toImage() const;
   QImage &imageRef();
   const QImage &constImageRef() const;
   QPaintEngine *paintEngine();
   void setAlphaChannel(const QPixmap &alphaChannel);
   void fill(uint pixelValue);
   void *duplicateNativeImage() const;
   void copyFrom(QVolatileImage *source, const QRect &rect);

 private:
   QSharedDataPointer<QVolatileImageData> d;
};

QT_END_NAMESPACE

#endif // QVOLATILEIMAGE_P_H
