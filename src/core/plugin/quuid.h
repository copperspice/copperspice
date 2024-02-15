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

#ifndef QUUID_H
#define QUUID_H

#include <qstring.h>

#if defined(Q_OS_WIN)

#ifndef GUID_DEFINED
#define GUID_DEFINED

struct _GUID {
   ulong   Data1;
   ushort  Data2;
   ushort  Data3;
   uchar   Data4[8];
};

using GUID    = _GUID;
using REFGUID = _GUID *;
using LPGUID  = _GUID *;

#endif

#endif

class Q_CORE_EXPORT QUuid
{
 public:
   enum Variant {
      VarUnknown        = -1,
      NCS               = 0, // 0 - -
      DCE               = 2, // 1 0 -
      Microsoft         = 6, // 1 1 0
      Reserved          = 7  // 1 1 1
   };

   enum Version {
      VerUnknown        = -1,
      Time              = 1, // 0 0 0 1
      EmbeddedPOSIX     = 2, // 0 0 1 0
      Md5               = 3, // 0 0 1 1
      Name = Md5,
      Random            = 4,  // 0 1 0 0
      Sha1              = 5 // 0 1 0 1
   };

   constexpr QUuid() : data1(0), data2(0), data3(0), data4{0, 0, 0, 0, 0, 0, 0, 0} {}

   constexpr QUuid(uint l, ushort w1, ushort w2, uchar b1, uchar b2, uchar b3,
         uchar b4, uchar b5, uchar b6, uchar b7, uchar b8)
      : data1(l), data2(w1), data3(w2), data4{b1, b2, b3, b4, b5, b6, b7, b8} {}

   QUuid(const QString &text);
   QUuid(const char *);

   QString toString() const;

   QUuid(const QByteArray &text);
   QByteArray toByteArray() const;

   QByteArray toRfc4122() const;
   static QUuid fromRfc4122(const QByteArray &bytes);
   bool isNull() const;

   constexpr bool operator==(const QUuid &other) const {
      if (data1 != other.data1 || data2 != other.data2 ||
            data3 != other.data3) {
         return false;
      }

      for (uint i = 0; i < 8; i++)
         if (data4[i] != other.data4[i]) {
            return false;
         }

      return true;
   }

   constexpr bool operator!=(const QUuid &other) const {
      return ! (*this == other);
   }

   bool operator<(const QUuid &other) const;
   bool operator>(const QUuid &other) const;

#if defined(Q_OS_WIN)
   // On Windows we have a type GUID that is used by the platform API, so we
   // provide convenience operators to cast from and to this type.
   constexpr QUuid(const GUID &guid)
      : data1(guid.Data1), data2(guid.Data2), data3(guid.Data3),
        data4{guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]}
   {
   }

   constexpr QUuid &operator=(const GUID &guid) {
      *this = QUuid(guid);
      return *this;
   }

   // emerald - 'constexpr operator GUID()' const does not work in MSVC 19.15
   operator GUID() const {
      GUID guid = { data1, data2, data3, { data4[0], data4[1], data4[2], data4[3],
            data4[4], data4[5], data4[6], data4[7] } };

      return guid;
   }

   constexpr bool operator==(const GUID &guid) const {
      return *this == QUuid(guid);
   }

   constexpr bool operator!=(const GUID &guid) const {
      return !(*this == guid);
   }
#endif

   static QUuid createUuid();
   static QUuid createUuidV3(const QUuid &ns, const QByteArray &baseData);
   static QUuid createUuidV5(const QUuid &ns, const QByteArray &baseData);

   static QUuid createUuidV3(const QUuid &ns, const QString &baseData) {
      return QUuid::createUuidV3(ns, baseData.toUtf8());
   }

   static QUuid createUuidV5(const QUuid &ns, const QString &baseData) {
      return QUuid::createUuidV5(ns, baseData.toUtf8());
   }

   QUuid::Variant variant() const;
   QUuid::Version version() const;

   uint    data1;
   ushort  data2;
   ushort  data3;
   uchar   data4[8];
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QUuid &uuid);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QUuid &uuid);

Q_CORE_EXPORT uint qHash(const QUuid &uuid, uint seed = 0);

inline bool operator<=(const QUuid &lhs, const QUuid &rhs)
{
   return !(rhs < lhs);
}

inline bool operator>=(const QUuid &lhs, const QUuid &rhs)
{
   return !(lhs < rhs);
}
#endif
