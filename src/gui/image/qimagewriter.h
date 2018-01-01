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

#ifndef QIMAGEWRITER_H
#define QIMAGEWRITER_H

#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtGui/qimageiohandler.h>

QT_BEGIN_NAMESPACE

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
   explicit QImageWriter(QIODevice *device, const QByteArray &format);
   explicit QImageWriter(const QString &fileName, const QByteArray &format = QByteArray());
   ~QImageWriter();

   void setFormat(const QByteArray &format);
   QByteArray format() const;

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

   // Obsolete as of 4.1
   void setDescription(const QString &description);
   QString description() const;

   void setText(const QString &key, const QString &text);

   bool canWrite() const;
   bool write(const QImage &image);

   ImageWriterError error() const;
   QString errorString() const;

   bool supportsOption(QImageIOHandler::ImageOption option) const;

   static QList<QByteArray> supportedImageFormats();

 private:
   Q_DISABLE_COPY(QImageWriter)
   QImageWriterPrivate *d;
};

QT_END_NAMESPACE

#endif // QIMAGEWRITER_H
