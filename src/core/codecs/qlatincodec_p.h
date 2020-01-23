/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QLATINCODEC_P_H
#define QLATINCODEC_P_H

#include <QtCore/qtextcodec.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TEXTCODEC

class QLatin1Codec : public QTextCodec
{
 public:
   ~QLatin1Codec();

   QString convertToUnicode(const char *, int, ConverterState *) const override;
   QByteArray convertFromUnicode(QStringView str, ConverterState *) const override;

   QString name() const override;
   QStringList aliases() const override;
   int mibEnum() const override;
};

class QLatin15Codec: public QTextCodec
{
 public:
   ~QLatin15Codec();

   QString convertToUnicode(const char *, int, ConverterState *) const override;
   QByteArray convertFromUnicode(QStringView str, ConverterState *) const override;

   QString name() const override;
   QStringList aliases() const override;
   int mibEnum() const override;
};

#endif // QT_NO_TEXTCODEC

QT_END_NAMESPACE

#endif // QLATINCODEC_P_H
