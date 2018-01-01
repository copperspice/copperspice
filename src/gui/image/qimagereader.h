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

#ifndef QIMAGEREADER_H
#define QIMAGEREADER_H

#include <QtCore/qbytearray.h>
#include <QtGui/qimage.h>
#include <QtGui/qimageiohandler.h>

QT_BEGIN_NAMESPACE

class QColor;
class QIODevice;
class QRect;
class QSize;
class QStringList;
class QImageReaderPrivate;

class Q_GUI_EXPORT QImageReader
{

 public:
   enum ImageReaderError {
      UnknownError,
      FileNotFoundError,
      DeviceError,
      UnsupportedFormatError,
      InvalidDataError
   };

   QImageReader();
   explicit QImageReader(QIODevice *device, const QByteArray &format = QByteArray());
   explicit QImageReader(const QString &fileName, const QByteArray &format = QByteArray());
   ~QImageReader();

   void setFormat(const QByteArray &format);
   QByteArray format() const;

   void setAutoDetectImageFormat(bool enabled);
   bool autoDetectImageFormat() const;

   void setDecideFormatFromContent(bool ignored);
   bool decideFormatFromContent() const;

   void setDevice(QIODevice *device);
   QIODevice *device() const;

   void setFileName(const QString &fileName);
   QString fileName() const;

   QSize size() const;

   QImage::Format imageFormat() const;

   QStringList textKeys() const;
   QString text(const QString &key) const;

   void setClipRect(const QRect &rect);
   QRect clipRect() const;

   void setScaledSize(const QSize &size);
   QSize scaledSize() const;

   void setQuality(int quality);
   int quality() const;

   void setScaledClipRect(const QRect &rect);
   QRect scaledClipRect() const;

   void setBackgroundColor(const QColor &color);
   QColor backgroundColor() const;

   bool supportsAnimation() const;

   bool canRead() const;
   QImage read();
   bool read(QImage *image);

   bool jumpToNextImage();
   bool jumpToImage(int imageNumber);
   int loopCount() const;
   int imageCount() const;
   int nextImageDelay() const;
   int currentImageNumber() const;
   QRect currentImageRect() const;

   ImageReaderError error() const;
   QString errorString() const;

   bool supportsOption(QImageIOHandler::ImageOption option) const;

   static QByteArray imageFormat(const QString &fileName);
   static QByteArray imageFormat(QIODevice *device);
   static QList<QByteArray> supportedImageFormats();

 private:
   Q_DISABLE_COPY(QImageReader)
   QImageReaderPrivate *d;
};

QT_END_NAMESPACE

#endif // QIMAGEREADER_H
