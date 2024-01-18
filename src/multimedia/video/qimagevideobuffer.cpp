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

#include <qimagevideobuffer_p.h>

#include <qimage.h>
#include <qvariant.h>

#include <qabstractvideobuffer_p.h>

class QImageVideoBufferPrivate : public QAbstractVideoBufferPrivate
{
 public:
   QImageVideoBufferPrivate()
      : mapMode(QAbstractVideoBuffer::NotMapped) {
   }

   QAbstractVideoBuffer::MapMode mapMode;
   QImage image;
};

QImageVideoBuffer::QImageVideoBuffer(const QImage &image)
   : QAbstractVideoBuffer(*new QImageVideoBufferPrivate, NoHandle)
{
   Q_D(QImageVideoBuffer);

   d->image = image;
}

QImageVideoBuffer::~QImageVideoBuffer()
{
}

QAbstractVideoBuffer::MapMode QImageVideoBuffer::mapMode() const
{
   return d_func()->mapMode;
}

uchar *QImageVideoBuffer::map(MapMode mode, int *numBytes, int *bytesPerLine)
{
   Q_D(QImageVideoBuffer);

   if (d->mapMode == NotMapped && d->image.bits() && mode != NotMapped) {
      d->mapMode = mode;

      if (numBytes) {
         *numBytes = d->image.byteCount();
      }

      if (bytesPerLine) {
         *bytesPerLine = d->image.bytesPerLine();
      }

      return d->image.bits();

   } else {
      return nullptr;
   }
}

void QImageVideoBuffer::unmap()
{
   Q_D(QImageVideoBuffer);

   d->mapMode = NotMapped;
}
