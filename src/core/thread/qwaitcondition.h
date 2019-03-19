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

#ifndef QWAITCONDITION_H
#define QWAITCONDITION_H

#include <QtCore/qglobal.h>
#include <limits.h>

QT_BEGIN_NAMESPACE

class QWaitConditionPrivate;
class QMutex;
class QReadWriteLock;

class Q_CORE_EXPORT QWaitCondition
{
 public:
   QWaitCondition();
   ~QWaitCondition();

   bool wait(QMutex *mutex, unsigned long time = ULONG_MAX);
   bool wait(QReadWriteLock *readWriteLock, unsigned long time = ULONG_MAX);

   void wakeOne();
   void wakeAll();

 private:
   Q_DISABLE_COPY(QWaitCondition)

   QWaitConditionPrivate *d;
};

QT_END_NAMESPACE

#endif // QWAITCONDITION_H
