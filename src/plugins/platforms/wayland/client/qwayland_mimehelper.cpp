/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qwayland_mimehelper.h>

#include <qbuffer.h>
#include <qcolor.h>
#include <qimage.h>
#include <qimagewriter.h>
#include <qurl.h>

QByteArray QWaylandMimeHelper::getByteArray(QMimeData *mimeData, const QString &mimeType)
{
   QByteArray content;

   if (mimeType == "text/plain") {
      content = mimeData->text().toUtf8();

   } else if (mimeData->hasImage() && (mimeType == "application/x-qt-image" || mimeType.startsWith("image/"))) {

      QImage image = mimeData->imageData().value<QImage>();

      if (! image.isNull()) {
         QBuffer buf;
         buf.open(QIODevice::ReadWrite);

         QByteArray fmt = "BMP";

         if (mimeType.startsWith("image/")) {
            QByteArray imgFmt = mimeType.mid(6).toUpper().toUtf8();

            if (QImageWriter::supportedImageFormats().contains(imgFmt)) {
               fmt = imgFmt;
            }
         }

         QImageWriter wr(&buf, fmt);

         wr.write(image);
         content = buf.buffer();
      }

   } else if (mimeType == "application/x-color") {
      content = mimeData->colorData().value<QColor>().name().toUtf8();

   } else if (mimeType == "text/uri-list") {
      const QList<QUrl> &urls = mimeData->urls();

      for (const auto &item : urls) {
         content.append(item.toEncoded());
         content.append('\n');
      }

   } else {
      content = mimeData->data(mimeType);
   }

   return content;
}
