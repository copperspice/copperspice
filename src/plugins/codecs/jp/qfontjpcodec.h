/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QFONTJPCODEC_H
#define QFONTJPCODEC_H

#include <QtCore/qtextcodec.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QJpUnicodeConv;

#ifdef Q_WS_X11
class QFontJis0201Codec : public QTextCodec
{
public:
    QFontJis0201Codec();

    static QByteArray _name();
    static QList<QByteArray> _aliases() { return QList<QByteArray>(); }
    static int _mibEnum();

    QByteArray name() const { return _name(); }
    QList<QByteArray> aliases() const { return _aliases(); }
    int mibEnum() const { return _mibEnum(); }

    QByteArray convertFromUnicode(const QChar *uc, int len,  ConverterState *) const;
    QString convertToUnicode(const char*, int,  ConverterState *) const;
};

class QFontJis0208Codec : public QTextCodec
{
public:
    QFontJis0208Codec();
    ~QFontJis0208Codec();

    static QByteArray _name();
    static QList<QByteArray> _aliases() { return QList<QByteArray>(); }
    static int _mibEnum();

    QByteArray name() const { return _name(); }
    QList<QByteArray> aliases() const { return _aliases(); }
    int mibEnum() const { return _mibEnum(); }

    QString convertToUnicode(const char* /*chars*/, int /*len*/, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *uc, int len, ConverterState *) const;
private:
    QJpUnicodeConv *convJP;
};
#endif

QT_END_NAMESPACE

#endif // QFONTJPCODEC_H
