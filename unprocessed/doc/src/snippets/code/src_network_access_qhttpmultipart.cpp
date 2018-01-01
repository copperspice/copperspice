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

//! [0]
QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

QHttpPart textPart;
textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"text\""));
textPart.setBody("my text");

QHttpPart imagePart;
imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\""));
QFile *file = new QFile("image.jpg");
file->open(QIODevice::ReadOnly);
imagePart.setBodyDevice(file);
file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

multiPart->append(textPart);
multiPart->append(imagePart);

QUrl url("http://my.server.tld");
QNetworkRequest request(url);

QNetworkAccessManager manager;
QNetworkReply *reply = manager.post(request, multiPart);
multiPart->setParent(reply); // delete the multiPart with the reply
// here connect signals etc.
//! [0]
