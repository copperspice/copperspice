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

#ifndef QTHREADPOOL_P_H
#define QTHREADPOOL_P_H

#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qset.h>
#include <QtCore/qqueue.h>

QT_BEGIN_NAMESPACE

class QThreadPoolThread;

class QThreadPoolPrivate
{
   Q_DECLARE_PUBLIC(QThreadPool)
   friend class QThreadPoolThread;

 public:
   QThreadPoolPrivate();
   virtual ~QThreadPoolPrivate() {}

   bool tryStart(QRunnable *task);
   void enqueueTask(QRunnable *task, int priority = 0);
   int activeThreadCount() const;

   void tryToStartMoreThreads();
   bool tooManyThreadsActive() const;

   void startThread(QRunnable *runnable = 0);
   void reset();
   bool waitForDone(int msecs = -1);
   bool startFrontRunnable();
   void stealRunnable(QRunnable *);

   mutable QMutex mutex;
   QSet<QThreadPoolThread *> allThreads;
   QQueue<QThreadPoolThread *> waitingThreads;
   QQueue<QThreadPoolThread *> expiredThreads;
   QList<QPair<QRunnable *, int> > queue;
   QWaitCondition noActiveThreads;

   bool isExiting;
   int expiryTimeout;
   int maxThreadCount;
   int reservedThreads;
   int activeThreads;

 protected:
   QThreadPool *q_ptr;

};

QT_END_NAMESPACE

#endif
