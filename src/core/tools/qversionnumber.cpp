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

#include <qversionnumber.h>

#include <qdatastream.h>
#include <qdebug.h>
#include <qhash.h>

#include <qlocale_tools_p.h>

#include <algorithm>
#include <limits>

QVector<int> QVersionNumber::segments() const
{
   return m_segments.pointer_segments;
}

QVersionNumber QVersionNumber::normalized() const
{
   int i;

   for (i = m_segments.size(); i; --i)
      if (m_segments.at(i - 1) != 0) {
         break;
      }

   QVersionNumber result(*this);
   result.m_segments.resize(i);

   return result;
}

bool QVersionNumber::isPrefixOf(const QVersionNumber &other) const
{
   if (segmentCount() > other.segmentCount()) {
      return false;
   }

   for (int i = 0; i < segmentCount(); ++i) {
      if (segmentAt(i) != other.segmentAt(i)) {
         return false;
      }
   }

   return true;
}

int QVersionNumber::compare(const QVersionNumber &v1, const QVersionNumber &v2)
{
   int commonlen = qMin(v1.segmentCount(), v2.segmentCount());

   for (int i = 0; i < commonlen; ++i) {
      if (v1.segmentAt(i) != v2.segmentAt(i)) {
         return v1.segmentAt(i) - v2.segmentAt(i);
      }
   }

   // ran out of segments in v1 and/or v2 and need to check the first trailing
   // segment to finish the compare
   if (v1.segmentCount() > commonlen) {
      // v1 is longer
      if (v1.segmentAt(commonlen) != 0) {
         return v1.segmentAt(commonlen);
      } else {
         return 1;
      }

   } else if (v2.segmentCount() > commonlen) {
      // v2 is longer
      if (v2.segmentAt(commonlen) != 0) {
         return -v2.segmentAt(commonlen);
      } else {
         return -1;
      }
   }

   // the two version numbers are the same
   return 0;
}

QVersionNumber QVersionNumber::commonPrefix(const QVersionNumber &v1, const QVersionNumber &v2)
{
   int commonlen = qMin(v1.segmentCount(), v2.segmentCount());
   int i;

   for (i = 0; i < commonlen; ++i) {
      if (v1.segmentAt(i) != v2.segmentAt(i)) {
         break;
      }
   }

   if (i == 0) {
      return QVersionNumber();
   }

   QVersionNumber result(v1);
   result.m_segments.resize(i);

   return result;
}

QString QVersionNumber::toString() const
{
   QString version;

   bool first = true;

   for (int i = 0; i < segmentCount(); ++i) {
      if (! first) {
         version += QLatin1Char('.');
      }

      version += QString::number(segmentAt(i));
      first = false;
   }

   return version;
}

QVersionNumber QVersionNumber::fromString(const QString &string, int *suffixIndex)
{
   QVector<int> seg;

   const QByteArray cString(string.toLatin1());

   const char *start = cString.constData();
   const char *end   = start;
   const char *lastGoodEnd = start;
   const char *endOfString = cString.constData() + cString.size();

   do {
      bool ok = false;
      const quint64 value = qstrtoull(start, &end, 10, &ok);

      if (! ok || value > quint64(std::numeric_limits<int>::max())) {
         break;
      }

      seg.append(int(value));
      start = end + 1;
      lastGoodEnd = end;

   } while (start < endOfString && (end < endOfString && *end == '.'));

   if (suffixIndex) {
      *suffixIndex = int(lastGoodEnd - cString.constData());
   }

   return QVersionNumber(std::move(seg));
}

void QVersionNumber::SegmentStorage::setVector(int len, int maj, int min, int mic)
{
   pointer_segments.clear();
   pointer_segments.resize(len);
   pointer_segments.data()[0] = maj;

   if (len > 1) {
      pointer_segments.data()[1] = min;

      if (len > 2) {
         pointer_segments.data()[2] = mic;
      }
   }
}

QDataStream &operator<<(QDataStream &stream, const QVersionNumber &version)
{
   stream << version.segments();
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QVersionNumber &version)
{

   version.m_segments.pointer_segments.clear();
   stream >> version.m_segments.pointer_segments;

   return stream;
}

QDebug operator<<(QDebug debug, const QVersionNumber &version)
{
   QDebugStateSaver saver(debug);
   debug.noquote() << version.toString();

   return debug;
}

uint qHash(const QVersionNumber &key, uint seed)
{
   for (int i = 0; i < key.segmentCount(); ++i) {
      seed = qHash(key.segmentAt(i), seed);
   }

   return seed;
}
