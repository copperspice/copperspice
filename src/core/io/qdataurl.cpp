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

#include <qdataurl_p.h>

#include <qplatformdefs.h>
#include <qurl.h>

Q_CORE_EXPORT QPair<QString, QByteArray> qDecodeDataUrl(const QUrl &uri)
{
   QString mimeType;
   QByteArray payload;

   if (uri.scheme() == "data" && uri.host().isEmpty()) {
      mimeType = "text/plain;charset=US-ASCII";

      QByteArray data = QByteArray::fromPercentEncoding(uri.toEncoded());

      // remove the data: scheme
      data.remove(0, 5);

      // parse it:
      int pos = data.indexOf(',');

      if (pos != -1) {
         payload = data.mid(pos + 1);

         data.truncate(pos);
         data = data.trimmed();

         // find out if the payload is encoded in Base64
         if (data.endsWith(";base64")) {
            payload = QByteArray::fromBase64(payload);
            data.chop(7);
         }

         if (data.toLower().startsWith("charset")) {
            int i = 7;

            while (data.at(i) == ' ') {
               ++i;
            }

            if (data.at(i) == '=') {
               data.prepend("text/plain;");
            }
         }

         if (! data.isEmpty()) {
            mimeType = QString::fromUtf8(data.trimmed());
         }

      }
   }

   return QPair<QString, QByteArray>(mimeType, payload);
}
