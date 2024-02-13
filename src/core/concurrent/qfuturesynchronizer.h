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

#ifndef QFutureSynchronizer_H
#define QFutureSynchronizer_H

#include <qfuture.h>

template <typename T>
class QFutureSynchronizer
{
 public:
   QFutureSynchronizer()
      : m_cancelOnWait(false)
   {
   }

   explicit QFutureSynchronizer(const QFuture<T> &future)
      : m_cancelOnWait(false)
   {
      addFuture(future);
   }

   QFutureSynchronizer(const QFutureSynchronizer &) = delete;
   QFutureSynchronizer &operator=(const QFutureSynchronizer &) = delete;

   ~QFutureSynchronizer()
   {
      waitForFinished();
   }

   void setFuture(const QFuture<T> &future) {
      waitForFinished();
      m_futures.clear();
      addFuture(future);
   }

   void addFuture(const QFuture<T> &future) {
      m_futures.append(future);
   }

   void waitForFinished() {
      if (m_cancelOnWait) {
         for (int i = 0; i < m_futures.count(); ++i) {
            m_futures[i].cancel();
         }
      }

      for (int i = 0; i < m_futures.count(); ++i) {
         m_futures[i].waitForFinished();
      }
   }

   void clearFutures() {
      m_futures.clear();
   }

   QList<QFuture<T>> futures() const {
      return m_futures;
   }

   void setCancelOnWait(bool enabled) {
      m_cancelOnWait = enabled;
   }

   bool cancelOnWait() const {
      return m_cancelOnWait;
   }

 protected:
   QList<QFuture<T>> m_futures;
   bool m_cancelOnWait;
};

#endif
