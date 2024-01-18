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

#ifndef QWIN_THREADPOOLRUNNER_H
#define QWIN_THREADPOOLRUNNER_H

#include <qmutex.h>
#include <qrunnable.h>
#include <qthreadpool.h>
#include <qwaitcondition.h>

class QWindowsThreadPoolRunner
{
   // nested class implementing QRunnable to execute a function
   template <class RunnableFunction>
   class Runnable : public QRunnable
   {
      public:
         explicit Runnable(QMutex *m, QWaitCondition *c, RunnableFunction f)
            : m_mutex(m), m_condition(c), m_function(f)
         {
         }

         void run() override {
            m_function();
            m_mutex->lock();
            m_condition->wakeAll();
            m_mutex->unlock();
         }

      private:
         QMutex *m_mutex;
         QWaitCondition *m_condition;
         RunnableFunction m_function;
   };

   public:
      QWindowsThreadPoolRunner()
      {
      }

      QWindowsThreadPoolRunner(const QWindowsThreadPoolRunner &) = delete;
      QWindowsThreadPoolRunner &operator=(const QWindowsThreadPoolRunner &) = delete;

      template <class Function>
      bool run(Function f, unsigned long timeOutMSecs = 5000) {
         QThreadPool *pool = QThreadPool::globalInstance();
         Q_ASSERT(pool);

         Runnable<Function> *runnable = new Runnable<Function>(&m_mutex, &m_condition, f);
         m_mutex.lock();
         pool->start(runnable);
         const bool ok = m_condition.wait(&m_mutex, timeOutMSecs);
         m_mutex.unlock();

         if (!ok) {
            pool->cancel(runnable);
         }

         return ok;
      }

   private:
      QMutex m_mutex;
      QWaitCondition m_condition;
};

#endif
