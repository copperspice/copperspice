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

#include <quuid.h>

#include <qcryptographichash.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qendian.h>
#include <qfile.h>
#include <qthreadstorage.h>

#include <qtools_p.h>

#include <stdlib.h>                // for RAND_MAX

template <class Integral>
void cs_internal_toHex(QByteArray &retval, Integral value)
{
   static const char digits[] = "0123456789abcdef";

   value = qToBigEndian(value);

   const char *p = reinterpret_cast<const char *>(&value);

   for (uint i = 0; i < sizeof(Integral); ++i) {
      uint j = (p[i] >> 4) & 0xf;
      retval.append(digits[j]);

      j = p[i] & 0xf;
      retval.append(digits[j]);
   }
}

template <class Char, class Integral>
bool _q_fromHex(const Char *&src, Integral &value)
{
   value = 0;

   for (uint i = 0; i < sizeof(Integral) * 2; ++i) {
      int ch = *src++;
      int tmp;

      if (ch >= '0' && ch <= '9') {
         tmp = ch - '0';

      } else if (ch >= 'a' && ch <= 'f') {
         tmp = ch - 'a' + 10;

      } else if (ch >= 'A' && ch <= 'F') {
         tmp = ch - 'A' + 10;

      } else {
         return false;

      }

      value = value * 16 + tmp;
   }

   return true;
}

QByteArray cs_internal_uuidToHex(const uint &d1, const ushort &d2, const ushort &d3, const uchar (&d4)[8])
{
   QByteArray retval;

   retval.append('{');

   cs_internal_toHex(retval, d1);
   retval.append('-');

   cs_internal_toHex(retval, d2);
   retval.append('-');

   cs_internal_toHex(retval, d3);
   retval.append('-');

   for (int i = 0; i < 2; ++i) {
      cs_internal_toHex(retval, d4[i]);
   }

   retval.append('-');

   for (int i = 2; i < 8; i++) {
      cs_internal_toHex(retval, d4[i]);
   }

   retval.append('}');

   return retval;
}

bool _q_uuidFromHex(const char *src, uint &d1, ushort &d2, ushort &d3, uchar (&d4)[8])
{
   if (*src == '{') {
      src++;
   }

   if (! _q_fromHex(src, d1)
         || *src++ != '-'  || ! _q_fromHex(src, d2)
         || *src++ != '-'  || ! _q_fromHex(src, d3)
         || *src++ != '-'  || ! _q_fromHex(src, d4[0]) || ! _q_fromHex(src, d4[1])
         || *src++ != '-'  || ! _q_fromHex(src, d4[2]) || ! _q_fromHex(src, d4[3])
         || ! _q_fromHex(src, d4[4]) || !_q_fromHex(src, d4[5]) || !_q_fromHex(src, d4[6]) || !_q_fromHex(src, d4[7])) {

      return false;
   }

   return true;
}

static QUuid createFromName(const QUuid &ns, const QByteArray &baseData, QCryptographicHash::Algorithm algorithm, int version)
{
   QByteArray hashResult;

   // create a scope so later resize won't reallocate
   {
      QCryptographicHash hash(algorithm);
      hash.addData(ns.toRfc4122());
      hash.addData(baseData);
      hashResult = hash.result();
   }
   hashResult.resize(16); // Sha1 will be too long

   QUuid result = QUuid::fromRfc4122(hashResult);

   result.data3 &= 0x0FFF;
   result.data3 |= (version << 12);
   result.data4[0] &= 0x3F;
   result.data4[0] |= 0x80;

   return result;
}
QUuid::QUuid(const QString &text)
{
   if (text.length() < 36) {
      *this = QUuid();
      return;
   }

   if (text.startsWith('{') && text.length() < 37) {
      *this = QUuid();
      return;
   }

   if (! _q_uuidFromHex(text.constData(), data1, data2, data3, data4)) {
      *this = QUuid();
      return;
   }
}

QUuid::QUuid(const char *text)
{
   if (! text) {
      *this = QUuid();
      return;
   }

   if (! _q_uuidFromHex(text, data1, data2, data3, data4)) {
      *this = QUuid();
      return;
   }
}

QUuid::QUuid(const QByteArray &text)
{
   if (text.length() < 36) {
      *this = QUuid();
      return;
   }

   if (text.startsWith('{') && text.length() < 37) {
      *this = QUuid();
      return;
   }

   if (! _q_uuidFromHex(text.constData(), data1, data2, data3, data4)) {
      *this = QUuid();
      return;
   }
}

