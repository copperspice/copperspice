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

#include <qimageiohandler.h>
#include <qstringlist.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEFORMAT_MNG
#undef QT_NO_IMAGEFORMAT_MNG
#endif
#include <qmnghandler_p.h>

#include <qiodevice.h>
#include <qbytearray.h>

class QMngPlugin : public QImageIOPlugin
{
   CS_OBJECT(QMngPlugin)

   CS_PLUGIN_IID(QImageIOHandlerInterface_ID)
   CS_PLUGIN_KEY("mng")

public:
   QStringList keys() const;
   Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
   QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

CS_PLUGIN_REGISTER(QMngPlugin)

QStringList QMngPlugin::keys() const
{
    return QStringList() << "mng";
}

QImageIOPlugin::Capabilities QMngPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "mng")
        return Capabilities(CanRead);

    if (!format.isEmpty())
        return 0;

    if (!device->isOpen())
        return 0;

    Capabilities cap;

    if (device->isReadable() && QMngHandler::canRead(device))
        cap |= CanRead;

    return cap;
}

QImageIOHandler *QMngPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QMngHandler *hand = new QMngHandler();
    hand->setDevice(device);
    hand->setFormat(format);
    return hand;
}

#endif
