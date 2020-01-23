/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qmetatype.h>
#include <qshareddata.h>
#include <qmediaresource.h>

class QMediaPlaylist;
class QMediaContentPrivate;

class Q_MULTIMEDIA_EXPORT QMediaContent
{
 public:
   QMediaContent();

   QMediaContent(const QUrl &contentUrl);
   QMediaContent(const QNetworkRequest &contentRequest);
   QMediaContent(const QMediaResource &contentResource);
   QMediaContent(const QMediaResourceList &resources);
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

   QMediaResourceList resources() const;

   QMediaPlaylist *playlist() const;

 private:
   QSharedDataPointer<QMediaContentPrivate> d;
};

template<>
class Q_MULTIMEDIA_EXPORT cs_typeName_internal<QMediaContent, void>
{
 public:
   static const QString &typeName();
};

Q_DECLARE_METATYPE(QMediaContent)

#endif
