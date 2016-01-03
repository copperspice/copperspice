/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QSIMPLECODEC_P_H
#define QSIMPLECODEC_P_H

#include <qtextcodec.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TEXTCODEC

template <typename T> class QAtomicPointer;

class QSimpleTextCodec: public QTextCodec
{
 public:
   enum { numSimpleCodecs = 30 };

   explicit QSimpleTextCodec(int);
   ~QSimpleTextCodec();

   QString convertToUnicode(const char *, int, ConverterState *) const;
   QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

   QByteArray name() const;
   QList<QByteArray> aliases() const;
   int mibEnum() const;

 private:
   int forwardIndex;
   mutable QAtomicPointer<QByteArray> reverseMap;
};

#endif // QT_NO_TEXTCODEC

QT_END_NAMESPACE

#endif // QSIMPLECODEC_P_H
