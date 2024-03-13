/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <qmutex.h>
#include <qthread.h>
#include <qwaitcondition.h>

#include <cs_catch2.h>

TEST_CASE("QThread traits", "[qthread]")
{
   REQUIRE(std::is_copy_constructible_v<QThread> == false);
   REQUIRE(std::is_move_constructible_v<QThread> == false);

   REQUIRE(std::is_copy_assignable_v<QThread> == false);
   REQUIRE(std::is_move_assignable_v<QThread> == false);

   REQUIRE(std::has_virtual_destructor_v<QThread> == true);
}

class Current_Thread : public QThread
{
   public:
      Qt::HANDLE id;
      QThread *thread;

      void run() override {
         id = QThread::currentThreadId();
         thread = QThread::currentThread();
      }
};

class Mutex_Thread : public QThread
{
   public:
      QMutex mutex;
      QWaitCondition cond;

      void run() override {
        QMutexLocker locker(&mutex);
        cond.wakeOne();
      }
};

TEST_CASE("QThread finished_running", "[qthread]")
{
   Mutex_Thread thread;

   REQUIRE(thread.isRunning() == false);
   REQUIRE(thread.isFinished() == false);

   QMutexLocker locker(&thread.mutex);
   thread.start();

   REQUIRE(thread.isRunning() == true);
   REQUIRE(thread.isFinished() == false);

   thread.cond.wait(locker.mutex());
   bool result = thread.wait(30000);

   REQUIRE(result == true);

   REQUIRE(thread.isRunning() ==  false );
   REQUIRE(thread.isFinished() == true);
}

TEST_CASE("QThread set_priority", "[qthread]")
{
   Mutex_Thread thread;

   REQUIRE(thread.priority() == QThread::InheritPriority);

   QMutexLocker locker(&thread.mutex);
   thread.start();

   // change the priority of a running thread
   thread.setPriority(QThread::IdlePriority);
   REQUIRE(thread.priority() == QThread::IdlePriority);

   thread.setPriority(QThread::NormalPriority);
   REQUIRE(thread.priority() == QThread::NormalPriority);

   thread.setPriority(QThread::HighPriority);
   REQUIRE(thread.priority() == QThread::HighPriority);

   thread.cond.wait(locker.mutex());
   REQUIRE(thread.wait(30000) == true);
}

TEST_CASE("QThread thread_count", "[qthread]")
{
   REQUIRE(QThread::idealThreadCount() > 0);
}

TEST_CASE("QThread thread_id", "[qthread]")
{
   Current_Thread thread;

   thread.id     = nullptr;
   thread.thread = nullptr;
   thread.start();

   REQUIRE(thread.wait(30000) == true);

   REQUIRE(thread.id != nullptr);
   REQUIRE(thread.id != QThread::currentThreadId());

   REQUIRE(thread.thread == static_cast<QThread *>(&thread));
}

