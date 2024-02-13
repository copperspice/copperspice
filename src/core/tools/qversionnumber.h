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

#ifndef QVERSIONNUMBER_H
#define QVERSIONNUMBER_H

#include <qnamespace.h>
#include <qstring.h>
#include <qvector.h>

class QVersionNumber;

Q_CORE_EXPORT uint qHash(const QVersionNumber &key, uint seed = 0);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QVersionNumber &version);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QVersionNumber &version);

class QVersionNumber
{
   struct SegmentStorage {
      QVector<int> pointer_segments;

      // set the InlineSegmentMarker and set length to zero
      SegmentStorage()
      {}

      SegmentStorage(const QVector<int> &seg) : pointer_segments(seg) {
      }

      explicit SegmentStorage(QVector<int> &&seg) : pointer_segments(std::move(seg)) {

      }

      SegmentStorage(std::initializer_list<int> args) : pointer_segments(args) {

      }

      int size() const {
         return pointer_segments.size();
      }

      void resize(int len) {
         pointer_segments.resize(len);
      }

      int at(int index) const {
         return pointer_segments.at(index);
      }

      void setSegments(int len, int maj, int min = 0, int mic = 0) {
         setVector(len, maj, min, mic);
      }

    private:
      Q_CORE_EXPORT void setVector(int len, int maj, int min, int mic);

   } m_segments;

 public:
   QVersionNumber() : m_segments()
   { }

   explicit QVersionNumber(const QVector<int> &seg) : m_segments(seg)
   { }

   explicit QVersionNumber(QVector<int> &&seg)
      : m_segments(std::move(seg))
   { }

   QVersionNumber(std::initializer_list<int> args)
      : m_segments(args)
   { }

   explicit QVersionNumber(int maj) {
      m_segments.setSegments(1, maj);
   }

   explicit QVersionNumber(int maj, int min) {
      m_segments.setSegments(2, maj, min);
   }

   explicit QVersionNumber(int maj, int min, int mic) {
      m_segments.setSegments(3, maj, min, mic);
   }

   bool isNull() const {
      return segmentCount() == 0;
   }

   bool isNormalized() const {
      return isNull() || segmentAt(segmentCount() - 1) != 0;
   }

   int majorVersion() const {
      return segmentAt(0);
   }

   int minorVersion() const {
      return segmentAt(1);
   }

   int microVersion() const {
      return segmentAt(2);
   }

   Q_CORE_EXPORT QVersionNumber normalized() const;

   Q_CORE_EXPORT QVector<int> segments() const;

   int segmentAt(int index) const {
      return (m_segments.size() > index) ? m_segments.at(index) : 0;
   }

   int segmentCount() const {
      return m_segments.size();
   }

   Q_CORE_EXPORT bool isPrefixOf(const QVersionNumber &other) const;

   Q_CORE_EXPORT static int compare(const QVersionNumber &v1, const QVersionNumber &v2);

   Q_CORE_EXPORT static QVersionNumber commonPrefix(const QVersionNumber &v1, const QVersionNumber &v2);

   Q_CORE_EXPORT QString toString() const;
   Q_CORE_EXPORT static QVersionNumber fromString(const QString &string, int *suffixIndex = nullptr);

 private:
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QVersionNumber &version);
   friend Q_CORE_EXPORT uint qHash(const QVersionNumber &key, uint seed);
};

Q_CORE_EXPORT QDebug operator<<(QDebug, const QVersionNumber &version);

inline bool operator> (const QVersionNumber &lhs, const QVersionNumber &rhs)
{
   return QVersionNumber::compare(lhs, rhs) > 0;
}

inline bool operator>=(const QVersionNumber &lhs, const QVersionNumber &rhs)
{
   return QVersionNumber::compare(lhs, rhs) >= 0;
}

inline bool operator< (const QVersionNumber &lhs, const QVersionNumber &rhs)
{
   return QVersionNumber::compare(lhs, rhs) < 0;
}

inline bool operator<=(const QVersionNumber &lhs, const QVersionNumber &rhs)
{
   return QVersionNumber::compare(lhs, rhs) <= 0;
}

inline bool operator==(const QVersionNumber &lhs, const QVersionNumber &rhs)
{
   return QVersionNumber::compare(lhs, rhs) == 0;
}

inline bool operator!=(const QVersionNumber &lhs, const QVersionNumber &rhs)
{
   return QVersionNumber::compare(lhs, rhs) != 0;
}

#endif
