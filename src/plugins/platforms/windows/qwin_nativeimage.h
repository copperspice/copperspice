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

#ifndef QWINDOWSNATIVEIMAGE_H
#define QWINDOWSNATIVEIMAGE_H

#include <qwin_additional.h>
#include <qimage.h>

class QWindowsNativeImage
{
 public:
   QWindowsNativeImage(int width, int height, QImage::Format format);

   QWindowsNativeImage(const QWindowsNativeImage &) = delete;
   QWindowsNativeImage &operator=(const QWindowsNativeImage &) = delete;

   ~QWindowsNativeImage();

   inline int width() const  {
      return m_image.width();
   }

   inline int height() const {
      return m_image.height();
   }

   QImage &image() {
      return m_image;
   }

   const QImage &image() const {
      return m_image;
   }

   HDC hdc() const {
      return m_hdc;
   }

   static QImage::Format systemFormat();

 private:
   const HDC m_hdc;
   QImage m_image;

   HBITMAP m_bitmap;
   HBITMAP m_null_bitmap;
};

#endif
