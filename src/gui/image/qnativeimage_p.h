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

#ifndef QNATIVEIMAGE_P_H
#define QNATIVEIMAGE_P_H

#include <qimage.h>

class QWindow;

class QNativeImage
{
 public:
   QNativeImage(int width, int height, QImage::Format format, bool isTextBuffer = false, QWindow *window = nullptr);

   QNativeImage(const QNativeImage &) = delete;
   QNativeImage &operator=(const QNativeImage &) = delete;

   ~QNativeImage();

   inline int width() const;
   inline int height() const;

   QImage image;

   static QImage::Format systemFormat();
};

inline int QNativeImage::width() const
{
   return image.width();
}

inline int QNativeImage::height() const
{
   return image.height();
}

#endif