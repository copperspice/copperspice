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

#ifndef QUUID_H
#define QUUID_H

#include <QtCore/qstring.h>

#if defined(Q_OS_WIN)

#ifndef GUID_DEFINED
#define GUID_DEFINED

typedef struct _GUID {
   ulong   Data1;
   ushort  Data2;
   ushort  Data3;
   uchar   Data4[8];
} GUID, *REFGUID, *LPGUID;
#endif

#endif

QT_BEGIN_NAMESPACE

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
      Name              = 3, // 0 0 1 1
      Random            = 4  // 0 1 0 0
   };

   QUuid() {
      data1 = 0;
      data2 = 0;
      data3 = 0;
      for (int i = 0; i < 8; i++) {
         data4[i] = 0;
      }
   }
   QUuid(uint l, ushort w1, ushort w2, uchar b1, uchar b2, uchar b3, uchar b4, uchar b5, uchar b6, uchar b7, uchar b8) {
      data1 = l;
      data2 = w1;
      data3 = w2;
      data4[0] = b1;
      data4[1] = b2;
      data4[2] = b3;
      data4[3] = b4;
      data4[4] = b5;
      data4[5] = b6;
      data4[6] = b7;
      data4[7] = b8;
   }

#ifndef QT_NO_QUUID_STRING
   QUuid(const QString &);
   QUuid(const char *);

   QString toString() const;

   QUuid(const QByteArray &);
   QByteArray toByteArray() const;
#endif

   QByteArray toRfc4122() const;
   static QUuid fromRfc4122(const QByteArray &);
   bool isNull() const;

   bool operator==(const QUuid &orig) const {
      uint i;
      if (data1 != orig.data1 || data2 != orig.data2 ||
            data3 != orig.data3) {
         return false;
      }

      for (i = 0; i < 8; i++)
         if (data4[i] != orig.data4[i]) {
            return false;
         }

      return true;
   }

   bool operator!=(const QUuid &orig) const {
      return !(*this == orig);
   }

   bool operator<(const QUuid &other) const;
   bool operator>(const QUuid &other) const;

#if defined(Q_OS_WIN)
   // On Windows we have a type GUID that is used by the platform API, so we
   // provide convenience operators to cast from and to this type.
   QUuid(const GUID &guid) {
      data1 = guid.Data1;
      data2 = guid.Data2;
      data3 = guid.Data3;
      for (int i = 0; i < 8; i++) {
         data4[i] = guid.Data4[i];
      }
   }

   QUuid &operator=(const GUID &guid) {
      *this = QUuid(guid);
      return *this;
   }

   operator GUID() const {
      GUID guid = { data1, data2, data3, { data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7] } };
      return guid;
   }

   bool operator==(const GUID &guid) const {
      return *this == QUuid(guid);
   }

   bool operator!=(const GUID &guid) const {
      return !(*this == guid);
   }
#endif
   static QUuid createUuid();
   QUuid::Variant variant() const;
   QUuid::Version version() const;

   uint    data1;
   ushort  data2;
   ushort  data3;
   uchar   data4[8];
};

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QUuid &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QUuid &);
#endif

Q_CORE_EXPORT uint qHash(const QUuid &uuid);

QT_END_NAMESPACE

#endif // QUUID_H
