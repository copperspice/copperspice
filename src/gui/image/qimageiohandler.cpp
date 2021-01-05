/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <qbytearray.h>
#include <qimage.h>
#include <qvariant.h>

class QIODevice;

class QImageIOHandlerPrivate
{
   Q_DECLARE_PUBLIC(QImageIOHandler)

 public:
   QImageIOHandlerPrivate(QImageIOHandler *q);
   virtual ~QImageIOHandlerPrivate();

   QIODevice *device;
   mutable QByteArray format;

   QImageIOHandler *q_ptr;
};

QImageIOHandlerPrivate::QImageIOHandlerPrivate(QImageIOHandler *q)
{
   device = 0;
   q_ptr = q;
}

QImageIOHandlerPrivate::~QImageIOHandlerPrivate()
{
}

QImageIOHandler::QImageIOHandler()
   : d_ptr(new QImageIOHandlerPrivate(this))
{
}

QImageIOHandler::QImageIOHandler(QImageIOHandlerPrivate &dd)
   : d_ptr(&dd)
{
}

QImageIOHandler::~QImageIOHandler()
{
}

void QImageIOHandler::setDevice(QIODevice *device)
{
   Q_D(QImageIOHandler);
   d->device = device;
}

QIODevice *QImageIOHandler::device() const
{
   Q_D(const QImageIOHandler);
   return d->device;
}

void QImageIOHandler::setFormat(const QByteArray &format)
{
   Q_D(QImageIOHandler);
   d->format = format;
}

void QImageIOHandler::setFormat(const QByteArray &format) const
{
   Q_D(const QImageIOHandler);
   d->format = format;
}

QByteArray QImageIOHandler::format() const
{
   Q_D(const QImageIOHandler);
   return d->format;
}

QByteArray QImageIOHandler::name() const
{
   return format();
}

bool QImageIOHandler::write(const QImage &image)
{
   (void) image;
   return false;
}

void QImageIOHandler::setOption(ImageOption option, const QVariant &value)
{
   (void) option;
   (void) value;
}

QVariant QImageIOHandler::option(ImageOption option) const
{
   (void) option;
   return QVariant();
}

bool QImageIOHandler::supportsOption(ImageOption option) const
{
   (void) option;
   return false;
}

int QImageIOHandler::currentImageNumber() const
{
   return 0;
}

QRect QImageIOHandler::currentImageRect() const
{
   return QRect();
}

int QImageIOHandler::imageCount() const
{
   return canRead() ? 1 : 0;
}

bool QImageIOHandler::jumpToNextImage()
{
   return false;
}

bool QImageIOHandler::jumpToImage(int imageNumber)
{
   (void) imageNumber;
   return false;
}

int QImageIOHandler::loopCount() const
{
   return 0;
}

int QImageIOHandler::nextImageDelay() const
{
   return 0;
}

QImageIOPlugin::QImageIOPlugin(QObject *parent)
   : QObject(parent)
{
}

QImageIOPlugin::~QImageIOPlugin()
{
}

