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

#include <qimageiohandler.h>
#include <qstringlist.h>

#if ! defined(QT_NO_IMAGEFORMATPLUGIN) && ! defined(QT_NO_SVGRENDERER)

#include <qsvgiohandler.h>

#include <qiodevice.h>
#include <qbytearray.h>
#include <qdebug.h>

class QSvgPlugin : public QImageIOPlugin
{
   CS_OBJECT(QSvgPlugin)

   CS_PLUGIN_IID(QImageIOHandlerInterface_ID)
   CS_PLUGIN_KEY("svg, svgz")

public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QString &format) const override;
    QImageIOHandler *create(QIODevice *device, const QString &format = QString()) const override;
};

CS_PLUGIN_REGISTER(QSvgPlugin)

QStringList QSvgPlugin::keys() const
{
    return QStringList() << "svg" << "svgz";
}

QImageIOPlugin::Capabilities QSvgPlugin::capabilities(QIODevice *device, const QString &format) const
{
    if (format == "svg" || format == "svgz") {
        return Capabilities(CanRead);
    }

    if (! format.isEmpty()) {
        return Qt::EmptyFlag;
    }

    Capabilities cap;

    if (device->isReadable() && QSvgIOHandler::canRead(device)) {
        cap |= CanRead;
    }

    return cap;
}

QImageIOHandler *QSvgPlugin::create(QIODevice *device, const QString &format) const
{
    QSvgIOHandler *hand = new QSvgIOHandler();
    hand->setDevice(device);
    hand->setFormat(format);

    return hand;
}

#endif
