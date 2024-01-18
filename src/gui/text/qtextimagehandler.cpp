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

#include <qtextimagehandler_p.h>
#include <qapplication.h>
#include <qtextformat.h>
#include <qpainter.h>
#include <qdebug.h>
#include <qpalette.h>
#include <qthread.h>

#include <qtextengine_p.h>

extern QString qt_findAtNxFile(const QString &baseFileName, qreal targetDevicePixelRatio,
   qreal *sourceDevicePixelRatio);

static QString resolveFileName(QString fileName, QUrl *url, qreal targetDevicePixelRatio,
   qreal *sourceDevicePixelRatio)
{
   // We might use the fileName for loading if url loading fails
   // try to make sure it is a valid file path.
   // Also, QFile{Info}::exists works only on filepaths (not urls)

   if (url->isValid()) {
      if (url->scheme() == "qrc") {
         fileName = fileName.right(fileName.length() - 3);
      } else if (url->scheme() == "file") {
         fileName = url->toLocalFile();
      }
   }

   if (targetDevicePixelRatio <= 1.0) {
      return fileName;
   }

   // try to find a Nx version
   return qt_findAtNxFile(fileName, targetDevicePixelRatio, sourceDevicePixelRatio);
}

static QPixmap getPixmap(QTextDocument *doc, const QTextImageFormat &format, const qreal devicePixelRatio = 1.0)
{
   QPixmap pm;

   QString name = format.name();
   if (name.startsWith(QLatin1String(":/"))) {
      // auto-detect resources
      name.prepend(QLatin1String("qrc"));
   }

   QUrl url = QUrl::fromEncoded(name.toUtf8());

   qreal sourcePixelRatio = 1.0;
   name = resolveFileName(name, &url, devicePixelRatio, &sourcePixelRatio);

   const QVariant data = doc->resource(QTextDocument::ImageResource, url);

   if (data.type() == QVariant::Pixmap || data.type() == QVariant::Image) {
      pm = data.value<QPixmap>();

   } else if (data.type() == QVariant::ByteArray) {
      pm.loadFromData(data.toByteArray());
   }

   if (pm.isNull()) {
      QImage img;

      if (name.isEmpty() || !img.load(name)) {
         return QPixmap(QLatin1String(":/copperspice/styles/commonstyle/images/file-16.png"));
      }

      pm = QPixmap::fromImage(img);
      doc->addResource(QTextDocument::ImageResource, url, pm);
   }

   return pm;
}

static QSize getPixmapSize(QTextDocument *doc, const QTextImageFormat &format)
{
   QPixmap pm;

   const bool hasWidth  = format.hasProperty(QTextFormat::ImageWidth);
   const int width      = qRound(format.width());
   const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
   const int height     = qRound(format.height());

   QSize size(width, height);
   if (!hasWidth || !hasHeight) {
      pm = getPixmap(doc, format);

      const int pmWidth = pm.width() / pm.devicePixelRatio();
      const int pmHeight = pm.height() / pm.devicePixelRatio();
      if (!hasWidth) {
         if (!hasHeight) {
            size.setWidth(pmWidth);
         } else {
            size.setWidth(qRound(height * (pmWidth / (qreal) pmHeight)));
         }
      }

      if (!hasHeight) {
         if (!hasWidth) {
            size.setHeight(pmHeight);
         } else {
            size.setHeight(qRound(width * (pmHeight / (qreal) pmWidth)));
         }
      }
   }

   qreal scale = 1.0;
   QPaintDevice *pdev = doc->documentLayout()->paintDevice();
   if (pdev) {
      if (pm.isNull()) {
         pm = getPixmap(doc, format);
      }

      if (!pm.isNull()) {
         scale = qreal(pdev->logicalDpiY()) / qreal(qt_defaultDpi());
      }
   }
   size *= scale;

   return size;
}

static QImage getImage(QTextDocument *doc, const QTextImageFormat &format, const qreal devicePixelRatio = 1.0)
{
   QImage image;

   QString name = format.name();
   if (name.startsWith(QLatin1String(":/"))) {
      // auto-detect resources
      name.prepend(QLatin1String("qrc"));
   }

   QUrl url = QUrl::fromEncoded(name.toUtf8());
   qreal sourcePixelRatio = 1.0;
   name = resolveFileName(name, &url, devicePixelRatio, &sourcePixelRatio);

   const QVariant data = doc->resource(QTextDocument::ImageResource, url);

   if (data.type() == QVariant::Image) {
      image = data.value<QImage>();

   } else if (data.type() == QVariant::ByteArray) {
      image.loadFromData(data.toByteArray());

   }

   if (image.isNull()) {
      if (name.isEmpty() || !image.load(name)) {
         return QImage(QLatin1String(":/copperspice/styles/commonstyle/images/file-16.png"));
      }

      doc->addResource(QTextDocument::ImageResource, url, image);
   }
   if (sourcePixelRatio != 1.0) {
      image.setDevicePixelRatio(sourcePixelRatio);
   }

   return image;
}

static QSize getImageSize(QTextDocument *doc, const QTextImageFormat &format)
{
   QImage image;

   const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
   const int width = qRound(format.width());
   const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
   const int height = qRound(format.height());

   QSize size(width, height);
   if (!hasWidth || !hasHeight) {
      image = getImage(doc, format);
      if (!hasWidth) {
         size.setWidth(image.width() / image.devicePixelRatio());
      }

      if (!hasHeight) {
         size.setHeight(image.height() / image.devicePixelRatio());
      }
   }

   qreal scale = 1.0;
   QPaintDevice *pdev = doc->documentLayout()->paintDevice();

   if (pdev) {
      if (image.isNull()) {
         image = getImage(doc, format);
      }
      if (!image.isNull()) {
         scale = qreal(pdev->logicalDpiY()) / qreal(qt_defaultDpi());
      }
   }
   size *= scale;

   return size;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
   : QObject(parent)
{
}

QSizeF QTextImageHandler::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
   (void) posInDocument;

   const QTextImageFormat imageFormat = format.toImageFormat();

   if (QCoreApplication::instance()->thread() != QThread::currentThread()) {
      return getImageSize(doc, imageFormat);
   }

   return getPixmapSize(doc, imageFormat);
}

QImage QTextImageHandler::image(QTextDocument *doc, const QTextImageFormat &imageFormat)
{
   Q_ASSERT(doc != nullptr);

   return getImage(doc, imageFormat);
}

void QTextImageHandler::drawObject(QPainter *p, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
   (void) posInDocument;
   const QTextImageFormat imageFormat = format.toImageFormat();

   if (QCoreApplication::instance()->thread() != QThread::currentThread()) {
      const QImage image = getImage(doc, imageFormat, p->device()->devicePixelRatioF());
      p->drawImage(rect, image, image.rect());

   } else {
      const QPixmap pixmap = getPixmap(doc, imageFormat, p->device()->devicePixelRatioF());
      p->drawPixmap(rect, pixmap, pixmap.rect());
   }
}


