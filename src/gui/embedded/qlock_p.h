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

#ifndef QLOCK_P_H
#define QLOCK_P_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QLockData;

class Q_GUI_EXPORT QLock
{
 public:
   QLock(const QString &filename, char id, bool create = false);
   ~QLock();

   enum Type { Read, Write };

   bool isValid() const;
   void lock(Type type);
   void unlock();
   bool locked() const;

 private:
   Type type;
   QLockData *data;
};


// Nice class for ensuring the lock is released.
// Just create one on the stack and the lock is automatically released
// when QLockHandle is destructed.
class Q_GUI_EXPORT QLockHandle
{
 public:
   QLockHandle(QLock *l, QLock::Type type) : qlock(l) {
      qlock->lock(type);
   }
   ~QLockHandle() {
      if (locked()) {
         qlock->unlock();
      }
   }

   void lock(QLock::Type type) {
      qlock->lock(type);
   }
   void unlock() {
      qlock->unlock();
   }
   bool locked() const {
      return qlock->locked();
   }

 private:
   QLock *qlock;
};

QT_END_NAMESPACE

#endif // QLOCK_P_H
