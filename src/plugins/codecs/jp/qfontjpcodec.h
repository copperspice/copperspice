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

#ifndef QFONTJPCODEC_H
#define QFONTJPCODEC_H

#include <qtextcodec.h>
#include <qlist.h>

class QJpUnicodeConv;

#ifdef Q_WS_X11
class QFontJis0201Codec : public QTextCodec
{
public:
    QFontJis0201Codec();

    static QByteArray _name();
    static QList<QByteArray> _aliases() { return QList<QByteArray>(); }
    static int _mibEnum();

    QByteArray name() const override { return _name(); }
    QList<QByteArray> aliases() const override { return _aliases(); }
    int mibEnum() const override { return _mibEnum(); }

    QString convertToUnicode(const char *in, int len,  ConverterState *) const override;
    QByteArray convertFromUnicode(QStringView str, ConverterState *) const override;

};

class QFontJis0208Codec : public QTextCodec
{
public:
    QFontJis0208Codec();
    ~QFontJis0208Codec();

    static QByteArray _name();
    static QList<QByteArray> _aliases() { return QList<QByteArray>(); }
    static int _mibEnum();

    QByteArray name() const override { return _name(); }
    QList<QByteArray> aliases() const override { return _aliases(); }
    int mibEnum() const override { return _mibEnum(); }

    QByteArray convertFromUnicode(QStringView str, ConverterState *) const override;
    QString convertToUnicode(const char *chars, int len, ConverterState *) const override;

private:
    QJpUnicodeConv *convJP;
};
#endif

#endif // QFONTJPCODEC_H
