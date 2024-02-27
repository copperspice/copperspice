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

#ifndef MEDIASAMPLEVIDEOBUFFER_H
#define MEDIASAMPLEVIDEOBUFFER_H

#include <dshow.h>

#include <qabstractvideobuffer.h>

class MediaSampleVideoBuffer : public QAbstractVideoBuffer
{
 public:
   MediaSampleVideoBuffer(IMediaSample *sample, int bytesPerLine);
   ~MediaSampleVideoBuffer();

   IMediaSample *sample() {
      return m_sample;
   }

   uchar *map(MapMode mode, int *numBytes, int *bytesPerLine) override;
   void unmap() override;

   MapMode mapMode() const override;

 private:
   IMediaSample *m_sample;
   int m_bytesPerLine;
   MapMode m_mapMode;
};


#endif
