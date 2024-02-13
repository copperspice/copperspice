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

#ifndef QFILEDEVICE_P_H
#define QFILEDEVICE_P_H

#include <qiodevice_p.h>
#include <qringbuffer_p.h>

class QAbstractFileEngine;
class QFSFileEngine;

class QFileDevicePrivate : public QIODevicePrivate
{
   Q_DECLARE_PUBLIC(QFileDevice)

 protected:
   QFileDevicePrivate();
   ~QFileDevicePrivate();

   virtual QAbstractFileEngine *engine() const;

   QFileDevice::FileHandleFlags handleFlags;

   mutable QAbstractFileEngine *fileEngine;
   bool lastWasWrite;
   QRingBuffer writeBuffer;
   inline bool ensureFlushed() const;

   bool putCharHelper(char c) override;

   QFileDevice::FileError error;
   void setError(QFileDevice::FileError err);
   void setError(QFileDevice::FileError err, const QString &errorString);
   void setError(QFileDevice::FileError err, int errNum);

   mutable qint64 cachedSize;
};

inline bool QFileDevicePrivate::ensureFlushed() const
{
   // This function ensures that the write buffer has been flushed (const
   // because certain const functions need to call it.
   if (lastWasWrite) {
      const_cast<QFileDevicePrivate *>(this)->lastWasWrite = false;

      if (! const_cast<QFileDevice *>(q_func())->flush()) {
         return false;
      }
   }

   return true;
}

#endif
