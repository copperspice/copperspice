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

#ifndef DIRECTSHOWMEDIATYPE_H
#define DIRECTSHOWMEDIATYPE_H

#include <dshow.h>

#include <qvideosurfaceformat.h>

#include <dvdmedia.h>

class DirectShowMediaType : public AM_MEDIA_TYPE
{
 public:
   DirectShowMediaType() {
      memset(this, 0, sizeof(DirectShowMediaType));
   }
   DirectShowMediaType(const AM_MEDIA_TYPE &type) {
      copy(this, type);
   }
   DirectShowMediaType(const DirectShowMediaType &other) {
      copy(this, other);
   }
   DirectShowMediaType &operator =(const AM_MEDIA_TYPE &type) {
      freeData(this);
      copy(this, type);
      return *this;
   }
   DirectShowMediaType &operator =(const DirectShowMediaType &other) {
      freeData(this);
      copy(this, other);
      return *this;
   }
   ~DirectShowMediaType() {
      freeData(this);
   }

   void clear() {
      freeData(this);
      memset(this, 0, sizeof(DirectShowMediaType));
   }

   static void copy(AM_MEDIA_TYPE *target, const AM_MEDIA_TYPE &source);
   static void freeData(AM_MEDIA_TYPE *type);
   static void deleteType(AM_MEDIA_TYPE *type);

   static GUID convertPixelFormat(QVideoFrame::PixelFormat format);
   static QVideoSurfaceFormat formatFromType(const AM_MEDIA_TYPE &type);

   static int bytesPerLine(const QVideoSurfaceFormat &format);

 private:
   static QVideoSurfaceFormat::Direction scanLineDirection(QVideoFrame::PixelFormat pixelFormat, const BITMAPINFOHEADER &bmiHeader);
};

#endif
