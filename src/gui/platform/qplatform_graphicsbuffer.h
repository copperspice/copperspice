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

#ifndef QPLATFORM_GRAPHICSBUFFER_H
#define QPLATFORM_GRAPHICSBUFFER_H

#include <qsize.h>
#include <qrect.h>
#include <qpixelformat.h>
#include <qflags.h>
#include <qobject.h>

class Q_GUI_EXPORT QPlatformGraphicsBuffer : public QObject
{
   GUI_CS_OBJECT(QPlatformGraphicsBuffer)

 public:
   enum AccessType {
      None                = 0x00,
      SWReadAccess        = 0x01,
      SWWriteAccess       = 0x02,
      TextureAccess       = 0x04,
      HWCompositor        = 0x08
   };
   using AccessTypes = QFlags<AccessType>;

   enum Origin {
      OriginBottomLeft,
      OriginTopLeft
   };

   virtual ~QPlatformGraphicsBuffer();

   AccessTypes isLocked() const {
      return m_lock_access;
   }
   bool lock(AccessTypes access, const QRect &rect = QRect());
   void unlock();

   virtual bool bindToTexture(const QRect &rect = QRect()) const;

   virtual const uchar *data() const;
   virtual uchar *data();
   virtual int bytesPerLine() const;
   int byteCount() const;

   virtual Origin origin() const;

   QSize size() const {
      return m_size;
   }
   QPixelFormat format() const {
      return m_format;
   }

   GUI_CS_SIGNAL_1(Public, void unlocked(AccessTypes previousAccessTypes))
   GUI_CS_SIGNAL_2(unlocked, previousAccessTypes)

 protected:
   QPlatformGraphicsBuffer(const QSize &size, const QPixelFormat &format);

   virtual bool doLock(AccessTypes access, const QRect &rect = QRect()) = 0;
   virtual void doUnlock() = 0;

 private:
   QSize m_size;
   QPixelFormat m_format;
   AccessTypes m_lock_access;
};

#endif
