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

#ifndef QIMAGEWRITER_H
#define QIMAGEWRITER_H

#include <qbytearray.h>
#include <qcoreapplication.h>
#include <qlist.h>
#include <qimageiohandler.h>

class QIODevice;
class QImage;
class QImageWriterPrivate;

class Q_GUI_EXPORT QImageWriter
{
 public:
   enum ImageWriterError {
      UnknownError,
      DeviceError,
      UnsupportedFormatError
   };

   QImageWriter();
   explicit QImageWriter(QIODevice *device, const QString &format);
   explicit QImageWriter(const QString &fileName, const QString &format = QString());

   QImageWriter(const QImageWriter &) = delete;
   QImageWriter &operator=(const QImageWriter &) = delete;

   ~QImageWriter();

   void setFormat(const QString &format);
   QString format() const;

   void setDevice(QIODevice *device);
   QIODevice *device() const;

   void setFileName(const QString &fileName);
   QString fileName() const;

   void setQuality(int quality);
   int quality() const;

   void setCompression(int compression);
   int compression() const;

   void setGamma(float gamma);
   float gamma() const;

   void setSubType(const QByteArray &type);
   QByteArray subType() const;
   QList<QByteArray> supportedSubTypes() const;
   void setOptimizedWrite(bool optimize);
   bool optimizedWrite() const;
   void setProgressiveScanWrite(bool progressive);
   bool progressiveScanWrite() const;
   QImageIOHandler::Transformations transformation() const;
   void setTransformation(QImageIOHandler::Transformations transform);

   void setText(const QString &key, const QString &text);
   bool canWrite() const;
   bool write(const QImage &image);

   ImageWriterError error() const;
   QString errorString() const;

   bool supportsOption(QImageIOHandler::ImageOption option) const;

   static QList<QString> supportedImageFormats();
   static QList<QString> supportedMimeTypes();

 private:
   Q_DECLARE_TR_FUNCTIONS(QImageWriter)
   QImageWriterPrivate *d;
};

#endif
