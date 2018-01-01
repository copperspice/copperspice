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

#ifndef QTCONCURRENTRUNBASE_H
#define QTCONCURRENTRUNBASE_H

#include <QtCore/qglobal.h>
#include <QtCore/qfuture.h>
#include <QtCore/qrunnable.h>
#include <QtCore/qthreadpool.h>

QT_BEGIN_NAMESPACE

namespace QtConcurrent {

template <typename T>
struct SelectSpecialization {
   template <class Normal, class Void>
   struct Type {
      typedef Normal type;
   };
};

template <>
struct SelectSpecialization<void> {
   template <class Normal, class Void>
   struct Type {
      typedef Void type;
   };
};

template <typename T>
class RunFunctionTaskBase : public QFutureInterface<T> , public QRunnable
{
 public:
   QFuture<T> start() {
      this->setRunnable(this);
      this->reportStarted();
      QFuture<T> future = this->future();
      QThreadPool::globalInstance()->start(this, /*m_priority*/ 0);
      return future;
   }

   void run() override {}
   virtual void runFunctor() = 0;
};

template <typename T>
class RunFunctionTask : public RunFunctionTaskBase<T>
{
 public:
   void run() {
      if (this->isCanceled()) {
         this->reportFinished();
         return;
      }

      try {
         this->runFunctor();

      } catch (QtConcurrent::Exception &e) {
         QFutureInterface<T>::reportException(e);
      } catch (...) {
         QFutureInterface<T>::reportException(QtConcurrent::UnhandledException());
      }

      this->reportResult(result);
      this->reportFinished();
   }
   T result;
};

template <>
class RunFunctionTask<void> : public RunFunctionTaskBase<void>
{
 public:
   void run() override {
      if (this->isCanceled()) {
         this->reportFinished();
         return;
      }

      try {
         this->runFunctor();

      } catch (QtConcurrent::Exception &e) {
         QFutureInterface<void>::reportException(e);
      } catch (...) {
         QFutureInterface<void>::reportException(QtConcurrent::UnhandledException());
      }

      this->reportFinished();
   }
};

} //namespace QtConcurrent

QT_END_NAMESPACE

#endif
