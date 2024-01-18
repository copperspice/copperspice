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

#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qstringlist.h>

#ifndef QT_NO_TEXTCODECPLUGIN

#include "qeucjpcodec.h"
#include "qjiscodec.h"
#include "qsjiscodec.h"
#ifdef Q_WS_X11
#include "qfontjpcodec.h"
#endif

QT_BEGIN_NAMESPACE

class JPTextCodecs : public QTextCodecPlugin
{
public:
    JPTextCodecs() {}

    QList<QByteArray> names() const;
    QList<QByteArray> aliases() const;
    QList<int> mibEnums() const;

    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QByteArray &);
};

QList<QByteArray> JPTextCodecs::names() const
{
    QList<QByteArray> list;
    list += QEucJpCodec::_name();
    list += QJisCodec::_name();
    list += QSjisCodec::_name();
#ifdef Q_WS_X11
    list += QFontJis0201Codec::_name();
    list += QFontJis0208Codec::_name();
#endif
    return list;
}

QList<QByteArray> JPTextCodecs::aliases() const
{
    QList<QByteArray> list;
    list += QEucJpCodec::_aliases();
    list += QJisCodec::_aliases();
    list += QSjisCodec::_aliases();
#ifdef Q_WS_X11
    list += QFontJis0201Codec::_aliases();
    list += QFontJis0208Codec::_aliases();
#endif
    return list;
}

QList<int> JPTextCodecs::mibEnums() const
{
    QList<int> list;
    list += QEucJpCodec::_mibEnum();
    list += QJisCodec::_mibEnum();
    list += QSjisCodec::_mibEnum();
#ifdef Q_WS_X11
    list += QFontJis0201Codec::_mibEnum();
    list += QFontJis0208Codec::_mibEnum();
#endif
    return list;
}

QTextCodec *JPTextCodecs::createForMib(int mib)
{
    if (mib == QEucJpCodec::_mibEnum())
        return new QEucJpCodec;
    if (mib == QJisCodec::_mibEnum())
        return new QJisCodec;
    if (mib == QSjisCodec::_mibEnum())
        return new QSjisCodec;
#ifdef Q_WS_X11
    if (mib == QFontJis0208Codec::_mibEnum())
        return new QFontJis0208Codec;
    if (mib == QFontJis0201Codec::_mibEnum())
        return new QFontJis0201Codec;
#endif
    return 0;
}


QTextCodec *JPTextCodecs::createForName(const QByteArray &name)
{
    if (name == QEucJpCodec::_name() || QEucJpCodec::_aliases().contains(name))
        return new QEucJpCodec;
    if (name == QJisCodec::_name() || QJisCodec::_aliases().contains(name))
        return new QJisCodec;
    if (name == QSjisCodec::_name() || QSjisCodec::_aliases().contains(name))
        return new QSjisCodec;
#ifdef Q_WS_X11
    if (name == QFontJis0208Codec::_name() || QFontJis0208Codec::_aliases().contains(name))
        return new QFontJis0208Codec;
    if (name == QFontJis0201Codec::_name() || QFontJis0201Codec::_aliases().contains(name))
        return new QFontJis0201Codec;
#endif
    return 0;
}

Q_EXPORT_STATIC_PLUGIN(JPTextCodecs);
Q_EXPORT_PLUGIN2(qjpcodecs, JPTextCodecs);

QT_END_NAMESPACE

#endif // QT_NO_TEXTCODECPLUGIN
