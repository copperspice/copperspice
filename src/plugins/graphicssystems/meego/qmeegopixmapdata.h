/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef MPIXMAPDATA_H
#define MPIXMAPDATA_H

#include <private/qpixmapdata_gl_p.h>

struct QMeeGoImageInfo
{
    Qt::HANDLE handle;
    QImage::Format rawFormat;
};

class QMeeGoPixmapData : public QGLPixmapData
{
public:
    QMeeGoPixmapData();
    void fromTexture(GLuint textureId, int w, int h, bool alpha);
    QPixmapData *createCompatiblePixmapData() const;

    virtual void fromEGLSharedImage(Qt::HANDLE handle, const QImage &softImage);
    virtual void fromImage (const QImage &image, Qt::ImageConversionFlags flags);
    virtual QImage toImage() const;
    virtual void updateFromSoftImage();

    QImage softImage;

    static QHash <void*, QMeeGoImageInfo*> sharedImagesMap;

    static Qt::HANDLE imageToEGLSharedImage(const QImage &image);
    static bool destroyEGLSharedImage(Qt::HANDLE h);
    static void registerSharedImage(Qt::HANDLE handle, const QImage &si);
};

#endif
