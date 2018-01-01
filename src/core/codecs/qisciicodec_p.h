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

#ifndef QISCIICODEC_P_H
#define QISCIICODEC_P_H

#include <qtextcodec.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_CODECS

class QIsciiCodec : public QTextCodec
{
 public:
   explicit QIsciiCodec(int i) : idx(i) {}
   ~QIsciiCodec();

   QString convertToUnicode(const char *, int, ConverterState *) const override;
   QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;

   QByteArray name() const override;
   int mibEnum() const override;

 private:
   int idx;
};

#endif

QT_END_NAMESPACE

#endif // QISCIICODEC_P_H
