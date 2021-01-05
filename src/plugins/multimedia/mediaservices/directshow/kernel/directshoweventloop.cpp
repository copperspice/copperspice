/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <directshoweventloop.h>

#include <qcoreapplication.h>
#include <qcoreevent.h>

class DirectShowPostedEvent
{
 public:
   DirectShowPostedEvent(QObject *receiver, QEvent *event)
      : receiver(receiver)
      , event(event)
      , next(0) {
   }

   ~DirectShowPostedEvent() {
      delete event;
   }

   QObject *receiver;
   QEvent *event;
   DirectShowPostedEvent *next;
};

DirectShowEventLoop::DirectShowEventLoop(QObject *parent)
   : QObject(parent)
   , m_postsHead(0)
   , m_postsTail(0)
   , m_eventHandle(::CreateEvent(0, 0, 0, 0))
   , m_waitHandle(::CreateEvent(0, 0, 0, 0))
{
}

DirectShowEventLoop::~DirectShowEventLoop()
{
   ::CloseHandle(m_eventHandle);
   ::CloseHandle(m_waitHandle);

   for (DirectShowPostedEvent *post = m_postsHead; post; post = m_postsHead) {
      m_postsHead = m_postsHead->next;

      delete post;
   }
}

void DirectShowEventLoop::wait(QMutex *mutex)
{
   ::ResetEvent(m_waitHandle);

   mutex->unlock();

   HANDLE handles[] = { m_eventHandle, m_waitHandle };
   while (::WaitForMultipleObjects(2, handles, false, INFINITE) == WAIT_OBJECT_0) {
      processEvents();
   }

   mutex->lock();
}

void DirectShowEventLoop::wake()
{
   ::SetEvent(m_waitHandle);
}

void DirectShowEventLoop::postEvent(QObject *receiver, QEvent *event)
{
   QMutexLocker locker(&m_mutex);

   DirectShowPostedEvent *post = new DirectShowPostedEvent(receiver, event);

   if (m_postsTail) {
      m_postsTail->next = post;
   } else {
      m_postsHead = post;
   }

   m_postsTail = post;

   QCoreApplication::postEvent(this, new QEvent(QEvent::User));
   ::SetEvent(m_eventHandle);
}

void DirectShowEventLoop::customEvent(QEvent *event)
{
   if (event->type() == QEvent::User) {
      processEvents();
   } else {
      QObject::customEvent(event);
   }
}

void DirectShowEventLoop::processEvents()
{
   QMutexLocker locker(&m_mutex);

   ::ResetEvent(m_eventHandle);

   while (m_postsHead) {
      DirectShowPostedEvent *post = m_postsHead;
      m_postsHead = m_postsHead->next;

      if (!m_postsHead) {
         m_postsTail = 0;
      }

      locker.unlock();
      QCoreApplication::sendEvent(post->receiver, post->event);
      delete post;
      locker.relock();
   }
}
