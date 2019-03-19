/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
#include <qtextengine_p.h>
#include <qpalette.h>
#include <qtextbrowser.h>
#include <qthread.h>

QT_BEGIN_NAMESPACE

static QPixmap getPixmap(QTextDocument *doc, const QTextImageFormat &format)
{
   QPixmap pm;

   QString name = format.name();
   if (name.startsWith(QLatin1String(":/"))) { // auto-detect resources
      name.prepend(QLatin1String("qrc"));
   }

   QUrl url = QUrl::fromEncoded(name.toUtf8());
   const QVariant data = doc->resource(QTextDocument::ImageResource, url);

   if (data.type() == QVariant::Pixmap || data.type() == QVariant::Image) {
      pm = qvariant_cast<QPixmap>(data);
   } else if (data.type() == QVariant::ByteArray) {
      pm.loadFromData(data.toByteArray());
   }

   if (pm.isNull()) {
      QString context;

#ifndef QT_NO_TEXTBROWSER
      QTextBrowser *browser = qobject_cast<QTextBrowser *>(doc->parent());
      if (browser) {
         context = browser->source().toString();
      }
#endif
      QImage img;


      if (img.isNull()) {          // try direct loading
         name = format.name();    // remove qrc:/ prefix again
         if (name.isEmpty() || !img.load(name)) {
            return QPixmap(QLatin1String(":/copperspice/styles/commonstyle/images/file-16.png"));
         }
      }
      pm = QPixmap::fromImage(img);
      doc->addResource(QTextDocument::ImageResource, url, pm);
   }

   return pm;
}

static QSize getPixmapSize(QTextDocument *doc, const QTextImageFormat &format)
{
   QPixmap pm;

   const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
   const int width = qRound(format.width());
   const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
   const int height = qRound(format.height());

   QSize size(width, height);
   if (!hasWidth || !hasHeight) {
      pm = getPixmap(doc, format);
      if (!hasWidth) {
         if (!hasHeight) {
            size.setWidth(pm.width());
         } else {
            size.setWidth(qRound(height * (pm.width() / (qreal) pm.height())));
         }
      }
      if (!hasHeight) {
         if (!hasWidth) {
            size.setHeight(pm.height());
         } else {
            size.setHeight(qRound(width * (pm.height() / (qreal) pm.width())));
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

static QImage getImage(QTextDocument *doc, const QTextImageFormat &format)
{
   QImage image;

   QString name = format.name();
   if (name.startsWith(QLatin1String(":/"))) { // auto-detect resources
      name.prepend(QLatin1String("qrc"));
   }

   QUrl url = QUrl::fromEncoded(name.toUtf8());
   const QVariant data = doc->resource(QTextDocument::ImageResource, url);

   if (data.type() == QVariant::Image) {
      image = qvariant_cast<QImage>(data);

   } else if (data.type() == QVariant::ByteArray) {
      image.loadFromData(data.toByteArray());

   }

   if (image.isNull()) {
      QString context;

#ifndef QT_NO_TEXTBROWSER
      QTextBrowser *browser = qobject_cast<QTextBrowser *>(doc->parent());
      if (browser) {
         context = browser->source().toString();
      }
#endif


      if (image.isNull()) {        // try direct loading
         name = format.name();    // remove qrc:/ prefix again
         if (name.isEmpty() || !image.load(name)) {
            return QImage(QLatin1String(":/copperspice/styles/commonstyle/images/file-16.png"));
         }
      }
      doc->addResource(QTextDocument::ImageResource, url, image);
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
         size.setWidth(image.width());
      }
      if (!hasHeight) {
         size.setHeight(image.height());
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
   Q_UNUSED(posInDocument)
   const QTextImageFormat imageFormat = format.toImageFormat();

   if (qApp->thread() != QThread::currentThread()) {
      return getImageSize(doc, imageFormat);
   }
   return getPixmapSize(doc, imageFormat);
}

void QTextImageHandler::drawObject(QPainter *p, const QRectF &rect, QTextDocument *doc, int posInDocument,
                                   const QTextFormat &format)
{
   Q_UNUSED(posInDocument)
   const QTextImageFormat imageFormat = format.toImageFormat();

   if (qApp->thread() != QThread::currentThread()) {
      const QImage image = getImage(doc, imageFormat);
      p->drawImage(rect, image, image.rect());
   } else {
      const QPixmap pixmap = getPixmap(doc, imageFormat);
      p->drawPixmap(rect, pixmap, pixmap.rect());
   }
}

QT_END_NAMESPACE
