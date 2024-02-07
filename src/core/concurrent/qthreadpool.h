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

#ifndef QTHREADPOOL_H
#define QTHREADPOOL_H

#include <qglobal.h>
#include <qrunnable.h>
#include <qscopedpointer.h>
#include <qthread.h>

class QThreadPoolPrivate;

class Q_CORE_EXPORT QThreadPool : public QObject
{
   CORE_CS_OBJECT(QThreadPool)

   Q_DECLARE_PRIVATE(QThreadPool)

   CORE_CS_PROPERTY_READ(expiryTimeout, expiryTimeout)
   CORE_CS_PROPERTY_WRITE(expiryTimeout, setExpiryTimeout)

   CORE_CS_PROPERTY_READ(maxThreadCount, maxThreadCount)
   CORE_CS_PROPERTY_WRITE(maxThreadCount, setMaxThreadCount)

   CORE_CS_PROPERTY_READ(activeThreadCount, activeThreadCount)

   friend class QFutureInterfaceBase;

 public:
   QThreadPool(QObject *parent = nullptr);
   ~QThreadPool();

   static QThreadPool *globalInstance();

   void start(QRunnable *runnable, int priority = 0);
   bool tryStart(QRunnable *runnable);

   int expiryTimeout() const;
   void setExpiryTimeout(int expiryTimeout);

   int maxThreadCount() const;
   void setMaxThreadCount(int maxThreadCount);

   int activeThreadCount() const;

   void reserveThread();
   void releaseThread();

   bool waitForDone(int msecs = -1);

   void clear();
   void cancel(QRunnable *runnable);

 protected:
   QScopedPointer<QThreadPoolPrivate> d_ptr;

};

#endif
