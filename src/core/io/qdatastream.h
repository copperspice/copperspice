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

#ifndef QDATASTREAM_H
#define QDATASTREAM_H

#include <qglobal.h>
#include <qscopedpointer.h>
#include <qiodevice.h>
#include <qcontainerfwd.h>

#ifdef Status
#error qdatastream.h must be included before any header file that defines Status
#endif

class QByteArray;
class QChar32;
class QIODevice;
class QString8;
class QString16;

class QDataStream;
class QDataStreamPrivate;

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QString &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QString &);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QString8 &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QString8 &);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QString16 &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QString16 &);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QChar32 &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QChar32 &);

class Q_CORE_EXPORT QDataStream
{
 public:

#if CS_VERSION >= 0x010500
#error (CopperSpice compile issue in qdatastream.h) Verify version number is listed in the following enum
#endif

   enum Version {
      Qt_4_0 = 7,
      Qt_4_1 = Qt_4_0,
      Qt_4_2 = 8,
      Qt_4_3 = 9,
      Qt_4_4 = 10,
      Qt_4_5 = 11,
      Qt_4_6 = 12,
      Qt_4_7 = Qt_4_6,
      Qt_4_8 = Qt_4_7,

      CS_1_0 = 128,
      CS_1_1 = CS_1_0,
      CS_1_2 = CS_1_1,
      CS_1_3 = CS_1_2,
      CS_1_4 = CS_1_3,

      CS_DefaultStreamVersion = CS_1_4
   };

   enum ByteOrder {
      BigEndian = QSysInfo::BigEndian,
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
   explicit QDataStream(QIODevice *);

   QDataStream(QByteArray *, QIODevice::OpenMode flags);
   QDataStream(const QByteArray &);
   virtual ~QDataStream();

   QIODevice *device() const;
   void setDevice(QIODevice *);
   void unsetDevice();

   bool atEnd() const;

   Status status() const;
   void setStatus(Status status);
   void resetStatus();

   FloatingPointPrecision floatingPointPrecision() const;
   void setFloatingPointPrecision(FloatingPointPrecision precision);

   ByteOrder byteOrder() const;
   void setByteOrder(ByteOrder);

   int version() const;
   void setVersion(int);

   QDataStream &operator>>(qint8 &i);
   QDataStream &operator>>(quint8 &i);
   QDataStream &operator>>(qint16 &i);
   QDataStream &operator>>(quint16 &i);
   QDataStream &operator>>(qint32 &i);
   QDataStream &operator>>(quint32 &i);
   QDataStream &operator>>(qint64 &i);
   QDataStream &operator>>(quint64 &i);
   QDataStream &operator>>(bool &i);
   QDataStream &operator>>(float &f);
   QDataStream &operator>>(double &f);
   QDataStream &operator>>(long &i);
   QDataStream &operator>>(unsigned long &i);
   QDataStream &operator>>(char *&str);

   QDataStream &operator<<(qint8 i);
   QDataStream &operator<<(quint8 i);
   QDataStream &operator<<(qint16 i);
   QDataStream &operator<<(quint16 i);
   QDataStream &operator<<(qint32 i);
   QDataStream &operator<<(quint32 i);
   QDataStream &operator<<(qint64 i);
   QDataStream &operator<<(quint64 i);
   QDataStream &operator<<(bool i);
   QDataStream &operator<<(float f);
   QDataStream &operator<<(double f);
   QDataStream &operator<<(long i);
   QDataStream &operator<<(unsigned long i);
   QDataStream &operator<<(const char *str);

   QDataStream &readBytes(char *&, uint &len);
   int readRawData(char *, int len);

   QDataStream &writeBytes(const char *, uint len);
   int writeRawData(const char *, int len);

   int skipRawData(int len);

 private:
   Q_DISABLE_COPY(QDataStream)

   QScopedPointer<QDataStreamPrivate> d;

