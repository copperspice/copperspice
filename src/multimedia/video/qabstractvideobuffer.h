/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QABSTRACTVIDEOBUFFER_H
#define QABSTRACTVIDEOBUFFER_H

#include <qmultimedia.h>
#include <qmetatype.h>
#include <qstring.h>

class QVariant;
class QAbstractVideoBufferPrivate;
class QAbstractPlanarVideoBufferPrivate;

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

   virtual void release();

   HandleType handleType() const;

   virtual MapMode mapMode() const = 0;

   virtual uchar *map(MapMode mode, int *numBytes, int *bytesPerLine) = 0;
   int mapPlanes(MapMode mode, int *numBytes, int bytesPerLine[4], uchar *data[4]);
   virtual void unmap() = 0;

   virtual QVariant handle() const;

 protected:
   QAbstractVideoBuffer(QAbstractVideoBufferPrivate &dd, HandleType type);

   QAbstractVideoBufferPrivate *d_ptr;
   HandleType m_type;

 private:
   Q_DECLARE_PRIVATE(QAbstractVideoBuffer)
   Q_DISABLE_COPY(QAbstractVideoBuffer)
};

class Q_MULTIMEDIA_EXPORT QAbstractPlanarVideoBuffer : public QAbstractVideoBuffer
{
 public:
   QAbstractPlanarVideoBuffer(HandleType type);
   virtual ~QAbstractPlanarVideoBuffer();

   uchar *map(MapMode mode, int *numBytes, int *bytesPerLine);
   virtual int map(MapMode mode, int *numBytes, int bytesPerLine[4], uchar *data[4]) = 0;

 protected:
   QAbstractPlanarVideoBuffer(QAbstractPlanarVideoBufferPrivate &dd, HandleType type);

 private:
   Q_DISABLE_COPY(QAbstractPlanarVideoBuffer)
};


Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, QAbstractVideoBuffer::HandleType);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, QAbstractVideoBuffer::MapMode);


Q_DECLARE_METATYPE(QAbstractVideoBuffer::HandleType)
Q_DECLARE_METATYPE(QAbstractVideoBuffer::MapMode)


#endif
