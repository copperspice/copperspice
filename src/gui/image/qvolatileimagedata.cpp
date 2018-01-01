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

#include <qvolatileimagedata_p.h>
#include <QtGui/qpaintengine.h>

QT_BEGIN_NAMESPACE

QVolatileImageData::QVolatileImageData()
   : pengine(0)
{
}

QVolatileImageData::QVolatileImageData(int w, int h, QImage::Format format)
   : pengine(0)
{
   image = QImage(w, h, format);
}

QVolatileImageData::QVolatileImageData(const QImage &sourceImage)
   : pengine(0)
{
   image = sourceImage;
}

QVolatileImageData::QVolatileImageData(void *, void *)
   : pengine(0)
{
   // Not supported.
}

QVolatileImageData::QVolatileImageData(const QVolatileImageData &other)
   : QSharedData()
{
   image = other.image;
   // The detach is not mandatory here but we do it nonetheless in order to
   // keep the behavior consistent with other platforms.
   image.detach();
   pengine = 0;
}

QVolatileImageData::~QVolatileImageData()
{
   delete pengine;
}

void QVolatileImageData::beginDataAccess() const
{
   // nothing to do here
}

void QVolatileImageData::endDataAccess(bool readOnly) const
{
   Q_UNUSED(readOnly);
   // nothing to do here
}

bool QVolatileImageData::ensureFormat(QImage::Format format)
{
   if (image.format() != format) {
      image = image.convertToFormat(format);
   }
   return true;
}

void *QVolatileImageData::duplicateNativeImage() const
{
   return 0;
}

void QVolatileImageData::ensureImage()
{
   // nothing to do here
}

QT_END_NAMESPACE
