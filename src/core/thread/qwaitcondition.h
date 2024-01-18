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

#ifndef QWAITCONDITION_H
#define QWAITCONDITION_H

#include <qglobal.h>

class QWaitConditionPrivate;
class QMutex;
class QReadWriteLock;

#include <limits.h>

class Q_CORE_EXPORT QWaitCondition
{
 public:
   QWaitCondition();

   QWaitCondition(const QWaitCondition &) = delete;
   QWaitCondition &operator=(const QWaitCondition &) = delete;

   ~QWaitCondition();

   bool wait(QMutex *mutex, unsigned long time = ULONG_MAX);
   bool wait(QReadWriteLock *readWriteLock, unsigned long time = ULONG_MAX);

   void wakeOne();
   void wakeAll();

 private:
   QWaitConditionPrivate *d;
};

#endif
