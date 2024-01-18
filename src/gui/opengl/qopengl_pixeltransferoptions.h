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

#ifndef QOPENGLPIXELUPLOADOPTIONS_H
#define QOPENGLPIXELUPLOADOPTIONS_H

#include <qglobal.h>

#if !defined(QT_NO_OPENGL)

#include <QSharedDataPointer>

class QOpenGLPixelTransferOptionsData;

class Q_GUI_EXPORT QOpenGLPixelTransferOptions
{
public:
    QOpenGLPixelTransferOptions();
    QOpenGLPixelTransferOptions(const QOpenGLPixelTransferOptions &);

    QOpenGLPixelTransferOptions &operator=(QOpenGLPixelTransferOptions &&other)
    {
      swap(other);
      return *this;
    }

    QOpenGLPixelTransferOptions &operator=(const QOpenGLPixelTransferOptions &);
    ~QOpenGLPixelTransferOptions();

    void swap(QOpenGLPixelTransferOptions &other)
    { data.swap(other.data); }

    void setAlignment(int alignment);
    int alignment() const;

    void setSkipImages(int skipImages);
    int skipImages() const;

    void setSkipRows(int skipRows);
    int skipRows() const;

    void setSkipPixels(int skipPixels);
    int skipPixels() const;

    void setImageHeight(int imageHeight);
    int imageHeight() const;

    void setRowLength(int rowLength);
    int rowLength() const;

    void setLeastSignificantByteFirst(bool lsbFirst);
    bool isLeastSignificantBitFirst() const;

    void setSwapBytesEnabled(bool swapBytes);
    bool isSwapBytesEnabled() const;

private:
    QSharedDataPointer<QOpenGLPixelTransferOptionsData> data;
};

#endif // QT_NO_OPENGL

#endif
