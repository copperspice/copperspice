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

#ifndef QTEXTSTREAM_H
#define QTEXTSTREAM_H

#include <qiodevice.h>
#include <qstring.h>
#include <qchar.h>
#include <qlocale.h>
#include <qscopedpointer.h>

#include <stdio.h>

#ifdef Status
#error qtextstream.h must be included before any header file that defines Status
#endif

class QTextCodec;
class QTextDecoder;
class QTextStreamPrivate;

class Q_CORE_EXPORT QTextStream
{
   Q_DECLARE_PRIVATE(QTextStream)

 public:
   enum RealNumberNotation {
      SmartNotation,
      FixedNotation,
      ScientificNotation
   };

   enum FieldAlignment {
      AlignLeft,
      AlignRight,
      AlignCenter,
      AlignAccountingStyle
   };

   enum Status {
      Ok,
      ReadPastEnd,
      ReadCorruptData,
      WriteFailed
   };

   enum NumberFlag {
      ShowBase = 0x1,
      ForcePoint = 0x2,
      ForceSign = 0x4,
      UppercaseBase = 0x8,
      UppercaseDigits = 0x10
   };
   using NumberFlags = QFlags<NumberFlag>;

   class Params
   {
      int   p_realNumberPrecision;
      int   p_integerBase;
      int   p_fieldWidth;
      QChar p_padChar;

      QTextStream::FieldAlignment     p_fieldAlignment;
      QTextStream::RealNumberNotation p_realNumberNotation;
      QTextStream::NumberFlags        p_numberFlags;

      friend class QTextStream;
   };

