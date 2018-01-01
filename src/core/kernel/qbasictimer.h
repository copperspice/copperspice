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

#ifndef QBASICTIMER_H
#define QBASICTIMER_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QObject;

class Q_CORE_EXPORT QBasicTimer
{
   int id;

 public:
   inline QBasicTimer() : id(0) {}
   inline ~QBasicTimer() {
      if (id) {
         stop();
      }
   }

   inline bool isActive() const {
      return id != 0;
   }

   inline int timerId() const {
      return id;
   }

   void start(int msec, QObject *obj);
   void stop();
};
Q_DECLARE_TYPEINFO(QBasicTimer, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

#endif // QBASICTIMER_H
