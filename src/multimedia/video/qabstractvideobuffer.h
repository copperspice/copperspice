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

#ifndef QABSTRACTVIDEOBUFFER_H
#define QABSTRACTVIDEOBUFFER_H

#include <QtCore/qmetatype.h>

QT_BEGIN_NAMESPACE

class QVariant;
class QAbstractVideoBufferPrivate;

class Q_MULTIMEDIA_EXPORT QAbstractVideoBuffer
{
 public:
   enum HandleType {
      NoHandle,
      GLTextureHandle,
      XvShmImageHandle,
      CoreImageHandle,
      QPixmapHandle,
      UserHandle = 1000
   };

   enum MapMode {
      NotMapped = 0x00,
      ReadOnly  = 0x01,
      WriteOnly = 0x02,
      ReadWrite = ReadOnly | WriteOnly
   };

   QAbstractVideoBuffer(HandleType type);
   virtual ~QAbstractVideoBuffer();

   HandleType handleType() const;

   virtual MapMode mapMode() const = 0;

   virtual uchar *map(MapMode mode, int *numBytes, int *bytesPerLine) = 0;
   virtual void unmap() = 0;

   virtual QVariant handle() const;

 protected:
   QAbstractVideoBuffer(QAbstractVideoBufferPrivate &dd, HandleType type);

   QAbstractVideoBufferPrivate *d_ptr;

 private:
   Q_DECLARE_PRIVATE(QAbstractVideoBuffer)
   Q_DISABLE_COPY(QAbstractVideoBuffer)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QAbstractVideoBuffer::HandleType)
Q_DECLARE_METATYPE(QAbstractVideoBuffer::MapMode)


#endif
