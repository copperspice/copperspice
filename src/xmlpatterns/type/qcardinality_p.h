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

#ifndef QCardinality_P_H
#define QCardinality_P_H

#include <qglobal.h>
#include <qassert.h>
#include <qstringfwd.h>

namespace QPatternist {

class Cardinality
{
 public:
   typedef qint32 Count;

   enum CustomizeDisplayName {
      IncludeExplanation  = 1,
      ExcludeExplanation
   };

   Cardinality(const Cardinality &other)
      : m_min(other.m_min), m_max(other.m_max)
   { }

   Cardinality()
      : m_min(-1), m_max(0)
   { }

   static Cardinality empty() {
      return Cardinality(0, 0);
   }

   static Cardinality exactlyOne() {
      return Cardinality(1, 1);
   }

   static Cardinality zeroOrOne() {
      return Cardinality(0, 1);
   }

   static Cardinality zeroOrMore() {
      return Cardinality(0, -1);
   }

   static Cardinality oneOrMore() {
      return Cardinality(1, -1);
   }

   static Cardinality twoOrMore() {
      return Cardinality(2, -1);
   }

   static Cardinality fromCount(const Count count) {
      Q_ASSERT_X(count > -1, Q_FUNC_INFO, "Count smaller than 0 is not allowed");
      return Cardinality(count, count);
   }

   static Cardinality fromRange(const Count minimum, const Count maximum) {
      Q_ASSERT_X(minimum > -1, Q_FUNC_INFO, "Minimum should never be less than 0");
      Q_ASSERT_X(minimum <= maximum || maximum == -1, Q_FUNC_INFO, "Minimum can not be larger than maximum");

      return Cardinality(minimum, maximum);
   }

   static Cardinality fromExact(const Count count) {
      Q_ASSERT(count >= 0);
      return Cardinality(count, count);
   }

   Count minimum() const {
      Q_ASSERT_X(m_min != -1, Q_FUNC_INFO, "The cardinality are invalid.");
      return m_min;
   }

   Count maximum() const {
      Q_ASSERT_X(m_min != -1, Q_FUNC_INFO, "The cardinality are invalid.");
      return m_max;
   }

   bool allowsMany() const {
      Q_ASSERT_X(m_min != -1, Q_FUNC_INFO, "The cardinality are invalid.");
      return m_max == -1 || m_max > 1;
   }

   bool allowsEmpty() const {
      Q_ASSERT_X(m_min != -1, Q_FUNC_INFO, "The cardinality are invalid.");
      return m_min == 0;
   }

   Cardinality toWithoutMany() const {
      return m_min == 0 ? Cardinality(0, 1) : Cardinality(1, 1);
   }

   bool isMatch(const Cardinality &other) const {
      Q_ASSERT_X(m_min != -1 && other.m_min != -1, Q_FUNC_INFO, "One of the cardinalities are invalid.");

      if (other.m_min < m_min) {
         return false;

      } else {
         // we now know the minimum will always be ok.

         if (m_max == -1) {
            return true;                     // allow infinite so anything can match
         } else if (other.m_max == -1) {
            return false;                    // other allows infinity while we do not
         } else {
            return m_max >= other.m_max;
         }
      }
   }

   bool canMatch(const Cardinality &other) const {
      Q_ASSERT_X(m_min != -1 && other.m_min != -1, Q_FUNC_INFO, "One of the cardinalities are invalid.");

      if (m_max == -1) {
         return m_min <= other.m_min || other.m_max >= m_min || other.m_max == -1;
      } else {
         if (m_max == other.m_min) {
            return true;
         } else if (m_max > other.m_min) {
            return other.m_max >= m_min || other.m_max == -1;
         } else {
            // m_max < other.m_min
            return false;
         }
      }
   }

   bool isEmpty() const {
      Q_ASSERT_X(m_min != -1, Q_FUNC_INFO, "Cardinality is invalid");
      return m_min == 0 && m_max == 0;
   }

   bool isZeroOrOne() const {
      Q_ASSERT_X(m_min != -1, Q_FUNC_INFO, "Cardinality is invalid");
      return m_min == 0 && m_max == 1;
   }

