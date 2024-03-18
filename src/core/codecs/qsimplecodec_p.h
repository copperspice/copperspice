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

#ifndef QSIMPLECODEC_P_H
#define QSIMPLECODEC_P_H

#include <qtextcodec.h>

#ifndef QT_NO_TEXTCODEC

template <typename T>
class QAtomicPointer;

class QSimpleTextCodec: public QTextCodec
{
 public:
   static constexpr const int numSimpleCodecs = 30;

   explicit QSimpleTextCodec(int);
   ~QSimpleTextCodec();

   QString convertToUnicode(const char *, int, ConverterState *) const override;
   QByteArray convertFromUnicode(QStringView str, ConverterState *) const override;

   QString name() const override;
   QStringList aliases() const override;
   int mibEnum() const override;

 private:
   int forwardIndex;
   mutable QAtomicPointer<QByteArray> reverseMap;
};

#endif // QT_NO_TEXTCODEC

#endif // QSIMPLECODEC_P_H
