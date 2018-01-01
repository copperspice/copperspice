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

#ifndef QIMAGEIOHANDLER_H
#define QIMAGEIOHANDLER_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QImage;
class QRect;
class QSize;
class QVariant;
class QImageIOHandlerPrivate;

class Q_GUI_EXPORT QImageIOHandler
{
   Q_DECLARE_PRIVATE(QImageIOHandler)

 public:
   QImageIOHandler();
   virtual ~QImageIOHandler();

   void setDevice(QIODevice *device);
   QIODevice *device() const;

   void setFormat(const QByteArray &format);
   void setFormat(const QByteArray &format) const;
   QByteArray format() const;

   virtual QByteArray name() const;

   virtual bool canRead() const = 0;
   virtual bool read(QImage *image) = 0;
   virtual bool write(const QImage &image);

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
      ImageFormat
   };
   virtual QVariant option(ImageOption option) const;
   virtual void setOption(ImageOption option, const QVariant &value);
   virtual bool supportsOption(ImageOption option) const;

   // incremental loading
   virtual bool jumpToNextImage();
   virtual bool jumpToImage(int imageNumber);
   virtual int loopCount() const;
   virtual int imageCount() const;
   virtual int nextImageDelay() const;
   virtual int currentImageNumber() const;
   virtual QRect currentImageRect() const;

 protected:
   QImageIOHandler(QImageIOHandlerPrivate &dd);
   QScopedPointer<QImageIOHandlerPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QImageIOHandler)
};

struct Q_GUI_EXPORT QImageIOHandlerFactoryInterface : public QFactoryInterface {
   virtual QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const = 0;
};

#define QImageIOHandlerFactoryInterface_iid "com.copperspice.QImageIOHandlerFactoryInterface"
CS_DECLARE_INTERFACE(QImageIOHandlerFactoryInterface, QImageIOHandlerFactoryInterface_iid)

class Q_GUI_EXPORT QImageIOPlugin : public QObject, public QImageIOHandlerFactoryInterface
{
   GUI_CS_OBJECT_MULTIPLE(QImageIOPlugin, QObject)
   CS_INTERFACES(QImageIOHandlerFactoryInterface, QFactoryInterface)

 public:
   explicit QImageIOPlugin(QObject *parent = nullptr);
   virtual ~QImageIOPlugin();

   enum Capability {
      CanRead = 0x1,
      CanWrite = 0x2,
      CanReadIncremental = 0x4
   };
   using Capabilities = QFlags<Capability>;

   virtual Capabilities capabilities(QIODevice *device, const QByteArray &format) const = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QImageIOPlugin::Capabilities)

QT_END_NAMESPACE

#endif // QIMAGEIOHANDLER_H
