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

#ifndef QNATIVEIMAGE_P_H
#define QNATIVEIMAGE_P_H

#include <qimage.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>

#elif defined(Q_WS_X11)
#include <qt_x11_p.h>

#elif defined(Q_OS_MAC)
#include <qt_mac_p.h>

#endif

QT_BEGIN_NAMESPACE

class QWidget;

class QNativeImage
{
 public:
   QNativeImage(int width, int height, QImage::Format format, bool isTextBuffer = false, QWidget *widget = 0);
   ~QNativeImage();

   inline int width() const;
   inline int height() const;

   QImage image;

   static QImage::Format systemFormat();

#ifdef Q_OS_WIN
   HDC hdc;
   HBITMAP bitmap;
   HBITMAP null_bitmap;

#elif defined(Q_WS_X11) && !defined(QT_NO_MITSHM)
   XImage *xshmimg;
   Pixmap xshmpm;
   XShmSegmentInfo xshminfo;

#elif defined(Q_OS_MAC)
   CGContextRef cg;
#endif

 private:
   Q_DISABLE_COPY(QNativeImage)
};

inline int QNativeImage::width() const
{
   return image.width();
}
inline int QNativeImage::height() const
{
   return image.height();
}

QT_END_NAMESPACE

#endif // QNATIVEIMAGE_P_H
