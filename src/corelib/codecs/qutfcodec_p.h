/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QUTFCODEC_P_H
#define QUTFCODEC_P_H

#include "QtCore/qtextcodec.h"
#include "qtextcodec_p.h"

QT_BEGIN_NAMESPACE

enum DataEndianness {
   DetectEndianness,
   BigEndianness,
   LittleEndianness
};

struct QUtf8 {
   static QString convertToUnicode(const char *, int, QTextCodec::ConverterState *);
   static QByteArray convertFromUnicode(const QChar *, int, QTextCodec::ConverterState *);
};

struct QUtf16 {
   static QString convertToUnicode(const char *, int, QTextCodec::ConverterState *, DataEndianness = DetectEndianness);
   static QByteArray convertFromUnicode(const QChar *, int, QTextCodec::ConverterState *,
                                        DataEndianness = DetectEndianness);
};

struct QUtf32 {
   static QString convertToUnicode(const char *, int, QTextCodec::ConverterState *, DataEndianness = DetectEndianness);
   static QByteArray convertFromUnicode(const QChar *, int, QTextCodec::ConverterState *,
                                        DataEndianness = DetectEndianness);
};

#ifndef QT_NO_TEXTCODEC

class QUtf8Codec : public QTextCodec
{
 public:
   ~QUtf8Codec();

   QByteArray name() const;
   int mibEnum() const;

   QString convertToUnicode(const char *, int, ConverterState *) const;
   QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;
   void convertToUnicode(QString *target, const char *, int, ConverterState *) const;
};

class QUtf16Codec : public QTextCodec
{
 protected:
 public:
   QUtf16Codec() {
      e = DetectEndianness;
   }
   ~QUtf16Codec();

   QByteArray name() const;
   QList<QByteArray> aliases() const;
   int mibEnum() const;

   QString convertToUnicode(const char *, int, ConverterState *) const;
   QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

 protected:
   DataEndianness e;
};

class QUtf16BECodec : public QUtf16Codec
{
 public:
   QUtf16BECodec() : QUtf16Codec() {
      e = BigEndianness;
   }
   QByteArray name() const;
   QList<QByteArray> aliases() const;
   int mibEnum() const;
};

class QUtf16LECodec : public QUtf16Codec
{
 public:
   QUtf16LECodec() : QUtf16Codec() {
      e = LittleEndianness;
   }
   QByteArray name() const;
   QList<QByteArray> aliases() const;
   int mibEnum() const;
};

class QUtf32Codec : public QTextCodec
{
 public:
   QUtf32Codec() {
      e = DetectEndianness;
   }
   ~QUtf32Codec();

   QByteArray name() const;
   QList<QByteArray> aliases() const;
   int mibEnum() const;

   QString convertToUnicode(const char *, int, ConverterState *) const;
   QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

 protected:
   DataEndianness e;
};

class QUtf32BECodec : public QUtf32Codec
{
 public:
   QUtf32BECodec() : QUtf32Codec() {
      e = BigEndianness;
   }
   QByteArray name() const;
   QList<QByteArray> aliases() const;
   int mibEnum() const;
};

class QUtf32LECodec : public QUtf32Codec
{
 public:
   QUtf32LECodec() : QUtf32Codec() {
      e = LittleEndianness;
   }
   QByteArray name() const;
   QList<QByteArray> aliases() const;
   int mibEnum() const;
};


#endif // QT_NO_TEXTCODEC

QT_END_NAMESPACE

#endif // QUTFCODEC_P_H
