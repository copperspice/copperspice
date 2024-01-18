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
#include <qlist.h>

#include "qgb18030codec.h"

#ifndef QT_NO_TEXTCODECPLUGIN

QT_BEGIN_NAMESPACE

class CNTextCodecs : public QTextCodecPlugin
{
public:
    CNTextCodecs() {}

    QList<QByteArray> names() const;
    QList<QByteArray> aliases() const;
    QList<int> mibEnums() const;

    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QByteArray &);
};

QList<QByteArray> CNTextCodecs::names() const
{
    QList<QByteArray> list;
    list += QGb18030Codec::_name();
    list += QGbkCodec::_name();
    list += QGb2312Codec::_name();
#ifdef Q_WS_X11
    list += QFontGb2312Codec::_name();
    list += QFontGbkCodec::_name();
#endif
    return list;
}

QList<QByteArray> CNTextCodecs::aliases() const
{
    QList<QByteArray> list;
    list += QGb18030Codec::_aliases();
    list += QGbkCodec::_aliases();
    list += QGb2312Codec::_aliases();
#ifdef Q_WS_X11
    list += QFontGb2312Codec::_aliases();
    list += QFontGbkCodec::_aliases();
#endif
    return list;
}

QList<int> CNTextCodecs::mibEnums() const
{
    QList<int> list;
    list += QGb18030Codec::_mibEnum();
    list += QGbkCodec::_mibEnum();
    list += QGb2312Codec::_mibEnum();
#ifdef Q_WS_X11
    list += QFontGb2312Codec::_mibEnum();
    list += QFontGbkCodec::_mibEnum();
#endif
    return list;
}

QTextCodec *CNTextCodecs::createForMib(int mib)
{
    if (mib == QGb18030Codec::_mibEnum())
        return new QGb18030Codec;
    if (mib == QGbkCodec::_mibEnum())
        return new QGbkCodec;
    if (mib == QGb2312Codec::_mibEnum())
        return new QGb2312Codec;
#ifdef Q_WS_X11
    if (mib == QFontGbkCodec::_mibEnum())
        return new QFontGbkCodec;
    if (mib == QFontGb2312Codec::_mibEnum())
        return new QFontGb2312Codec;
#endif
    return 0;
}


QTextCodec *CNTextCodecs::createForName(const QByteArray &name)
{
    if (name == QGb18030Codec::_name() || QGb18030Codec::_aliases().contains(name))
        return new QGb18030Codec;
    if (name == QGbkCodec::_name() || QGbkCodec::_aliases().contains(name))
        return new QGbkCodec;
    if (name == QGb2312Codec::_name() || QGb2312Codec::_aliases().contains(name))
        return new QGb2312Codec;
#ifdef Q_WS_X11
    if (name == QFontGbkCodec::_name() || QFontGbkCodec::_aliases().contains(name))
        return new QFontGbkCodec;
    if (name == QFontGb2312Codec::_name() || QFontGb2312Codec::_aliases().contains(name))
        return new QFontGb2312Codec;
#endif
    return 0;
}


Q_EXPORT_STATIC_PLUGIN(CNTextCodecs)
Q_EXPORT_PLUGIN2(qcncodecs, CNTextCodecs)

QT_END_NAMESPACE

#endif // QT_NO_TEXTCODECPLUGIN
