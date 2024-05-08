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

#ifndef QDATASTREAM_H
#define QDATASTREAM_H

#include <qcontainerfwd.h>
#include <qglobal.h>
#include <qiodevice.h>
#include <qregularexpression.h>
#include <qscopedpointer.h>

#ifdef Status
#error qdatastream.h must be included before any header file that defines Status
#endif

class QDataStream;
class QDataStreamPrivate;

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QString &str);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QString &str);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QString16 &str);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QString16 &str);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QChar32 &ch);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QChar32 &ch);

class Q_CORE_EXPORT QDataStream
{
 public:

#if CS_VERSION >= 0x011000
#error (CopperSpice compile issue in qdatastream.h) Verify version number is listed in the following enum
#endif

   enum Version {
      CS_1_0 = 128,
      CS_1_1 = CS_1_0,
      CS_1_2 = CS_1_1,
      CS_1_3 = CS_1_2,
      CS_1_4 = CS_1_3,
      CS_1_5 = CS_1_4,
      CS_1_6 = CS_1_5,
      CS_1_7 = CS_1_6,
      CS_1_8 = CS_1_7,
      CS_1_9 = CS_1_8,

      CS_DefaultStreamVersion = CS_1_9
   };

   enum ByteOrder {
      BigEndian    = QSysInfo::BigEndian,
      LittleEndian = QSysInfo::LittleEndian
   };

   enum Status {
      Ok,
      ReadPastEnd,
      ReadCorruptData,
      WriteFailed
   };

   enum FloatingPointPrecision {
      SinglePrecision,
      DoublePrecision
   };

   QDataStream();
   explicit QDataStream(QIODevice *device);

   QDataStream(QByteArray *buffer, QIODevice::OpenMode mode);
   QDataStream(const QByteArray &buffer);

   QDataStream(const QDataStream &) = delete;
   QDataStream &operator=(const QDataStream &) = delete;

   virtual ~QDataStream();

   inline QIODevice *device() const;
   void setDevice(QIODevice *device);

   bool atEnd() const;

   Status status() const;
   void setStatus(Status status);
   void resetStatus();

   FloatingPointPrecision floatingPointPrecision() const;
   void setFloatingPointPrecision(FloatingPointPrecision precision);

   inline ByteOrder byteOrder() const;
   void setByteOrder(ByteOrder order);

   inline int version() const;
   inline void setVersion(int version);

   QDataStream &operator>>(qint8 &i);
   inline QDataStream &operator>>(quint8 &i);
   QDataStream &operator>>(qint16 &i);
   inline QDataStream &operator>>(quint16 &i);
   QDataStream &operator>>(qint32 &i);
   inline QDataStream &operator>>(quint32 &i);
   QDataStream &operator>>(qint64 &i);
   inline QDataStream &operator>>(quint64 &i);
   QDataStream &operator>>(bool &i);
   QDataStream &operator>>(float &f);
   QDataStream &operator>>(double &f);
   QDataStream &operator>>(long &i);
   QDataStream &operator>>(unsigned long &i);
   QDataStream &operator>>(char *&str);

   QDataStream &operator<<(qint8 i);
   inline QDataStream &operator<<(quint8 i);
   QDataStream &operator<<(qint16 i);
   inline QDataStream &operator<<(quint16 i);
   QDataStream &operator<<(qint32 i);
   inline QDataStream &operator<<(quint32 i);
   QDataStream &operator<<(qint64 i);
   inline QDataStream &operator<<(quint64 i);
   QDataStream &operator<<(bool i);
   QDataStream &operator<<(float f);
   QDataStream &operator<<(double f);
   QDataStream &operator<<(long i);
   QDataStream &operator<<(unsigned long i);
   QDataStream &operator<<(const char *str);

   QDataStream &readBytes(char *&buffer, uint &len);
   int readRawData(char *buffer, int len);

   QDataStream &writeBytes(const char *buffer, uint len);
   int writeRawData(const char *buffer, int len);

   int skipRawData(int len);

 private:
   QScopedPointer<QDataStreamPrivate> d;

   QIODevice *m_device;
   bool owndev;
   bool noswap;
   ByteOrder byteorder;
   int ver;
   Status q_status;
};

inline QIODevice *QDataStream::device() const
{
   return m_device;
}

inline QDataStream::ByteOrder QDataStream::byteOrder() const
{
   return byteorder;
}

