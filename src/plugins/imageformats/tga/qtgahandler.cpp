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

#include "qtgahandler.h"
#include "qtgafile.h"

#include <QVariant>
#include <QDebug>
#include <QImage>

QT_BEGIN_NAMESPACE

QTgaHandler::QTgaHandler()
    : QImageIOHandler()
    , tga(0)
{
}

QTgaHandler::~QTgaHandler()
{
    delete tga;
}

bool QTgaHandler::canRead() const
{
    if (!tga)
        tga = new QTgaFile(device());
    if (tga->isValid())
    {
        setFormat("tga");
        return true;
    }
    return false;
}

bool QTgaHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("QTgaHandler::canRead() called with no device");
        return false;
    }
    QTgaFile tga(device);
    return tga.isValid();
}

bool QTgaHandler::read(QImage *image)
{
    if (!canRead())
        return false;
    *image = tga->readImage();
    return !image->isNull();
}

QByteArray QTgaHandler::name() const
{
    return "tga";
}

QVariant QTgaHandler::option(ImageOption option) const
{
    if (option == Size && canRead()) {
        return tga->size();
    } else if (option == CompressionRatio) {
        return tga->compression();
    } else if (option == ImageFormat) {
        return QImage::Format_ARGB32;
    }
    return QVariant();
}

void QTgaHandler::setOption(ImageOption option, const QVariant &value)
{
    Q_UNUSED(option);
    Q_UNUSED(value);
}

bool QTgaHandler::supportsOption(ImageOption option) const
{
    return option == CompressionRatio
            || option == Size
            || option == ImageFormat;
}

QT_END_NAMESPACE
