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

#ifndef QVOLATILEIMAGEDATA_P_H
#define QVOLATILEIMAGEDATA_P_H

#include <QtGui/qimage.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QVolatileImageData : public QSharedData
{
 public:
   QVolatileImageData();
   QVolatileImageData(int w, int h, QImage::Format format);
   QVolatileImageData(const QImage &sourceImage);
   QVolatileImageData(void *nativeImage, void *nativeMask);
   QVolatileImageData(const QVolatileImageData &other);
   ~QVolatileImageData();

   void beginDataAccess() const;
   void endDataAccess(bool readOnly = false) const;
   bool ensureFormat(QImage::Format format);
   void *duplicateNativeImage() const;
   void ensureImage();

   QImage image;
   QPaintEngine *pengine;
};

QT_END_NAMESPACE

#endif
