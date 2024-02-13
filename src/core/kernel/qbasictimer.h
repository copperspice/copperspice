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

#ifndef QBASICTIMER_H
#define QBASICTIMER_H

#include <qglobal.h>
#include <qnamespace.h>

class QObject;

class Q_CORE_EXPORT QBasicTimer
{
 public:
   QBasicTimer()
      : id(0)
   { }

   ~QBasicTimer() {
      if (id) {
         stop();
      }
   }

   bool isActive() const {
      return id != 0;
   }

   int timerId() const {
      return id;
   }

   void start(int msec, QObject *object);
   void start(int msec, Qt::TimerType timerType, QObject *object);
   void stop();

 private:
   int id;
};

#endif
