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

#include <qabstractvideobuffer_p.h>
#include <qvariant.h>
#include <QDebug>

int QAbstractVideoBufferPrivate::map(
   QAbstractVideoBuffer::MapMode mode,
   int *numBytes,
   int bytesPerLine[4],
   uchar *data[4])
{
   data[0] = q_ptr->map(mode, numBytes, bytesPerLine);
   return data[0] ? 1 : 0;
}

QAbstractVideoBuffer::QAbstractVideoBuffer(HandleType type)
   : d_ptr(nullptr), m_type(type)
{
}

// internal
QAbstractVideoBuffer::QAbstractVideoBuffer(QAbstractVideoBufferPrivate &dd, HandleType type)
   : d_ptr(&dd), m_type(type)
{
   d_ptr->q_ptr = this;
}

QAbstractVideoBuffer::~QAbstractVideoBuffer()
{
   delete d_ptr;
}

void QAbstractVideoBuffer::release()
{
   delete this;
}

QAbstractVideoBuffer::HandleType QAbstractVideoBuffer::handleType() const
{
   return m_type;
}

int QAbstractVideoBuffer::mapPlanes(MapMode mode, int *numBytes, int bytesPerLine[4], uchar *data[4])
{
   if (d_ptr) {
      return d_ptr->map(mode, numBytes, bytesPerLine, data);
   } else {
      data[0] = map(mode, numBytes, bytesPerLine);
      return data[0] ? 1 : 0;
   }
}

QVariant QAbstractVideoBuffer::handle() const
{
   return QVariant();
}

int QAbstractPlanarVideoBufferPrivate::map(QAbstractVideoBuffer::MapMode mode, int *numBytes, int bytesPerLine[4], uchar *data[4])
{
   return q_func()->map(mode, numBytes, bytesPerLine, data);
}

QAbstractPlanarVideoBuffer::QAbstractPlanarVideoBuffer(HandleType type)
   : QAbstractVideoBuffer(*new QAbstractPlanarVideoBufferPrivate, type)
{
}

QAbstractPlanarVideoBuffer::QAbstractPlanarVideoBuffer(QAbstractPlanarVideoBufferPrivate &dd, HandleType type)
   : QAbstractVideoBuffer(dd, type)
{
}

QAbstractPlanarVideoBuffer::~QAbstractPlanarVideoBuffer()
{
}

uchar *QAbstractPlanarVideoBuffer::map(MapMode mode, int *numBytes, int *bytesPerLine)
{
   uchar *data[4];
   int strides[4];

   if (map(mode, numBytes, strides, data) > 0) {
      if (bytesPerLine) {
         *bytesPerLine = strides[0];
      }

      return data[0];

   } else {
      return nullptr;
   }
}
QDebug operator<<(QDebug dbg, QAbstractVideoBuffer::HandleType type)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();

   switch (type) {
      case QAbstractVideoBuffer::NoHandle:
         return dbg << "NoHandle";

      case QAbstractVideoBuffer::GLTextureHandle:
         return dbg << "GLTextureHandle";

      case QAbstractVideoBuffer::XvShmImageHandle:
         return dbg << "XvShmImageHandle";

      case QAbstractVideoBuffer::CoreImageHandle:
         return dbg << "CoreImageHandle";

      case QAbstractVideoBuffer::QPixmapHandle:
         return dbg << "QPixmapHandle";

      default:
         return dbg << "UserHandle(" << int(type) << ')';
   }
}
QDebug operator<<(QDebug dbg, QAbstractVideoBuffer::MapMode mode)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();

   switch (mode) {
      case QAbstractVideoBuffer::ReadOnly:
         return dbg << "ReadOnly";

      case QAbstractVideoBuffer::ReadWrite:
         return dbg << "ReadWrite";

      case QAbstractVideoBuffer::WriteOnly:
         return dbg << "WriteOnly";

      default:
         return dbg << "NotMapped";
   }
}
