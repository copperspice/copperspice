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

#include <qplatformdefs.h>
#include <qurl.h>
#include <qdataurl_p.h>

QT_BEGIN_NAMESPACE

/*!
    \internal

    Decode a data: URL into its mimetype and payload. Returns a null string if
    the URL could not be decoded.
*/
Q_CORE_EXPORT QPair<QString, QByteArray> qDecodeDataUrl(const QUrl &uri)
{
   QString mimeType;
   QByteArray payload;

   if (uri.scheme() == QLatin1String("data") && uri.host().isEmpty()) {
      mimeType = QLatin1String("text/plain;charset=US-ASCII");

      // the following would have been the correct thing, but
      // reality often differs from the specification. People have
      // data: URIs with ? and #
      //QByteArray data = QByteArray::fromPercentEncoding(uri.encodedPath());
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
            int i = 7;      // strlen("charset")
            while (data.at(i) == ' ') {
               ++i;
            }
            if (data.at(i) == '=') {
               data.prepend("text/plain;");
            }
         }

         if (!data.isEmpty()) {
            mimeType = QLatin1String(data.trimmed());
         }

      }
   }

   return QPair<QString, QByteArray>(mimeType, payload);
}

QT_END_NAMESPACE
