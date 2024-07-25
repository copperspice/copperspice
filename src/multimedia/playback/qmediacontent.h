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

#ifndef QMEDIACONTENT_H
#define QMEDIACONTENT_H

#include <qmediaresource.h>
#include <qshareddata.h>

class QMediaContentPrivate;
class QMediaPlaylist;

class Q_MULTIMEDIA_EXPORT QMediaContent
{
 public:
   QMediaContent();

   QMediaContent(const QUrl &contentUrl);
   QMediaContent(const QNetworkRequest &contentRequest);
   QMediaContent(const QMediaResource &contentResource);
   QMediaContent(const QList<QMediaResource> &resources);
   QMediaContent(const QMediaContent &other);
   QMediaContent(QMediaPlaylist *playlist, const QUrl &contentUrl = QUrl(), bool takeOwnership = false);

   ~QMediaContent();

   QMediaContent &operator=(const QMediaContent &other);

   bool operator==(const QMediaContent &other) const;
   bool operator!=(const QMediaContent &other) const;

   bool isNull() const;

   QUrl canonicalUrl() const;
   QNetworkRequest canonicalRequest() const;
   QMediaResource canonicalResource() const;

   QList<QMediaResource> resources() const;

   QMediaPlaylist *playlist() const;

 private:
   QSharedDataPointer<QMediaContentPrivate> d;
};

CS_DECLARE_METATYPE(QMediaContent)

#endif
