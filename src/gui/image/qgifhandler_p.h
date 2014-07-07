/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
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

   bool canRead() const;
   bool read(QImage *image);
   bool write(const QImage &image);

   QByteArray name() const;

   static bool canRead(QIODevice *device);

   QVariant option(ImageOption option) const;
   void setOption(ImageOption option, const QVariant &value);
   bool supportsOption(ImageOption option) const;

   int imageCount() const;
   int loopCount() const;
   int nextImageDelay() const;
   int currentImageNumber() const;

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
