/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
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

#include <qopenglpixeltransferoptions.h>
#include <QSharedData>

class QOpenGLPixelTransferOptionsData : public QSharedData
{
public:
    QOpenGLPixelTransferOptionsData()
        : alignment(4)
        , skipImages(0)
        , skipRows(0)
        , skipPixels(0)
        , imageHeight(0)
        , rowLength(0)
        , lsbFirst(false)
        , swapBytes(false)
    {}

    int alignment;
    int skipImages;
    int skipRows;
    int skipPixels;
    int imageHeight;
    int rowLength;
    bool lsbFirst;
    bool swapBytes;
};

/*!
 * Constructs a new QOpenGLPixelTransferOptions instance with the default settings.
 */
QOpenGLPixelTransferOptions::QOpenGLPixelTransferOptions()
    : data(new QOpenGLPixelTransferOptionsData)
{
}

/*!
 * \internal
 */
QOpenGLPixelTransferOptions::QOpenGLPixelTransferOptions(const QOpenGLPixelTransferOptions &rhs)
    : data(rhs.data)
{
}

/*!
 * \internal
 */
QOpenGLPixelTransferOptions &QOpenGLPixelTransferOptions::operator=(const QOpenGLPixelTransferOptions &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

/*!
 * Destructor.
 */
QOpenGLPixelTransferOptions::~QOpenGLPixelTransferOptions()
{
}

/*!
 * Sets the \a alignment requirements for each pixel row. Corresponds to \c GL_UNPACK_ALIGNMENT.
 * The default value is 4, as specified by OpenGL.
 */
void QOpenGLPixelTransferOptions::setAlignment(int alignment)
{
    data->alignment = alignment;
}

/*!
 * \return the current alignment requirement for each pixel row.
 */
int QOpenGLPixelTransferOptions::alignment() const
{
    return data->alignment;
}

/*!
 * Sets the number of images that are skipped to \a skipImages.
 * Corresponds to \c GL_UNPACK_SKIP_IMAGES. Equivalent to incrementing the pointer
 * passed to QOpenGLTexture::setData(). The default value is 0.
 */
void QOpenGLPixelTransferOptions::setSkipImages(int skipImages)
{
    data->skipImages = skipImages;
}

/*!
 * \return the number of images that are skipped.
 */
int QOpenGLPixelTransferOptions::skipImages() const
{
    return data->skipImages;
}

/*!
 * Sets the number of rows that are skipped to \a skipRows.
 * Corresponds to \c GL_UNPACK_SKIP_ROWS. Equivalent to incrementing the pointer
 * passed to QOpenGLTexture::setData(). The default value is 0.
 */
void QOpenGLPixelTransferOptions::setSkipRows(int skipRows)
{
    data->skipRows = skipRows;
}

/*!
 * \return the number of rows that are skipped.
 */
int QOpenGLPixelTransferOptions::skipRows() const
{
    return data->skipRows;
}

/*!
 * Sets the number of pixels that are skipped to \a skipPixels.
 * Corresponds to \c GL_UNPACK_SKIP_PIXELS. Equivalent to incrementing the pointer
 * passed to QOpenGLTexture::setData(). The default value is 0.
 */
void QOpenGLPixelTransferOptions::setSkipPixels(int skipPixels)
{
    data->skipPixels = skipPixels;
}

/*!
 * \return the number of pixels that are skipped.
 */
int QOpenGLPixelTransferOptions::skipPixels() const
{
    return data->skipPixels;
}

/*!
 * Sets the image height for 3D textures to \a imageHeight.
 * Corresponds to \c GL_UNPACK_IMAGE_HEIGHT.
 * The default value is 0.
 */
void QOpenGLPixelTransferOptions::setImageHeight(int imageHeight)
{
    data->imageHeight = imageHeight;
}

/*!
 * \return the currently set image height.
 */
int QOpenGLPixelTransferOptions::imageHeight() const
{
    return data->imageHeight;
}

/*!
 * Sets the number of pixels in a row to \a rowLength.
 * Corresponds to \c GL_UNPACK_ROW_LENGTH.
 * The default value is 0.
 */
void QOpenGLPixelTransferOptions::setRowLength(int rowLength)
{
    data->rowLength = rowLength;
}

/*!
 * \return the currently set row length.
 */
int QOpenGLPixelTransferOptions::rowLength() const
{
    return data->rowLength;
}

/*!
 * \a lsbFirst specifies if bits within a byte are ordered from least to most significat.
 * The default value is \c false, meaning that the first bit in each byte is the
 * most significant one. This is significant for bitmap data only.
 * Corresponds to \c GL_UNPACK_LSB_FIRST.
 */
void QOpenGLPixelTransferOptions::setLeastSignificantByteFirst(bool lsbFirst)
{
    data->lsbFirst = lsbFirst;
}

/*!
 * \return \c true if bits within a byte are ordered from least to most significant.
 */
bool QOpenGLPixelTransferOptions::isLeastSignificantBitFirst() const
{
    return data->lsbFirst;
}

/*!
 * \a swapBytes specifies if the byte ordering for multibyte components is reversed.
 * The default value is \c false.
 * Corresponds to \c GL_UNPACK_SWAP_BYTES.
 */
void QOpenGLPixelTransferOptions::setSwapBytesEnabled(bool swapBytes)
{
    data->swapBytes = swapBytes;
}

/*!
 * \return \c true if the byte ordering for multibyte components is reversed.
 */
bool QOpenGLPixelTransferOptions::isSwapBytesEnabled() const
{
    return data->swapBytes;
}

