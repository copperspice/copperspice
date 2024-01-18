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

#include "qtgafile.h"

#include <QIODevice>
#include <QDebug>
#include <QDateTime>

struct TgaReader
{
    virtual ~TgaReader() {}
    virtual QRgb operator()(QIODevice *s) const = 0;
};

struct Tga16Reader : public TgaReader
{
    ~Tga16Reader() {}
    QRgb operator()(QIODevice *s) const
    {
        char ch1, ch2;
        if (s->getChar(&ch1) && s->getChar(&ch2)) {
            quint16 d = (int(ch1) & 0xFF) | ((int(ch2) & 0xFF) << 8);
            QRgb result = (d & 0x8000) ? 0xFF000000 : 0x00000000;
            result |= (d & 0x7C00 << 6) | (d & 0x03E0 << 3) | (d & 0x001F);
            return result;
        } else {
            return 0;
        }
    }
};

struct Tga24Reader : public TgaReader
{
    QRgb operator()(QIODevice *s) const
    {
        char r, g, b;
        if (s->getChar(&b) && s->getChar(&g) && s->getChar(&r))
            return qRgb(uchar(r), uchar(g), uchar(b));
        else
            return 0;
    }
};

struct Tga32Reader : public TgaReader
{
    QRgb operator()(QIODevice *s) const
    {
        char r, g, b, a;
        if (s->getChar(&b) && s->getChar(&g) && s->getChar(&r) && s->getChar(&a))
            return qRgba(uchar(r), uchar(g), uchar(b), uchar(a));
        else
            return 0;
    }
};

/*!
    \class QTgaFile
    \since 4.8
    \internal

    File data container for a TrueVision Graphics format file.

    Format is as described here:
    http://local.wasp.uwa.edu.au/~pbourke/dataformats/tga/
    http://netghost.narod.ru/gff2/graphics/summary/tga.htm

    Usage is:
    \code
    QTgaFile tga(myFile);
    QImage tgaImage;
    if (tga.isValid())
        tgaImage = tga.readImage();
    \endcode

    The class is designed to handle sequential and non-sequential
    sources, so during construction the mHeader is read.  Then during
    the readImage() call the rest of the data is read.

    After passing myFile to the constructor, if the QIODevice *myFile
    is read, or has seek() called, the results are undefined - so don't
    do that.
*/

/*!
    Construct a new QTgaFile object getting data from \a device.

    The object does not take ownership of the \a device, but until the
    object is destroyed do not do any non-const operations, eg seek or
    read on the device.
*/
QTgaFile::QTgaFile(QIODevice *device)
    : mDevice(device)
{
    ::memset(mHeader, 0, HeaderSize);
    if (!mDevice->isReadable())
    {
        mErrorMessage = QObject::tr("Could not read image data");
        return;
    }
    if (mDevice->isSequential())
    {
        mErrorMessage = QObject::tr("Sequential device (eg socket) for image read not supported");
        return;
    }
    if (!mDevice->seek(0))
    {
        mErrorMessage = QObject::tr("Seek file/device for image read failed");
        return;
    }
    int bytes = device->read((char*)mHeader, HeaderSize);
    if (bytes != HeaderSize)
    {
        mErrorMessage = QObject::tr("Image mHeader read failed");
        device->seek(0);
        return;
    }
    if (mHeader[ImageType] != 2)
    {
        // TODO: should support other image types
        mErrorMessage = QObject::tr("Image type not supported");
        device->seek(0);
        return;
    }
    int bitsPerPixel = mHeader[PixelDepth];
    bool validDepth = (bitsPerPixel == 16 || bitsPerPixel == 24 || bitsPerPixel == 32);
    if (!validDepth)
    {
        mErrorMessage = QObject::tr("Image depth not valid");
    }
    int fileBytes = mDevice->size();
    if (!mDevice->seek(fileBytes - FooterSize))
    {
        mErrorMessage = QObject::tr("Could not seek to image read footer");
        device->seek(0);
        return;
    }
    char footer[FooterSize];
    bytes = mDevice->read((char*)footer, FooterSize);
    if (bytes != FooterSize)
    {
        mErrorMessage = QObject::tr("Could not read footer");
    }
    if (qstrncmp(&footer[SignatureOffset], "TRUEVISION-XFILE", 16) != 0)
    {
        mErrorMessage = QObject::tr("Image type (non-TrueVision 2.0) not supported");
    }
    if (!mDevice->seek(0))
    {
        mErrorMessage = QObject::tr("Could not reset to start position");
    }
}

/*!
    \internal
    Destroy the device, recovering any resources.
*/
QTgaFile::~QTgaFile()
{
}

/*!
    \internal
    Reads an image file from the QTgaFile's device, and returns it.

    This method seeks to the absolute position of the image data in the file,
    so no assumptions are made about where the devices read pointer is when this
    method is called.  For this reason only random access devices are supported.

    If the constructor completed successfully, such that isValid() returns true,
    then this method is likely to succeed, unless the file is somehow corrupted.

    In the case that the read fails, the QImage returned will be null, such that
    QImage::isNull() will be true.
*/
QImage QTgaFile::readImage()
{
    if (!isValid())
        return QImage();

    int offset = mHeader[IdLength];  // Mostly always zero

    // Even in TrueColor files a color pallette may be present
    if (mHeader[ColorMapType] == 1)
        offset += littleEndianInt(&mHeader[CMapLength]) * littleEndianInt(&mHeader[CMapDepth]);

    mDevice->seek(HeaderSize + offset);

    char dummy;
    for (int i = 0; i < offset; ++i)
        mDevice->getChar(&dummy);

    int bitsPerPixel = mHeader[PixelDepth];
    int imageWidth = width();
    int imageHeight = height();

    unsigned char desc = mHeader[ImageDescriptor];
    //unsigned char xCorner = desc & 0x10; // 0 = left, 1 = right
    unsigned char yCorner = desc & 0x20; // 0 = lower, 1 = upper

    QImage im(imageWidth, imageHeight, QImage::Format_ARGB32);
    TgaReader *reader = 0;
    if (bitsPerPixel == 16)
        reader = new Tga16Reader();
    else if (bitsPerPixel == 24)
        reader = new Tga24Reader();
    else if (bitsPerPixel == 32)
        reader = new Tga32Reader();
    TgaReader &read = *reader;

    // For now only deal with yCorner, since no one uses xCorner == 1
    // Also this is upside down, since Qt has the origin flipped
    if (yCorner)
    {
        for (int y = 0; y < imageHeight; ++y)
            for (int x = 0; x < imageWidth; ++x)
                im.setPixel(x, y, read(mDevice));
    }
    else
    {
        for (int y = imageHeight - 1; y >= 0; --y)
            for (int x = 0; x < imageWidth; ++x)
                im.setPixel(x, y, read(mDevice));
    }

    delete reader;

    // TODO: add processing of TGA extension information - ie TGA 2.0 files
    return im;
}
