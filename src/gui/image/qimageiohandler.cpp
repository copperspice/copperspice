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

/*!
    Constructs a QImageIOHandler object.
*/
QImageIOHandler::QImageIOHandler()
   : d_ptr(new QImageIOHandlerPrivate(this))
{
}

/*! \internal

    Constructs a QImageIOHandler object, using the private member \a
    dd.
*/
QImageIOHandler::QImageIOHandler(QImageIOHandlerPrivate &dd)
   : d_ptr(&dd)
{
}

/*!
    Destructs the QImageIOHandler object.
*/
QImageIOHandler::~QImageIOHandler()
{
}

/*!
    Sets the device of the QImageIOHandler to \a device. The image
    handler will use this device when reading and writing images.

    The device can only be set once and must be set before calling
    canRead(), read(), write(), etc. If you need to read multiple
    files, construct multiple instances of the appropriate
    QImageIOHandler subclass.

    \sa device()
*/
void QImageIOHandler::setDevice(QIODevice *device)
{
   Q_D(QImageIOHandler);
   d->device = device;
}

/*!
    Returns the device currently assigned to the QImageIOHandler. If
    not device has been assigned, 0 is returned.
*/
QIODevice *QImageIOHandler::device() const
{
   Q_D(const QImageIOHandler);
   return d->device;
}

/*!
    Sets the format of the QImageIOHandler to \a format. The format is
    most useful for handlers that support multiple image formats.

    \sa format()
*/
void QImageIOHandler::setFormat(const QByteArray &format)
{
   Q_D(QImageIOHandler);
   d->format = format;
}

/*!
    Sets the format of the QImageIOHandler to \a format. The format is
    most useful for handlers that support multiple image formats.

    This function is declared const so that it can be called from canRead().

    \sa format()
*/
void QImageIOHandler::setFormat(const QByteArray &format) const
{
   Q_D(const QImageIOHandler);
   d->format = format;
}

/*!
    Returns the format that is currently assigned to
    QImageIOHandler. If no format has been assigned, an empty string
    is returned.

    \sa setFormat()
*/
QByteArray QImageIOHandler::format() const
{
   Q_D(const QImageIOHandler);
   return d->format;
}

/*!
    \fn bool QImageIOHandler::read(QImage *image)

    Read an image from the device, and stores it in \a image.
    Returns true if the image is successfully read; otherwise returns
    false.

    For image formats that support incremental loading, and for animation
    formats, the image handler can assume that \a image points to the
    previous frame.

    \sa canRead()
*/

/*!
    \fn bool QImageIOHandler::canRead() const

    Returns true if an image can be read from the device (i.e., the
    image format is supported, the device can be read from and the
    initial header information suggests that the image can be read);
    otherwise returns false.

    When reimplementing canRead(), make sure that the I/O device
    (device()) is left in its original state (e.g., by using peek()
    rather than read()).

    \sa read(), QIODevice::peek()
*/

/*!
    \obsolete

    Use format() instead.
*/

QByteArray QImageIOHandler::name() const
{
   return format();
}

/*!
    Writes the image \a image to the assigned device. Returns true on
    success; otherwise returns false.

    The default implementation does nothing, and simply returns false.
*/
bool QImageIOHandler::write(const QImage &image)
{
   Q_UNUSED(image);
   return false;
}

/*!
    Sets the option \a option with the value \a value.

    \sa option(), ImageOption
*/
void QImageIOHandler::setOption(ImageOption option, const QVariant &value)
{
   Q_UNUSED(option);
   Q_UNUSED(value);
}

/*!
    Returns the value assigned to \a option as a QVariant. The type of
    the value depends on the option. For example, option(Size) returns
    a QSize variant.

    \sa setOption(), supportsOption()
*/
QVariant QImageIOHandler::option(ImageOption option) const
{
   Q_UNUSED(option);
   return QVariant();
}

/*!
    Returns true if the QImageIOHandler supports the option \a option;
    otherwise returns false. For example, if the QImageIOHandler
    supports the \l Size option, supportsOption(Size) must return
    true.

    \sa setOption(), option()
*/
bool QImageIOHandler::supportsOption(ImageOption option) const
{
   Q_UNUSED(option);
   return false;
}

/*!
    For image formats that support animation, this function returns
    the sequence number of the current image in the animation. If
    this function is called before any image is read(), -1 is
    returned. The number of the first image in the sequence is 0.

    If the image format does not support animation, 0 is returned.

    \sa read()
*/
int QImageIOHandler::currentImageNumber() const
{
   return 0;
}

/*!
    Returns the rect of the current image. If no rect is defined for the
    image, and empty QRect() is returned.

    This function is useful for animations, where only parts of the frame
    may be updated at a time.
*/
QRect QImageIOHandler::currentImageRect() const
{
   return QRect();
}

/*!
    For image formats that support animation, this function returns
    the number of images in the animation. If the image format does
    not support animation, or if it is unable to determine the number
    of images, 0 is returned.

    The default implementation returns 1 if canRead() returns true;
    otherwise 0 is returned.
*/
int QImageIOHandler::imageCount() const
{
   return canRead() ? 1 : 0;
}

/*!
   For image formats that support animation, this function jumps to the
   next image.

   The default implementation does nothing, and returns false.
*/
bool QImageIOHandler::jumpToNextImage()
{
   return false;
}

/*!
   For image formats that support animation, this function jumps to the image
   whose sequence number is \a imageNumber. The next call to read() will
   attempt to read this image.

   The default implementation does nothing, and returns false.
*/
bool QImageIOHandler::jumpToImage(int imageNumber)
{
   Q_UNUSED(imageNumber);
   return false;
}

/*!
    For image formats that support animation, this function returns
    the number of times the animation should loop. If the image format
    does not support animation, 0 is returned.
*/
int QImageIOHandler::loopCount() const
{
   return 0;
}

/*!
    For image formats that support animation, this function returns
    the number of milliseconds to wait until reading the next
    image. If the image format does not support animation, 0 is
    returned.
*/
int QImageIOHandler::nextImageDelay() const
{
   return 0;
}

/*!
    Constructs an image plugin with the given \a parent. This is
    invoked automatically by the Q_EXPORT_PLUGIN2() macro.
*/
QImageIOPlugin::QImageIOPlugin(QObject *parent)
   : QObject(parent)
{
}

/*!
    Destroys the picture format plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QImageIOPlugin::~QImageIOPlugin()
{
}

