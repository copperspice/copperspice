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

#include <qplatform_graphicsbuffer.h>

#include <qopenglcontext.h>
#include <qopenglfunctions.h>
#include <qopengl.h>
#include <qdebug.h>

QPlatformGraphicsBuffer::QPlatformGraphicsBuffer(const QSize &size, const QPixelFormat &format)
   : m_size(size), m_format(format)
{
}

QPlatformGraphicsBuffer::~QPlatformGraphicsBuffer()
{
}

bool QPlatformGraphicsBuffer::bindToTexture(const QRect &rect) const
{
   (void) rect;
   return false;
}

bool QPlatformGraphicsBuffer::lock(AccessTypes access, const QRect &rect)
{
   bool locked = doLock(access, rect);

   if (locked) {
      m_lock_access |= access;
   }

   return locked;
}

void QPlatformGraphicsBuffer::unlock()
{
   if (m_lock_access == None) {
      return;
   }

   AccessTypes previous = m_lock_access;
   doUnlock();
   m_lock_access = None;

   emit unlocked(previous);
}

const uchar *QPlatformGraphicsBuffer::data() const
{
   return nullptr;
}

uchar *QPlatformGraphicsBuffer::data()
{
   return nullptr;
}

int QPlatformGraphicsBuffer::byteCount() const
{
   Q_ASSERT(isLocked() & SWReadAccess);
   return size().height() * bytesPerLine();
}

int QPlatformGraphicsBuffer::bytesPerLine() const
{
   return 0;
}

QPlatformGraphicsBuffer::Origin QPlatformGraphicsBuffer::origin() const
{
   return OriginTopLeft;
}

