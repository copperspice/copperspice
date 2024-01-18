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

#ifndef QRUNNABLE_H
#define QRUNNABLE_H

#include <qglobal.h>

class QRunnable
{
 public:
   virtual void run() = 0;

   QRunnable()
      : ref(0)
   { }

   virtual ~QRunnable()
   { }

   bool autoDelete() const {
      return ref != -1;
   }

   void setAutoDelete(bool autoDelete) {
      ref = autoDelete ? 0 : -1;
   }

 private:
   int ref;

   friend class QThreadPool;
   friend class QThreadPoolPrivate;
   friend class QThreadPoolThread;
};

#endif
