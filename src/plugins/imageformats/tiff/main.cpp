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
#include <qdebug.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEFORMAT_TIFF
#undef QT_NO_IMAGEFORMAT_TIFF
#endif

#include <qtiffhandler_p.h>

class QTiffPlugin : public QImageIOPlugin
{
   CS_OBJECT(QTiffPlugin)

   CS_PLUGIN_IID(QImageIOHandlerInterface_ID)
   CS_PLUGIN_KEY("tiff, tif")

public:
    Capabilities capabilities(QIODevice * device, const QByteArray & format) const;
    QImageIOHandler * create(QIODevice * device, const QByteArray & format = QByteArray()) const;
    QStringList keys() const;
};

CS_PLUGIN_REGISTER(QTiffPlugin)

QImageIOPlugin::Capabilities QTiffPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "tiff" || format == "tif")
        return Capabilities(CanRead | CanWrite);

    if (!format.isEmpty())
        return 0;

    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && QTiffHandler::canRead(device))
        cap |= CanRead;

    if (device->isWritable())
        cap |= CanWrite;

    return cap;
}

QImageIOHandler* QTiffPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *tiffHandler = new QTiffHandler();
    tiffHandler->setDevice(device);
    tiffHandler->setFormat(format);

    return tiffHandler;
}

QStringList QTiffPlugin::keys() const
{
    return QStringList() << "tiff" << "tif";
}

#endif