   bool isExactlyOne() const {
      Q_ASSERT_X(m_min != -1, Q_FUNC_INFO, "Cardinality is invalid");
      return m_min == 1 && m_max == 1;
   }

   bool isOneOrMore() const {
      Q_ASSERT_X(m_min != -1, Q_FUNC_INFO, "Cardinality is invalid");
      return m_min > 0 && (m_max == -1 || m_max >= 1);
   }

   bool isExact() const {
      Q_ASSERT_X(m_min != -1, Q_FUNC_INFO, "Cardinality is invalid");
      return m_min == m_max;
   }

   QString displayName(const CustomizeDisplayName explanation) const;

   Cardinality operator|(const Cardinality &other) const {
      Q_ASSERT_X(m_min != -1 && other.m_min != -1, Q_FUNC_INFO, "One of the cardinalities are invalid");
      if (m_max == -1 || other.m_max == -1) {
         return Cardinality(qMin(m_min, other.m_min), -1);
      } else {
         return Cardinality(qMin(m_min, other.m_min), qMax(m_max, other.m_max));
      }
   }

   Cardinality &operator|=(const Cardinality &other) {
      Q_ASSERT_X(m_min != -1 && other.m_min != -1, Q_FUNC_INFO, "One of the cardinalities are invalid");
      m_min = qMin(m_min, other.m_min);

      if (m_max == -1) {
         return *this;
      } else if (other.m_max == -1) {
         m_max = -1;
      } else {
         m_max = qMax(m_max, other.m_max);
      }

      return *this;
   }

   Cardinality operator&(const Cardinality &other) const {
      Q_ASSERT_X(m_min != -1 && other.m_min != -1, Q_FUNC_INFO, "One of the cardinalities are invalid");

      if (m_max < other.m_min) { /* No intersection. */
         return empty();
      }

      const Count min = qMax(m_min, other.m_min);

      if (m_max == -1) {
         return Cardinality(min, other.m_max);
      } else if (other.m_max == -1) {
         return Cardinality(min, m_max);
      } else {
         return Cardinality(min, qMin(m_max, other.m_max));
      }
   }

   Cardinality operator+(const Cardinality &other) const {
      Q_ASSERT_X(m_min != -1 && other.m_min != -1, Q_FUNC_INFO, "One of the cardinalities are invalid");
      if (m_max == -1 || other.m_max == -1) {
         return Cardinality(m_min + other.m_min, -1);
      } else {
         return Cardinality(m_min + other.m_min, m_max + other.m_max);
      }
   }

   Cardinality &operator+=(const Cardinality &other) {
      Q_ASSERT_X(m_min != -1 && other.m_min != -1, Q_FUNC_INFO, "One of the cardinalities are invalid");
      m_min += other.m_min;

      if (m_max == -1) {
         return *this;
      }
      if (other.m_max == -1) {
         m_max = -1;
      } else {
         m_max += other.m_max;
      }

      return *this;
   }

   Cardinality operator*(const Cardinality &other) const {
      Q_ASSERT_X(m_min != -1 && other.m_min != -1, Q_FUNC_INFO, "One of the cardinalities are invalid");

      if (m_max == -1 || other.m_max == -1) {
         return Cardinality(m_min * other.m_min, -1);
      } else {
         return Cardinality(m_min * other.m_min, m_max * other.m_max);
      }
   }

   Cardinality &operator=(const Cardinality &other) {
      Q_ASSERT_X(this != &other, Q_FUNC_INFO, "Assigning to oneself is not allowed");
      m_min = other.m_min;
      m_max = other.m_max;
      return *this;
   }

   bool operator==(const Cardinality &other) const {
      return m_min == other.m_min &&
             m_max == other.m_max;
   }

   bool operator!=(const Cardinality &other) const {
      return m_min != other.m_min ||
             m_max != other.m_max;
   }

 private:
   Cardinality(const Count min, const Count max)
      : m_min(min), m_max(max) {
   }

   Count m_min;
   Count m_max;
};

}

#endif
