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

#include <qmemoryvideobuffer_p.h>

#include <qbytearray.h>

#include <qabstractvideobuffer_p.h>

class QMemoryVideoBufferPrivate : public QAbstractVideoBufferPrivate
{
 public:
   QMemoryVideoBufferPrivate()
      : bytesPerLine(0), mapMode(QAbstractVideoBuffer::NotMapped)
   {
   }

   int bytesPerLine;
   QAbstractVideoBuffer::MapMode mapMode;
   QByteArray data;
};

QMemoryVideoBuffer::QMemoryVideoBuffer(const QByteArray &array, int bytesPerLine)
   : QAbstractVideoBuffer(*new QMemoryVideoBufferPrivate, NoHandle)
{
   Q_D(QMemoryVideoBuffer);

   d->data = array;
   d->bytesPerLine = bytesPerLine;
}

QMemoryVideoBuffer::~QMemoryVideoBuffer()
{
}

QAbstractVideoBuffer::MapMode QMemoryVideoBuffer::mapMode() const
{
   return d_func()->mapMode;
}

uchar *QMemoryVideoBuffer::map(MapMode mode, int *numBytes, int *bytesPerLine)
{
   Q_D(QMemoryVideoBuffer);

   if (d->mapMode == NotMapped && d->data.data() && mode != NotMapped) {
      d->mapMode = mode;

      if (numBytes) {
         *numBytes = d->data.size();
      }

      if (bytesPerLine) {
         *bytesPerLine = d->bytesPerLine;
      }

      return reinterpret_cast<uchar *>(d->data.data());

   } else {
      return nullptr;
   }
}

void QMemoryVideoBuffer::unmap()
{
   d_func()->mapMode = NotMapped;
}

