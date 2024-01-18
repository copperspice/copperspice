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

#ifndef QIMAGEIOHANDLER_H
#define QIMAGEIOHANDLER_H

#include <qiodevice.h>
#include <qplugin.h>
#include <qfactoryinterface.h>
#include <qscopedpointer.h>

class QImage;
class QRect;
class QSize;
class QVariant;
class QImageIOHandlerPrivate;

#define QImageIOHandlerInterface_ID "com.copperspice.CS.ImageIOHandlerInterface"

class Q_GUI_EXPORT QImageIOHandler
{
 public:
   enum ImageOption {
      Size,
      ClipRect,
      Description,
      ScaledClipRect,
      ScaledSize,
      CompressionRatio,
      Gamma,
      Quality,
      Name,
      SubType,
      IncrementalReading,
      Endianness,
      Animation,
      BackgroundColor,
      ImageFormat,
      SupportedSubTypes,
      OptimizedWrite,
      ProgressiveScanWrite,
      ImageTransformation,
      TransformedByDefault
   };

   enum Transformation {
      TransformationNone      = 0,
      TransformationMirror    = 1,
      TransformationFlip      = 2,
      TransformationRotate180 = TransformationMirror | TransformationFlip,
      TransformationRotate90  = 4,
      TransformationMirrorAndRotate90 = TransformationMirror | TransformationRotate90,
      TransformationFlipAndRotate90   = TransformationFlip | TransformationRotate90,
      TransformationRotate270         = TransformationRotate180 | TransformationRotate90
   };

   using Transformations = QFlags<Transformation>;

   QImageIOHandler();

   QImageIOHandler(const QImageIOHandler &) = delete;
   QImageIOHandler &operator=(const QImageIOHandler &) = delete;

   virtual ~QImageIOHandler();

   void setDevice(QIODevice *device);
   QIODevice *device() const;

   void setFormat(const QString &format);
   QString format() const;

   virtual QString name() const;

   virtual bool canRead() = 0;
   virtual bool read(QImage *image) = 0;
   virtual bool write(const QImage &image);

   virtual QVariant option(ImageOption option);
   virtual void setOption(ImageOption option, const QVariant &value);
   virtual bool supportsOption(ImageOption option) const;

   // incremental loading
   virtual bool jumpToNextImage();
   virtual bool jumpToImage(int imageNumber);
   virtual int loopCount() const;
   virtual int imageCount();
   virtual int nextImageDelay() const;
   virtual int currentImageNumber() const;
   virtual QRect currentImageRect() const;

 protected:
   QImageIOHandler(QImageIOHandlerPrivate &dd);
   QScopedPointer<QImageIOHandlerPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QImageIOHandler)
};

class Q_GUI_EXPORT QImageIOPlugin : public QObject
{
   GUI_CS_OBJECT(QImageIOPlugin)

 public:
   explicit QImageIOPlugin(QObject *parent = nullptr);
   virtual ~QImageIOPlugin();

   enum Capability {
      CanRead            = 0x1,
      CanWrite           = 0x2,
      CanReadIncremental = 0x4
   };
   using Capabilities = QFlags<Capability>;

   virtual Capabilities capabilities(QIODevice *device, const QString &format) const = 0;
   virtual QImageIOHandler *create(QIODevice *device, const QString &format = QString()) const = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QImageIOPlugin::Capabilities)

#endif