QUuid QUuid::createUuidV3(const QUuid &ns, const QByteArray &baseData)
{
   return createFromName(ns, baseData, QCryptographicHash::Md5, 3);
}

QUuid QUuid::createUuidV5(const QUuid &ns, const QByteArray &baseData)
{
   return createFromName(ns, baseData, QCryptographicHash::Sha1, 5);
}
QUuid QUuid::fromRfc4122(const QByteArray &bytes)
{
   if (bytes.isEmpty() || bytes.length() != 16) {
      return QUuid();
   }

   uint d1;
   ushort d2, d3;
   uchar d4[8];

   const uchar *data = reinterpret_cast<const uchar *>(bytes.constData());

   d1 = qFromBigEndian<quint32>(data);
   data += sizeof(quint32);

   d2 = qFromBigEndian<quint16>(data);
   data += sizeof(quint16);

   d3 = qFromBigEndian<quint16>(data);
   data += sizeof(quint16);

   for (int i = 0; i < 8; ++i) {
      d4[i] = *(data);
      data++;
   }

   return QUuid(d1, d2, d3, d4[0], d4[1], d4[2], d4[3], d4[4], d4[5], d4[6], d4[7]);
}

QString QUuid::toString() const
{
   return QString::fromLatin1(toByteArray());
}

QByteArray QUuid::toByteArray() const
{
   QByteArray retval = cs_internal_uuidToHex(data1, data2, data3, data4);
   return retval;
}

QByteArray QUuid::toRfc4122() const
{
   // a UUID has 16 btyes
   QByteArray bytes(16, Qt::NoData);
   uchar *data = reinterpret_cast<uchar *>(bytes.data());

   qToBigEndian(data1, data);
   data += sizeof(quint32);

   qToBigEndian(data2, data);
   data += sizeof(quint16);

   qToBigEndian(data3, data);
   data += sizeof(quint16);

   for (int i = 0; i < 8; ++i) {
      *(data) = data4[i];
      data++;
   }

   return bytes;
}

QDataStream &operator<<(QDataStream &stream, const QUuid &id)
{
   QByteArray bytes;

   if (stream.byteOrder() == QDataStream::BigEndian) {
      bytes = id.toRfc4122();

   } else {
      // a UUID has 16 bytes
      bytes = QByteArray(16, Qt::NoData);
      uchar *data = reinterpret_cast<uchar *>(bytes.data());

      qToLittleEndian(id.data1, data);
      data += sizeof(quint32);
      qToLittleEndian(id.data2, data);
      data += sizeof(quint16);
      qToLittleEndian(id.data3, data);
      data += sizeof(quint16);

      for (int i = 0; i < 8; ++i) {
         *(data) = id.data4[i];
         data++;
      }
   }

   if (stream.writeRawData(bytes.data(), 16) != 16) {
      stream.setStatus(QDataStream::WriteFailed);
   }

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QUuid &id)
{
   QByteArray bytes(16, Qt::NoData);

   if (stream.readRawData(bytes.data(), 16) != 16) {
      stream.setStatus(QDataStream::ReadPastEnd);
      return stream;
   }

   if (stream.byteOrder() == QDataStream::BigEndian) {
      id = QUuid::fromRfc4122(bytes);
   } else {
      const uchar *data = reinterpret_cast<const uchar *>(bytes.constData());

      id.data1 = qFromLittleEndian<quint32>(data);
      data += sizeof(quint32);
      id.data2 = qFromLittleEndian<quint16>(data);
      data += sizeof(quint16);
      id.data3 = qFromLittleEndian<quint16>(data);
      data += sizeof(quint16);

      for (int i = 0; i < 8; ++i) {
         id.data4[i] = *(data);
         data++;
      }
   }

   return stream;
}

bool QUuid::isNull() const
{
   return data4[0] == 0 && data4[1] == 0 && data4[2] == 0 && data4[3] == 0 &&
         data4[4] == 0 && data4[5] == 0 && data4[6] == 0 && data4[7] == 0 &&
         data1 == 0 && data2 == 0 && data3 == 0;
}

QUuid::Variant QUuid::variant() const
{
   if (isNull()) {
      return VarUnknown;
   }

   // Check the 3 MSB of data4[0]
   if ((data4[0] & 0x80) == 0x00) {
      return NCS;

   } else if ((data4[0] & 0xC0) == 0x80) {
      return DCE;

   } else if ((data4[0] & 0xE0) == 0xC0) {
      return Microsoft;

   } else if ((data4[0] & 0xE0) == 0xE0) {
      return Reserved;
   }

   return VarUnknown;
}

