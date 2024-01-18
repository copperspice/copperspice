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

#include "mediasamplevideobuffer.h"

MediaSampleVideoBuffer::MediaSampleVideoBuffer(IMediaSample *sample, int bytesPerLine)
   : QAbstractVideoBuffer(NoHandle)
   , m_sample(sample)
   , m_bytesPerLine(bytesPerLine)
   , m_mapMode(NotMapped)
{
   m_sample->AddRef();
}

MediaSampleVideoBuffer::~MediaSampleVideoBuffer()
{
   m_sample->Release();
}

uchar *MediaSampleVideoBuffer::map(MapMode mode, int *numBytes, int *bytesPerLine)
{
   if (m_mapMode == NotMapped && mode != NotMapped) {
      if (numBytes) {
         *numBytes = m_sample->GetActualDataLength();
      }

      if (bytesPerLine) {
         *bytesPerLine = m_bytesPerLine;
      }

      BYTE *bytes = nullptr;

      if (m_sample->GetPointer(&bytes) == S_OK) {
         m_mapMode = mode;

         return reinterpret_cast<uchar *>(bytes);
      }
   }

   return nullptr;
}

void MediaSampleVideoBuffer::unmap()
{
   m_mapMode = NotMapped;
}

QAbstractVideoBuffer::MapMode MediaSampleVideoBuffer::mapMode() const
{
   return m_mapMode;
}