inline int QDataStream::version() const
{
   return ver;
}

inline void QDataStream::setVersion(int version)
{
   ver = version;
}

inline QDataStream &QDataStream::operator>>(quint8 &i)
{
   qint8 tmp;

   *this >> tmp;
   i = bit_cast<quint8>(tmp);

   return *this;
}

inline QDataStream &QDataStream::operator>>(quint16 &i)
{
   qint16 tmp;

   *this >> tmp;
   i = bit_cast<quint16>(tmp);

   return *this;
}

inline QDataStream &QDataStream::operator>>(quint32 &i)
{
   qint32 tmp;

   *this >> tmp;
   i = bit_cast<quint32>(tmp);

   return *this;
}

inline QDataStream &QDataStream::operator>>(quint64 &i)
{
   qint64 tmp;

   *this >> tmp;
   i = bit_cast<quint64>(tmp);

   return *this;
}

inline QDataStream &QDataStream::operator<<(quint8 i)
{
   return *this << qint8(i);
}

inline QDataStream &QDataStream::operator<<(quint16 i)
{
   return *this << qint16(i);
}

inline QDataStream &QDataStream::operator<<(quint32 i)
{
   return *this << qint32(i);
}

inline QDataStream &QDataStream::operator<<(quint64 i)
{
   return *this << qint64(i);
}

template <typename T>
QDataStream &operator>>(QDataStream &stream, QList<T> &list)
{
   list.clear();

   quint32 c;
   stream >> c;

   for (quint32 i = 0; i < c; ++i) {
      T t;
      stream >> t;

      list.append(t);

      if (stream.atEnd()) {
         break;
      }
   }

   return stream;
}

template <typename T>
QDataStream &operator<<(QDataStream &stream, const QList<T> &list)
{
   stream << quint32(list.size());

   for (int i = 0; i < list.size(); ++i) {
      stream << list.at(i);
   }

   return stream;
}

template <typename T>
QDataStream &operator>>(QDataStream &stream, QLinkedList<T> &list)
{
   list.clear();
   quint32 c;

   stream >> c;

   for (quint32 i = 0; i < c; ++i) {
      T t;
      stream >> t;
      list.append(t);

      if (stream.atEnd()) {
         break;
      }
   }

   return stream;
}

template <typename T>
QDataStream &operator<<(QDataStream &stream, const QLinkedList<T> &list)
{
   stream << quint32(list.size());
   typename QLinkedList<T>::const_itterator it = list.constBegin();

   for (; it != list.constEnd(); ++it) {
      stream << *it;
   }

   return stream;
}

template <typename T>
QDataStream &operator>>(QDataStream &stream, QVector<T> &vector)
{
   vector.clear();
   quint32 c;

   stream >> c;
   vector.resize(c);

   for (quint32 i = 0; i < c; ++i) {
      T t;
      stream >> t;
      vector[i] = t;
   }

   return stream;
}

template <typename T>
QDataStream &operator<<(QDataStream &stream, const QVector<T> &vector)
{
   stream << quint32(vector.size());

   for (typename QVector<T>::const_iterator it = vector.begin(); it != vector.end(); ++it) {
      stream << *it;
   }

   return stream;
}

template <typename T>
QDataStream &operator>>(QDataStream &stream, QSet<T> &set)
{
   set.clear();
   quint32 c;
   stream >> c;

   for (quint32 i = 0; i < c; ++i) {
      T t;
      stream >> t;
      set << t;

      if (stream.atEnd()) {
         break;
      }
   }

   return stream;
}

template <typename T>
QDataStream &operator<<(QDataStream &stream, const QSet<T> &set)
{
   stream << quint32(set.size());
   typename QSet<T>::const_iterator i = set.constBegin();

   while (i != set.constEnd()) {
      stream << *i;
      ++i;
   }

   return stream;
}

template <class Key, class T>
QDataStream &operator>>(QDataStream &stream, QHash<Key, T> &hash)
{
   QDataStream::Status oldStatus = stream.status();
   stream.resetStatus();

   hash.clear();

   quint32 n;
   stream >> n;

   for (quint32 i = 0; i < n; ++i) {
      if (stream.status() != QDataStream::Ok) {
         break;
      }

      Key k;
      T t;
      stream >> k >> t;
      hash.insert(k, t);
   }

   if (stream.status() != QDataStream::Ok) {
      hash.clear();
   }

   if (oldStatus != QDataStream::Ok) {
      stream.setStatus(oldStatus);
   }

   return stream;
}