QUuid::Version QUuid::version() const
{
   // Check the 4 MSB of data3
   Version ver = (Version)(data3 >> 12);

   if (isNull() || (variant() != DCE) || ver < Time || ver > Sha1) {
      return VerUnknown;
   }

   return ver;
}

#define ISLESS(f1, f2) if (f1!=f2) return (f1<f2);

bool QUuid::operator<(const QUuid &other) const
{
   if (variant() != other.variant()) {
      return variant() < other.variant();
   }

   ISLESS(data1, other.data1);
   ISLESS(data2, other.data2);
   ISLESS(data3, other.data3);

   for (int n = 0; n < 8; n++) {
      ISLESS(data4[n], other.data4[n]);
   }

   return false;
}

#define ISMORE(f1, f2) if (f1!=f2) return (f1>f2);

bool QUuid::operator>(const QUuid &other) const
{
   if (variant() != other.variant()) {
      return variant() > other.variant();
   }

   ISMORE(data1, other.data1);
   ISMORE(data2, other.data2);
   ISMORE(data3, other.data3);

   for (int n = 0; n < 8; n++) {
      ISMORE(data4[n], other.data4[n]);
   }

   return false;
}

#if defined(Q_OS_WIN)

#include <objbase.h> // For CoCreateGuid

QUuid QUuid::createUuid()
{
   GUID guid;
   CoCreateGuid(&guid);
   QUuid result = guid;
   return result;
}

#else // ! Q_OS_WIN

#if defined(Q_OS_UNIX)

static QThreadStorage<QFile *> *devUrandomStorage()
{
   static QThreadStorage<QFile *> retval;
   return &retval;
}

#endif

QUuid QUuid::createUuid()
{
   static constexpr const int AmountToRead = 4 * sizeof(uint);

   QUuid result;
   uint *data = &(result.data1);

#if defined(Q_OS_UNIX)
   QFile *devUrandom;

   devUrandom = devUrandomStorage()->localData();

   if (! devUrandom) {
      devUrandom = new QFile(QLatin1String("/dev/urandom"));
      devUrandom->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
      devUrandomStorage()->setLocalData(devUrandom);
   }

   if (devUrandom->isOpen() && devUrandom->read((char *) data, AmountToRead) == AmountToRead) {
      // nothing more to do

   } else
#endif

   {
      static constexpr const int intbits = sizeof(int) * 8;
      static int randbits = 0;

      if (! randbits) {
         int r = 0;
         int max = RAND_MAX;

         do {
            ++r;
         } while ((max = max >> 1));

         randbits = r;
      }

      // Seed the PRNG once per thread with a combination of current time, a
      // stack address and a serial counter (since thread stack addresses are re-used).

      static QThreadStorage<int *> uuidseed;

      if (! uuidseed.hasLocalData()) {
         int *pseed = new int;
         static QAtomicInt serial = QAtomicInt {2};

         qsrand(*pseed = QDateTime::currentDateTime().toTime_t()
                     + quintptr(&pseed) + serial.fetchAndAddRelaxed(1));

         uuidseed.setLocalData(pseed);
      }

      int chunks = 16 / sizeof(uint);

      while (chunks--) {
         uint randNumber = 0;

         for (int filled = 0; filled < intbits; filled += randbits) {
            randNumber |= qrand() << filled;
         }

         *(data + chunks) = randNumber;
      }
   }

   result.data4[0] = (result.data4[0] & 0x3F) | 0x80;        // UV_DCE
   result.data3 = (result.data3 & 0x0FFF) | 0x4000;          // UV_Random

   return result;
}
#endif

QDebug operator<<(QDebug dbg, const QUuid &id)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QUuid(" << id.toString() << ')';
   return dbg;
}

uint qHash(const QUuid &uuid, uint seed)
{
   return uuid.data1 ^ uuid.data2 ^ (uuid.data3 << 16)
         ^ ((uuid.data4[0] << 24) | (uuid.data4[1] << 16) | (uuid.data4[2] << 8) | uuid.data4[3])
         ^ ((uuid.data4[4] << 24) | (uuid.data4[5] << 16) | (uuid.data4[6] << 8) | uuid.data4[7])
         ^ seed;
}
