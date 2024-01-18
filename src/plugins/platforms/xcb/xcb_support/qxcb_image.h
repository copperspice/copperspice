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

#ifndef QXCB_IMAGE_H
#define QXCB_IMAGE_H

#include <qxcb_screen.h>
#include <qpair.h>
#include <qimage.h>
#include <qpixmap.h>

#include <xcb/xcb_image.h>

QImage::Format qt_xcb_imageFormatForVisual(QXcbConnection *connection,
   uint8_t depth, const xcb_visualtype_t *visual);

QPixmap qt_xcb_pixmapFromXPixmap(QXcbConnection *connection, xcb_pixmap_t pixmap,
   int width, int height, int depth, const xcb_visualtype_t *visual);

xcb_pixmap_t qt_xcb_XPixmapFromBitmap(QXcbScreen *screen, const QImage &image);
xcb_cursor_t qt_xcb_createCursorXRender(QXcbScreen *screen, const QImage &image, const QPoint &spot);


#endif
