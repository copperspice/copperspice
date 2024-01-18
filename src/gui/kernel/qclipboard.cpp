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

#include <qclipboard.h>

#ifndef QT_NO_CLIPBOARD

#include <qpixmap.h>
#include "qmimedata.h"
#include <qvariant.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qtextcodec.h>

#include <qguiapplication_p.h>
#include <qplatform_integration.h>
#include <qplatform_clipboard.h>

QClipboard::QClipboard(QObject *parent)
   : QObject(parent)
{
}

QClipboard::~QClipboard()
{
}

QString QClipboard::text(QString &subtype, Mode mode) const
{
   const QMimeData *const data = mimeData(mode);
   if (!data) {
      return QString();
   }

   const QStringList formats = data->formats();

   if (subtype.isEmpty()) {
      if (formats.contains(QLatin1String("text/plain"))) {
         subtype = QLatin1String("plain");
      } else {
         for (int i = 0; i < formats.size(); ++i)
            if (formats.at(i).startsWith(QLatin1String("text/"))) {
               subtype = formats.at(i).mid(5);
               break;
            }
         if (subtype.isEmpty()) {
            return QString();
         }
      }
   } else if (!formats.contains(QLatin1String("text/") + subtype)) {
      return QString();
   }

   const QByteArray rawData = data->data(QLatin1String("text/") + subtype);

#ifndef QT_NO_TEXTCODEC
   QTextCodec *codec = QTextCodec::codecForMib(106); // utf-8 is default
   if (subtype == QLatin1String("html")) {
      codec = QTextCodec::codecForHtml(rawData, codec);
   } else {
      codec = QTextCodec::codecForUtfText(rawData, codec);
   }
   return codec->toUnicode(rawData);

#else
   return rawData;
#endif

}

QString QClipboard::text(Mode mode) const
{
   const QMimeData *data = mimeData(mode);
   return data ? data->text() : QString();
}

void QClipboard::setText(const QString &text, Mode mode)
{
   QMimeData *data = new QMimeData;
   data->setText(text);
   setMimeData(data, mode);
}

QImage QClipboard::image(Mode mode) const
{
   const QMimeData *data = mimeData(mode);
   if (!data) {
      return QImage();
   }

   return data->imageData().value<QImage>();
}

void QClipboard::setImage(const QImage &image, Mode mode)
{
   QMimeData *data = new QMimeData;
   data->setImageData(image);
   setMimeData(data, mode);
}

QPixmap QClipboard::pixmap(Mode mode) const
{
   const QMimeData *data = mimeData(mode);
   return data ? data->imageData().value<QPixmap>() : QPixmap();
}

void QClipboard::setPixmap(const QPixmap &pixmap, Mode mode)
{
   QMimeData *data = new QMimeData;
   data->setImageData(pixmap);
   setMimeData(data, mode);
}

const QMimeData *QClipboard::mimeData(Mode mode) const
{
   QPlatformClipboard *clipboard = QGuiApplicationPrivate::platformIntegration()->clipboard();
   if (!clipboard->supportsMode(mode)) {
      return nullptr;
   }

   return clipboard->mimeData(mode);
}

void QClipboard::setMimeData(QMimeData *src, Mode mode)
{
   QPlatformClipboard *clipboard = QGuiApplicationPrivate::platformIntegration()->clipboard();
   if (!clipboard->supportsMode(mode)) {
      if (src != nullptr) {
         qWarning("QClipboard::setMimeData() Unsupported clipboard mode, QMimeData will be deleted.");
         src->deleteLater();
      }
   } else {
      clipboard->setMimeData(src, mode);
   }
}
void QClipboard::clear(Mode mode)
{
   setMimeData(nullptr, mode);
}

bool QClipboard::supportsSelection() const
{
   return supportsMode(Selection);
}

bool QClipboard::supportsFindBuffer() const
{
   return supportsMode(FindBuffer);
}

bool QClipboard::ownsClipboard() const
{
   return ownsMode(Clipboard);
}

bool QClipboard::ownsSelection() const
{
   return ownsMode(Selection);
}

bool QClipboard::ownsFindBuffer() const
{
   return ownsMode(FindBuffer);
}

bool QClipboard::supportsMode(Mode mode) const
{
   QPlatformClipboard *clipboard = QGuiApplicationPrivate::platformIntegration()->clipboard();
   return clipboard->supportsMode(mode);
}
bool QClipboard::ownsMode(Mode mode) const
{
   QPlatformClipboard *clipboard = QGuiApplicationPrivate::platformIntegration()->clipboard();
   return clipboard->ownsMode(mode);
}
void QClipboard::emitChanged(Mode mode)
{
   switch (mode) {
      case Clipboard:
         emit dataChanged();
         break;
      case Selection:
         emit selectionChanged();
         break;
      case FindBuffer:
         emit findBufferChanged();
         break;
      default:
         break;
   }
   emit changed(mode);
}


#endif // QT_NO_CLIPBOARD

