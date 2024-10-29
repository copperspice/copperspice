/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <qopengl_pixeltransferoptions.h>

#include <qshareddata.h>

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

QOpenGLPixelTransferOptions::QOpenGLPixelTransferOptions()
    : data(new QOpenGLPixelTransferOptionsData)
{
}

QOpenGLPixelTransferOptions::QOpenGLPixelTransferOptions(const QOpenGLPixelTransferOptions &rhs)
    : data(rhs.data)
{
}

QOpenGLPixelTransferOptions &QOpenGLPixelTransferOptions::operator=(const QOpenGLPixelTransferOptions &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QOpenGLPixelTransferOptions::~QOpenGLPixelTransferOptions()
{
}

void QOpenGLPixelTransferOptions::setAlignment(int alignment)
{
    data->alignment = alignment;
}

int QOpenGLPixelTransferOptions::alignment() const
{
    return data->alignment;
}

void QOpenGLPixelTransferOptions::setSkipImages(int skipImages)
{
    data->skipImages = skipImages;
}

int QOpenGLPixelTransferOptions::skipImages() const
{
    return data->skipImages;
}

void QOpenGLPixelTransferOptions::setSkipRows(int skipRows)
{
    data->skipRows = skipRows;
}

int QOpenGLPixelTransferOptions::skipRows() const
{
    return data->skipRows;
}

void QOpenGLPixelTransferOptions::setSkipPixels(int skipPixels)
{
    data->skipPixels = skipPixels;
}

int QOpenGLPixelTransferOptions::skipPixels() const
{
    return data->skipPixels;
}

void QOpenGLPixelTransferOptions::setImageHeight(int imageHeight)
{
    data->imageHeight = imageHeight;
}

int QOpenGLPixelTransferOptions::imageHeight() const
{
    return data->imageHeight;
}

void QOpenGLPixelTransferOptions::setRowLength(int rowLength)
{
    data->rowLength = rowLength;
}

int QOpenGLPixelTransferOptions::rowLength() const
{
    return data->rowLength;
}

void QOpenGLPixelTransferOptions::setLeastSignificantByteFirst(bool lsbFirst)
{
    data->lsbFirst = lsbFirst;
}

bool QOpenGLPixelTransferOptions::isLeastSignificantBitFirst() const
{
    return data->lsbFirst;
}

void QOpenGLPixelTransferOptions::setSwapBytesEnabled(bool swapBytes)
{
    data->swapBytes = swapBytes;
}

bool QOpenGLPixelTransferOptions::isSwapBytesEnabled() const
{
    return data->swapBytes;
}