template <class Key, class T>
QDataStream &operator<<(QDataStream &stream, const QHash<Key, T> &hash)
{
   stream << quint32(hash.size());

   typename QHash<Key, T>::const_iterator iter = hash.begin();
   typename QHash<Key, T>::const_iterator end  = hash.end();

   while (iter != end) {
      stream << iter.key() << iter.value();
      ++iter;
   }

   return stream;
}

template <class Key, class T>
QDataStream &operator>>(QDataStream &stream, QMultiHash<Key, T> &hash)
{
   QDataStream::Status oldStatus = stream.status();
   stream.resetStatus();

   hash.clear();

   quint32 n;
   stream >> n;

   for (quint32 i = 0; i < n; ++i) {
      if (stream.status() != QDataStream::Ok) {
         break;
      }

      Key k;
      T t;
      stream >> k >> t;
      hash.insertMulti(k, t);
   }

   if (stream.status() != QDataStream::Ok) {
      hash.clear();
   }

   if (oldStatus != QDataStream::Ok) {
      stream.setStatus(oldStatus);
   }

   return stream;
}

template <class Key, class T>
QDataStream &operator<<(QDataStream &stream, const QMultiHash<Key, T> &hash)
{
   stream << quint32(hash.size());

   typename QMultiHash<Key, T>::const_iterator iter = hash.begin();
   typename QMultiHash<Key, T>::const_iterator end  = hash.end();

   while (iter != end) {
      stream << iter.key() << iter.value();
      ++iter;
   }

   return stream;
}

template <class aKey, class aT>
QDataStream &operator>>(QDataStream &stream, QMap<aKey, aT> &map)
{
   QDataStream::Status oldStatus = stream.status();
   stream.resetStatus();

   map.clear();

   quint32 n;
   stream >> n;

   for (quint32 i = 0; i < n; ++i) {
      if (stream.status() != QDataStream::Ok) {
         break;
      }

      aKey key;
      aT value;
      stream >> key >> value;
      map.insert(key, value);
   }

   if (stream.status() != QDataStream::Ok) {
      map.clear();
   }

   if (oldStatus != QDataStream::Ok) {
      stream.setStatus(oldStatus);
   }

   return stream;
}

template <class Key, class Val, class C>
QDataStream &operator<<(QDataStream &stream, const QMap<Key, Val, C> &map)
{
   stream << quint32(map.size());

   typename QMap<Key, Val, C>::const_iterator it = map.end();
   typename QMap<Key, Val, C>::const_iterator begin = map.begin();

   while (it != begin) {
      --it;
      stream << it.key() << it.value();
   }

   return stream;
}

template <class aKey, class aT>
QDataStream &operator>>(QDataStream &stream, QMultiMap<aKey, aT> &map)
{
   QDataStream::Status oldStatus = stream.status();
   stream.resetStatus();

   map.clear();

   quint32 n;
   stream >> n;

   for (quint32 i = 0; i < n; ++i) {
      if (stream.status() != QDataStream::Ok) {
         break;
      }

      aKey key;
      aT value;
      stream >> key >> value;
      map.insertMulti(key, value);
   }

   if (stream.status() != QDataStream::Ok) {
      map.clear();
   }

   if (oldStatus != QDataStream::Ok) {
      stream.setStatus(oldStatus);
   }

   return stream;
}

template <class Key, class Val, class C>
QDataStream &operator<<(QDataStream &stream, const QMultiMap<Key, Val, C> &map)
{
   stream << quint32(map.size());

   typename QMultiMap<Key, Val, C>::const_iterator it    = map.end();
   typename QMultiMap<Key, Val, C>::const_iterator begin = map.begin();

   while (it != begin) {
      --it;
      stream << it.key() << it.value();
   }

   return stream;
}

template <typename S>
QDataStream &operator<<(QDataStream &stream, const Cs::QRegularExpression<S> &regExp)
{
   stream << regExp.pattern() << quint32(regExp.patternOptions());
   return stream;
}

template <typename S>
QDataStream &operator>>(QDataStream &stream, Cs::QRegularExpression<S> &regExp)
{
   S pattern;
   quint32 patternOptions;

   stream >> pattern >> patternOptions;
   regExp.setPattern(pattern);
   regExp.setPatternOptions(QPatternOptionFlags(patternOptions));

   return stream;
}

#endif