   QIODevice *dev;
   bool owndev;
   bool noswap;
   ByteOrder byteorder;
   int ver;
   Status q_status;
};


/*****************************************************************************
  QDataStream inline functions
 *****************************************************************************/

inline QIODevice *QDataStream::device() const
{
   return dev;
}

inline QDataStream::ByteOrder QDataStream::byteOrder() const
{
   return byteorder;
}

inline int QDataStream::version() const
{
   return ver;
}

inline void QDataStream::setVersion(int v)
{
   ver = v;
}

inline QDataStream &QDataStream::operator>>(quint8 &i)
{
   return *this >> reinterpret_cast<qint8 &>(i);
}

inline QDataStream &QDataStream::operator>>(quint16 &i)
{
   return *this >> reinterpret_cast<qint16 &>(i);
}

inline QDataStream &QDataStream::operator>>(quint32 &i)
{
   return *this >> reinterpret_cast<qint32 &>(i);
}

inline QDataStream &QDataStream::operator>>(quint64 &i)
{
   return *this >> reinterpret_cast<qint64 &>(i);
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
QDataStream &operator>>(QDataStream &s, QList<T> &l)
{
   l.clear();

   quint32 c;
   s >> c;
   l.reserve(c);

   for (quint32 i = 0; i < c; ++i) {
      T t;
      s >> t;
      l.append(t);

      if (s.atEnd()) {
         break;
      }
   }

   return s;
}

template <typename T>
QDataStream &operator<<(QDataStream &s, const QList<T> &l)
{
   s << quint32(l.size());
   for (int i = 0; i < l.size(); ++i) {
      s << l.at(i);
   }

   return s;
}

template <typename T>
QDataStream &operator>>(QDataStream &s, QLinkedList<T> &l)
{
   l.clear();
   quint32 c;

   s >> c;

   for (quint32 i = 0; i < c; ++i) {
      T t;
      s >> t;
      l.append(t);
      if (s.atEnd()) {
         break;
      }
   }
   return s;
}

template <typename T>
QDataStream &operator<<(QDataStream &s, const QLinkedList<T> &l)
{
   s << quint32(l.size());
   typename QLinkedList<T>::ConstIterator it = l.constBegin();

   for (; it != l.constEnd(); ++it) {
      s << *it;
   }
   return s;
}

template<typename T>
QDataStream &operator>>(QDataStream &s, QVector<T> &v)
{
   v.clear();
   quint32 c;

   s >> c;
   v.resize(c);

   for (quint32 i = 0; i < c; ++i) {
      T t;
      s >> t;
      v[i] = t;
   }
   return s;
}

template<typename T>
QDataStream &operator<<(QDataStream &s, const QVector<T> &v)
{
   s << quint32(v.size());
   for (typename QVector<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
      s << *it;
   }
   return s;
}

template <typename T>
QDataStream &operator>>(QDataStream &in, QSet<T> &set)
{
   set.clear();
   quint32 c;
   in >> c;
   for (quint32 i = 0; i < c; ++i) {
      T t;
      in >> t;
      set << t;
      if (in.atEnd()) {
         break;
      }
   }
   return in;
}

template <typename T>
QDataStream &operator<<(QDataStream &out, const QSet<T> &set)
{
   out << quint32(set.size());
   typename QSet<T>::const_iterator i = set.constBegin();

   while (i != set.constEnd()) {
      out << *i;
      ++i;
   }
   return out;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, QHash<Key, T> &hash)
{
   QDataStream::Status oldStatus = in.status();
   in.resetStatus();

   hash.clear();

   quint32 n;
   in >> n;

   for (quint32 i = 0; i < n; ++i) {
      if (in.status() != QDataStream::Ok) {
         break;
      }

      Key k;
      T t;
      in >> k >> t;
      hash.insert(k, t);
   }

   if (in.status() != QDataStream::Ok) {
      hash.clear();
   }

   if (oldStatus != QDataStream::Ok) {
      in.setStatus(oldStatus);
   }

   return in;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator<<(QDataStream &out, const QHash<Key, T> &hash)
{
   out << quint32(hash.size());

   typename QHash<Key, T>::ConstIterator iter = hash.begin();
   typename QHash<Key, T>::ConstIterator end  = hash.end();

   while (iter != end) {
      out << iter.key() << iter.value();
      ++iter;
   }

   return out;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, QMultiHash<Key, T> &hash)
{
   QDataStream::Status oldStatus = in.status();
   in.resetStatus();

   hash.clear();

   quint32 n;
   in >> n;

   for (quint32 i = 0; i < n; ++i) {
      if (in.status() != QDataStream::Ok) {
         break;
      }

      Key k;
      T t;
      in >> k >> t;
      hash.insertMulti(k, t);
   }

   if (in.status() != QDataStream::Ok) {
      hash.clear();
   }

   if (oldStatus != QDataStream::Ok) {
      in.setStatus(oldStatus);
   }

   return in;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE QDataStream &operator<<(QDataStream &out, const QMultiHash<Key, T> &hash)
{
   out << quint32(hash.size());

   typename QMultiHash<Key, T>::ConstIterator iter = hash.begin();
   typename QMultiHash<Key, T>::ConstIterator end  = hash.end();

   while (iter != end) {
      out << iter.key() << iter.value();
      ++iter;
   }

   return out;
}

template <class aKey, class aT>
Q_OUTOFLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, QMap<aKey, aT> &map)
{
   QDataStream::Status oldStatus = in.status();
   in.resetStatus();

   map.clear();

   quint32 n;
   in >> n;

   for (quint32 i = 0; i < n; ++i) {
      if (in.status() != QDataStream::Ok) {
         break;
      }

      aKey key;
      aT value;
      in >> key >> value;
      map.insert(key, value);
   }

   if (in.status() != QDataStream::Ok) {
      map.clear();
   }

   if (oldStatus != QDataStream::Ok) {
      in.setStatus(oldStatus);
   }

   return in;
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE QDataStream &operator<<(QDataStream &out, const QMap<Key, Val, C> &map)
{
   out << quint32(map.size());
   typename QMap<Key, Val, C>::ConstIterator it = map.end();
   typename QMap<Key, Val, C>::ConstIterator begin = map.begin();

   while (it != begin) {
      --it;
      out << it.key() << it.value();
   }

   return out;
}

template <class aKey, class aT>
Q_OUTOFLINE_TEMPLATE QDataStream &operator>>(QDataStream &in, QMultiMap<aKey, aT> &map)
{
   QDataStream::Status oldStatus = in.status();
   in.resetStatus();

   map.clear();

   quint32 n;
   in >> n;

   for (quint32 i = 0; i < n; ++i) {
      if (in.status() != QDataStream::Ok) {
         break;
      }

      aKey key;
      aT value;
      in >> key >> value;
      map.insertMulti(key, value);
   }

   if (in.status() != QDataStream::Ok) {
      map.clear();
   }

   if (oldStatus != QDataStream::Ok) {
      in.setStatus(oldStatus);
   }

   return in;
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE QDataStream &operator<<(QDataStream &out, const QMultiMap<Key, Val, C> &map)
{
   out << quint32(map.size());

   typename QMultiMap<Key, Val, C>::ConstIterator it    = map.end();
   typename QMultiMap<Key, Val, C>::ConstIterator begin = map.begin();

   while (it != begin) {
      --it;
      out << it.key() << it.value();
   }

   return out;
}

#endif
