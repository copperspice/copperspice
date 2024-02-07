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

#ifndef QUTFCODEC_P_H
#define QUTFCODEC_P_H

#include <qtextcodec.h>

#include <qtextcodec_p.h>

enum DataEndianness {
   DetectEndianness,
   BigEndianness,
   LittleEndianness
};

struct QUtf8 {
   static QString convertToUnicode(const char *, int, QTextCodec::ConverterState *);
   static QByteArray convertFromUnicode(QStringView str, QTextCodec::ConverterState *);
};

struct QUtf16 {
   static QString convertToUnicode(const char *, int, QTextCodec::ConverterState *, DataEndianness = DetectEndianness);
   static QByteArray convertFromUnicode(QStringView str, QTextCodec::ConverterState *, DataEndianness = DetectEndianness);
};

struct QUtf32 {
   static QString convertToUnicode(const char *, int, QTextCodec::ConverterState *, DataEndianness = DetectEndianness);
   static QByteArray convertFromUnicode(QStringView str, QTextCodec::ConverterState *, DataEndianness = DetectEndianness);
};

#ifndef QT_NO_TEXTCODEC

class QUtf8Codec : public QTextCodec
{
 public:
   ~QUtf8Codec();

   QString convertToUnicode(const char *, int, ConverterState *) const override;
   QByteArray convertFromUnicode(QStringView str, ConverterState *) const override;
   void convertToUnicode(QString *target, const char *, int, ConverterState *) const;

   QString name() const override;
   int mibEnum() const override;
};

class QUtf16Codec : public QTextCodec
{

 public:
   QUtf16Codec() {
      e = DetectEndianness;
   }
   ~QUtf16Codec();

   QString name() const override;
   QStringList aliases() const override;
   int mibEnum() const override;

   QString convertToUnicode(const char *, int, ConverterState *) const override;
   QByteArray convertFromUnicode(QStringView str, ConverterState *) const override;

 protected:
   DataEndianness e;
};

class QUtf16BECodec : public QUtf16Codec
{
 public:
   QUtf16BECodec() : QUtf16Codec() {
      e = BigEndianness;
   }

   QString name() const override;
   QStringList aliases() const override;
   int mibEnum() const override;
};

class QUtf16LECodec : public QUtf16Codec
{
 public:
   QUtf16LECodec() : QUtf16Codec() {
      e = LittleEndianness;
   }

   QString name() const override;
   QStringList aliases() const override;
   int mibEnum() const override;
};

class QUtf32Codec : public QTextCodec
{
 public:
   QUtf32Codec() {
      e = DetectEndianness;
   }

   ~QUtf32Codec();

   QString name() const override;
   QStringList aliases() const override;
   int mibEnum() const override;

   QString convertToUnicode(const char *, int, ConverterState *) const override;
   QByteArray convertFromUnicode(QStringView str, ConverterState *) const override;

 protected:
   DataEndianness e;
};

class QUtf32BECodec : public QUtf32Codec
{
 public:
   QUtf32BECodec() : QUtf32Codec() {
      e = BigEndianness;
   }

   QString name() const override;
   QStringList aliases() const override;
   int mibEnum() const override;
};

class QUtf32LECodec : public QUtf32Codec
{
 public:
   QUtf32LECodec() : QUtf32Codec() {
      e = LittleEndianness;
   }

   QString name() const override;
   QStringList aliases() const override;
   int mibEnum() const override;
};

#endif // QT_NO_TEXTCODEC

#endif // QUTFCODEC_P_H
