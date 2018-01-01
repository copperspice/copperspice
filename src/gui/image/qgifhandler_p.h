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

#ifndef QGIFHANDLER_P_H
#define QGIFHANDLER_P_H

#include <QtGui/qimageiohandler.h>
#include <QtGui/qimage.h>
#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

class QGIFFormat;

class QGifHandler : public QImageIOHandler
{
 public:
   QGifHandler();
   ~QGifHandler();

   bool canRead() const override;
   bool read(QImage *image) override;
   bool write(const QImage &image) override;

   QByteArray name() const override;

   static bool canRead(QIODevice *device);

   QVariant option(ImageOption option) const override;
   void setOption(ImageOption option, const QVariant &value) override;
   bool supportsOption(ImageOption option) const override;

   int imageCount() const override;
   int loopCount() const override;
   int nextImageDelay() const override;
   int currentImageNumber() const override;

 private:
   bool imageIsComing() const;
   QGIFFormat *gifFormat;
   QString fileName;
   mutable QByteArray buffer;
   mutable QImage lastImage;

   mutable int nextDelay;
   mutable int loopCnt;
   int frameNumber;
   mutable QVector<QSize> imageSizes;
   mutable bool scanIsCached;
};

QT_END_NAMESPACE

#endif // QGIFHANDLER_P_H