   QTextStream();
   explicit QTextStream(QIODevice *device);
   explicit QTextStream(FILE *fileHandle, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
   explicit QTextStream(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
   explicit QTextStream(QByteArray *array, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
   explicit QTextStream(const QByteArray &array, QIODevice::OpenMode openMode = QIODevice::ReadOnly);

   QTextStream(const QTextStream &) = delete;
   QTextStream &operator=(const QTextStream &) = delete;

   virtual ~QTextStream();

#ifndef QT_NO_TEXTCODEC
   void setCodec(QTextCodec *codec);
   void setCodec(const char *codecName);
   QTextCodec *codec() const;
   void setAutoDetectUnicode(bool enabled);
   bool autoDetectUnicode() const;
   void setGenerateByteOrderMark(bool generate);
   bool generateByteOrderMark() const;
#endif

   Params getParams() const;
   void setParams(const Params &data);

   void setLocale(const QLocale &locale);
   QLocale locale() const;

   void setDevice(QIODevice *device);
   QIODevice *device() const;

   void setString(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
   QString *string() const;

   Status status() const;
   void setStatus(Status status);
   void resetStatus();

   bool atEnd() const;
   void reset();
   void flush();
   bool seek(qint64 pos);
   qint64 pos() const;

   void skipWhiteSpace();

   QString readLine(qint64 maxlen = 0);
   bool readLineInto(QString *line, qint64 maxlen = 0);
   QString readAll();
   QString read(qint64 maxlen);

   void setFieldAlignment(FieldAlignment alignment);
   FieldAlignment fieldAlignment() const;

   void setPadChar(QChar ch);
   QChar padChar() const;

   void setFieldWidth(int width);
   int fieldWidth() const;

   void setNumberFlags(NumberFlags flags);
   NumberFlags numberFlags() const;

   void setIntegerBase(int base);
   int integerBase() const;

   void setRealNumberNotation(RealNumberNotation notation);
   RealNumberNotation realNumberNotation() const;

   void setRealNumberPrecision(int precision);
   int realNumberPrecision() const;

   QTextStream &operator>>(QChar &ch);
   QTextStream &operator>>(char &ch);
   QTextStream &operator>>(signed short &i);
   QTextStream &operator>>(unsigned short &i);
   QTextStream &operator>>(signed int &i);
   QTextStream &operator>>(unsigned int &i);
   QTextStream &operator>>(signed long &i);
   QTextStream &operator>>(unsigned long &i);
   QTextStream &operator>>(qint64 &i);
   QTextStream &operator>>(quint64 &i);
   QTextStream &operator>>(float &f);
   QTextStream &operator>>(double &f);
   QTextStream &operator>>(QString &str);
   QTextStream &operator>>(QByteArray &array);

   QTextStream &operator<<(bool b);
   QTextStream &operator<<(QChar ch);
   QTextStream &operator<<(char ch);
   QTextStream &operator<<(signed short i);
   QTextStream &operator<<(unsigned short i);
   QTextStream &operator<<(signed int i);
   QTextStream &operator<<(unsigned int i);
   QTextStream &operator<<(signed long i);
   QTextStream &operator<<(unsigned long i);
   QTextStream &operator<<(qint64 i);
   QTextStream &operator<<(quint64 i);
   QTextStream &operator<<(float f);
   QTextStream &operator<<(double f);

   QTextStream &operator<<(const QString &str);
   QTextStream &operator<<(const QStringView &str);
   QTextStream &operator<<(const QByteArray &array);

   QTextStream &operator<<(const void *ptr);

   QTextStream &operator<<(const char *str) {
      return *this << QString::fromLatin1(str);
   }

 protected:
   QScopedPointer<QTextStreamPrivate> d_ptr;

 private:
   friend class QDebugStateSaverPrivate;
   friend class QDebug;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextStream::NumberFlags)

using QTextStreamFunction = QTextStream &(*)(QTextStream &);    // manipulator function
using QTSMFI              = void (QTextStream::*)(int);         // manipulator w/int argument
using QTSMFC              = void (QTextStream::*)(QChar);       // manipulator w/QChar argument

class Q_CORE_EXPORT QTextStreamManipulator
{
 public:
   QTextStreamManipulator(QTSMFI m, int a)
      : mf(m), mc(nullptr), arg(a)
   { }

   QTextStreamManipulator(QTSMFC m, QChar c)
      : mf(nullptr), mc(m), arg(-1), ch(c)
   { }

   void exec(QTextStream &s) {
      if (mf) {
         (s.*mf)(arg);

      } else {
         (s.*mc)(ch);
      }
   }

 private:
   QTSMFI mf;           // QTextStream member function
   QTSMFC mc;           // QTextStream member function
   int arg;             // member function argument
   QChar ch;
};

inline QTextStream &operator>>(QTextStream &s, QTextStreamFunction f)
{
   return (*f)(s);
}

inline QTextStream &operator<<(QTextStream &s, QTextStreamFunction f)
{
   return (*f)(s);
}

inline QTextStream &operator<<(QTextStream &s, QTextStreamManipulator m)
{
   m.exec(s);
   return s;
}

Q_CORE_EXPORT QTextStream &bin(QTextStream &s);
Q_CORE_EXPORT QTextStream &oct(QTextStream &s);
Q_CORE_EXPORT QTextStream &dec(QTextStream &s);
Q_CORE_EXPORT QTextStream &hex(QTextStream &s);

Q_CORE_EXPORT QTextStream &showbase(QTextStream &s);
Q_CORE_EXPORT QTextStream &forcesign(QTextStream &s);
Q_CORE_EXPORT QTextStream &forcepoint(QTextStream &s);
Q_CORE_EXPORT QTextStream &noshowbase(QTextStream &s);
Q_CORE_EXPORT QTextStream &noforcesign(QTextStream &s);
Q_CORE_EXPORT QTextStream &noforcepoint(QTextStream &s);

Q_CORE_EXPORT QTextStream &uppercasebase(QTextStream &s);
Q_CORE_EXPORT QTextStream &uppercasedigits(QTextStream &s);
Q_CORE_EXPORT QTextStream &lowercasebase(QTextStream &s);
Q_CORE_EXPORT QTextStream &lowercasedigits(QTextStream &s);

Q_CORE_EXPORT QTextStream &fixed(QTextStream &s);
Q_CORE_EXPORT QTextStream &scientific(QTextStream &s);

Q_CORE_EXPORT QTextStream &left(QTextStream &s);
Q_CORE_EXPORT QTextStream &right(QTextStream &s);
Q_CORE_EXPORT QTextStream &center(QTextStream &s);

Q_CORE_EXPORT QTextStream &endl(QTextStream &s);
Q_CORE_EXPORT QTextStream &flush(QTextStream &s);
Q_CORE_EXPORT QTextStream &reset(QTextStream &s);

Q_CORE_EXPORT QTextStream &bom(QTextStream &s);

Q_CORE_EXPORT QTextStream &ws(QTextStream &s);

inline QTextStreamManipulator qSetFieldWidth(int width)
{
   QTSMFI func = &QTextStream::setFieldWidth;
   return QTextStreamManipulator(func, width);
}

inline QTextStreamManipulator qSetPadChar(QChar ch)
{
   QTSMFC func = &QTextStream::setPadChar;
   return QTextStreamManipulator(func, ch);
}

inline QTextStreamManipulator qSetRealNumberPrecision(int precision)
{
   QTSMFI func = &QTextStream::setRealNumberPrecision;
   return QTextStreamManipulator(func, precision);
}

#endif
