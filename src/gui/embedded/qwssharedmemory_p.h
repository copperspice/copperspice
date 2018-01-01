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

#ifndef QWSSHAREDMEMORY_P_H
#define QWSSHAREDMEMORY_P_H

#include <qplatformdefs.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_QWS_MULTIPROCESS)

class QWSSharedMemory
{
 public:
   QWSSharedMemory();
   ~QWSSharedMemory();

   bool create(int size);
   bool attach(int id);
   void detach();

   int id() const {
      return shmId;
   }

   void *address() const {
      return shmBase;
   }
   int size() const;

 private:
   int shmId;
   void *shmBase;
   mutable int shmSize;

#ifdef QT_POSIX_IPC
   int hand;
#endif
};

#endif // QT_NO_QWS_MULTIPROCESS

QT_END_NAMESPACE

#endif // QWSSHAREDMEMORY_P_H
