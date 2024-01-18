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

#ifndef QAUDIOBUFFER_P_H
#define QAUDIOBUFFER_P_H

#include <qstring.h>
#include <qmultimedia.h>
#include <qaudioformat.h>

class Q_MULTIMEDIA_EXPORT QAbstractAudioBuffer
{
 public:
   virtual ~QAbstractAudioBuffer() {}

   // Lifetime management
   virtual void release() = 0;

   // Format related
   virtual QAudioFormat format() const = 0;
   virtual qint64 startTime() const = 0;
   virtual int frameCount() const = 0;

   // R/O Data
   virtual void *constData() const = 0;

   // For writable data we do this:
   // If we only have one reference to the provider,
   // call writableData().  If that does not return 0,
   // then we're finished.  If it does return 0, then we call
   // writableClone() to get a new buffer and then release
   // the old clone if that succeeds.  If it fails, we create
   // a memory clone from the constData and release the old buffer.
   // If writableClone() succeeds, we then call writableData() on it
   // and that should be good.

   virtual void *writableData() = 0;
   virtual QAbstractAudioBuffer *clone() const = 0;
};

#endif
