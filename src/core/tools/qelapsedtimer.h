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

#ifndef QElapsedTimer_H
#define QElapsedTimer_H

#include <qglobal.h>

#include <limits>

class Q_CORE_EXPORT QElapsedTimer
{
 public:
   enum ClockType {
      SystemTime,
      MonotonicClock,
      TickCounter,
      MachAbsoluteTime,
      PerformanceCounter
   };

   constexpr QElapsedTimer()
      : t1(std::numeric_limits<qint64>::lowest()), t2(std::numeric_limits<qint64>::lowest())
   {
   }

   static ClockType clockType();
   static bool isMonotonic();

   void start();
   qint64 restart();
   void invalidate();
   bool isValid() const;

   qint64 nsecsElapsed() const;
   qint64 elapsed() const;
   bool hasExpired(qint64 timeout) const;

   qint64 msecsSinceReference() const;
   qint64 msecsTo(const QElapsedTimer &other) const;
   qint64 secsTo(const QElapsedTimer &other) const;

   bool operator==(const QElapsedTimer &other) const {
      return t1 == other.t1 && t2 == other.t2;
   }

   bool operator!=(const QElapsedTimer &other) const {
      return !(*this == other);
   }

   friend bool Q_CORE_EXPORT operator<(const QElapsedTimer &v1, const QElapsedTimer &v2);

 private:
   qint64 t1;
   qint64 t2;
};

#endif
