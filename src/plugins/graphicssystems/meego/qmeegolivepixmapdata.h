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

#ifndef MLIVEPIXMAPDATA_H
#define MLIVEPIXMAPDATA_H

#include <QLinkedList>
#include <private/qpixmapdata_gl_p.h>
#include "qmeegoextensions.h"

class QMeeGoLivePixmapData;
typedef QLinkedList<QMeeGoLivePixmapData *> QMeeGoLivePixmapDataList;

class QMeeGoLivePixmapData : public QGLPixmapData
{
public:
    QMeeGoLivePixmapData(int w, int h, QImage::Format format);
    QMeeGoLivePixmapData(Qt::HANDLE h);
    ~QMeeGoLivePixmapData();

    QPixmapData *createCompatiblePixmapData() const;
    bool scroll(int dx, int dy, const QRect &rect);

    void initializeThroughEGLImage();

    QImage* lock(EGLSyncKHR fenceSync);
    bool release(QImage *img);
    Qt::HANDLE handle();

    EGLSurface getSurfaceForBackingPixmap();
    void destroySurfaceForPixmapData(QPixmapData* pmd);

    QPixmap *backingX11Pixmap;
    QImage lockedImage;
    QMeeGoLivePixmapDataList::Iterator pos;

    static void invalidateSurfaces();
};

#endif
