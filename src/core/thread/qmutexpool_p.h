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

#ifndef QMUTEXPOOL_P_H
#define QMUTEXPOOL_P_H

#include <QtCore/qatomic.h>
#include <QtCore/qmutex.h>
#include <QtCore/qvarlengtharray.h>

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QMutexPool
{
 public:
   explicit QMutexPool(QMutex::RecursionMode recursionMode = QMutex::NonRecursive, int size = 131);
   ~QMutexPool();

   inline QMutex *get(const void *address) {
      int index = uint(quintptr(address)) % mutexes.count();
      QMutex *m = mutexes[index].load();
      if (m) {
         return m;
      } else {
         return createMutex(index);
      }
   }
   static QMutexPool *instance();
   static QMutex *globalInstanceGet(const void *address);

 private:
   QMutex *createMutex(int index);
   QVarLengthArray<QAtomicPointer<QMutex>, 131> mutexes;
   QMutex::RecursionMode recursionMode;
};

extern Q_CORE_EXPORT QMutexPool *qt_global_mutexpool;

QT_END_NAMESPACE

#endif // QMUTEXPOOL_P_H
