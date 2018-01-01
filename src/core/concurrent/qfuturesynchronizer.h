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

#ifndef QFutureSynchronizer_H
#define QFutureSynchronizer_H

#include <QtCore/qfuture.h>

QT_BEGIN_NAMESPACE

template <typename T>
class QFutureSynchronizer
{
   Q_DISABLE_COPY(QFutureSynchronizer)

 public:
   QFutureSynchronizer() : m_cancelOnWait(false) { }
   explicit QFutureSynchronizer(const QFuture<T> &future)
      : m_cancelOnWait(false) {
      addFuture(future);
   }
   ~QFutureSynchronizer()  {
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

   QList<QFuture<T> > futures() const {
      return m_futures;
   }

   void setCancelOnWait(bool enabled) {
      m_cancelOnWait = enabled;
   }

   bool cancelOnWait() const {
      return m_cancelOnWait;
   }

 protected:
   QList<QFuture<T> > m_futures;
   bool m_cancelOnWait;
};

QT_END_NAMESPACE

#endif // QFUTRUESYNCHRONIZER_H
