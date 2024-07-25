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

#include <qmedianetworkplaylistprovider_p.h>

#include <qmediacontent.h>

#include <qmediaobject_p.h>
#include <qmediaplaylistprovider_p.h>

class QMediaNetworkPlaylistProviderPrivate: public QMediaPlaylistProviderPrivate
{
   Q_DECLARE_NON_CONST_PUBLIC(QMediaNetworkPlaylistProvider)

 public:
   bool load(const QNetworkRequest &request);

   QPlaylistFileParser parser;
   QList<QMediaContent> resources;

   void _q_handleParserError(QPlaylistFileParser::ParserError error, const QString &errorMsg);
   void _q_handleNewItem(const QVariant &content);

   QMediaNetworkPlaylistProvider *q_ptr;
};

bool QMediaNetworkPlaylistProviderPrivate::load(const QNetworkRequest &request)
{
   parser.stop();
   parser.start(request, false);

   return true;
}

void QMediaNetworkPlaylistProviderPrivate::_q_handleParserError(QPlaylistFileParser::ParserError error,
      const QString &errorMsg)
{
   Q_Q(QMediaNetworkPlaylistProvider);

   QMediaPlaylist::Error playlistError = QMediaPlaylist::NoError;

   switch (error) {
      case QPlaylistFileParser::NoError:
         return;

      case QPlaylistFileParser::FormatError:
         playlistError = QMediaPlaylist::FormatError;
         break;

      case QPlaylistFileParser::FormatNotSupportedError:
         playlistError = QMediaPlaylist::FormatNotSupportedError;
         break;

      case QPlaylistFileParser::NetworkError:
         playlistError = QMediaPlaylist::NetworkError;
         break;
   }

   parser.stop();

   emit q->loadFailed(playlistError, errorMsg);
}

void QMediaNetworkPlaylistProviderPrivate::_q_handleNewItem(const QVariant &content)
{
   Q_Q(QMediaNetworkPlaylistProvider);

   QUrl url;
   if (content.type() == QVariant::Url) {
      url = content.toUrl();

   } else if (content.type() == QVariant::Map) {
      url = content.toMap()["url"].toUrl();

   } else {
      return;
   }

   q->addMedia(QMediaContent(url));
}

QMediaNetworkPlaylistProvider::QMediaNetworkPlaylistProvider(QObject *parent)
   : QMediaPlaylistProvider(*new QMediaNetworkPlaylistProviderPrivate, parent)
{
   d_func()->q_ptr = this;

   connect(&d_func()->parser, &QPlaylistFileParser::newItem,  this, &QMediaNetworkPlaylistProvider::_q_handleNewItem);
   connect(&d_func()->parser, &QPlaylistFileParser::finished, this, &QMediaNetworkPlaylistProvider::loaded);
   connect(&d_func()->parser, &QPlaylistFileParser::error,    this, &QMediaNetworkPlaylistProvider::_q_handleParserError);
}

QMediaNetworkPlaylistProvider::~QMediaNetworkPlaylistProvider()
{
}

bool QMediaNetworkPlaylistProvider::isReadOnly() const
{
   return false;
}

bool QMediaNetworkPlaylistProvider::load(const QNetworkRequest &request, const char *format)
{
   (void) format;
   return d_func()->load(request);
}

int QMediaNetworkPlaylistProvider::mediaCount() const
{
   return d_func()->resources.size();
}

QMediaContent QMediaNetworkPlaylistProvider::media(int pos) const
{
   return d_func()->resources.value(pos);
}

bool QMediaNetworkPlaylistProvider::addMedia(const QMediaContent &content)
{
   Q_D(QMediaNetworkPlaylistProvider);

   int pos = d->resources.count();

   emit mediaAboutToBeInserted(pos, pos);
   d->resources.append(content);
   emit mediaInserted(pos, pos);

   return true;
}

bool QMediaNetworkPlaylistProvider::addMedia(const QList<QMediaContent> &items)
{
   Q_D(QMediaNetworkPlaylistProvider);

   if (items.isEmpty()) {
      return true;
   }

   int pos = d->resources.count();
   int end = pos + items.count() - 1;

   emit mediaAboutToBeInserted(pos, end);
   d->resources.append(items);
   emit mediaInserted(pos, end);

   return true;
}

bool QMediaNetworkPlaylistProvider::insertMedia(int pos, const QMediaContent &content)
{
   Q_D(QMediaNetworkPlaylistProvider);

   emit mediaAboutToBeInserted(pos, pos);
   d->resources.insert(pos, content);
   emit mediaInserted(pos, pos);

   return true;
}

bool QMediaNetworkPlaylistProvider::insertMedia(int pos, const QList<QMediaContent> &items)
{
   Q_D(QMediaNetworkPlaylistProvider);

   if (items.isEmpty()) {
      return true;
   }

   const int last = pos + items.count() - 1;

   emit mediaAboutToBeInserted(pos, last);
   for (int i = 0; i < items.count(); i++) {
      d->resources.insert(pos + i, items.at(i));
   }
   emit mediaInserted(pos, last);

   return true;
}

bool QMediaNetworkPlaylistProvider::removeMedia(int fromPos, int toPos)
{
   Q_D(QMediaNetworkPlaylistProvider);

   Q_ASSERT(fromPos >= 0);
   Q_ASSERT(fromPos <= toPos);
   Q_ASSERT(toPos < mediaCount());

   emit mediaAboutToBeRemoved(fromPos, toPos);
   d->resources.erase(d->resources.begin() + fromPos, d->resources.begin() + toPos + 1);
   emit mediaRemoved(fromPos, toPos);

   return true;
}

bool QMediaNetworkPlaylistProvider::removeMedia(int pos)
{
   Q_D(QMediaNetworkPlaylistProvider);

   emit mediaAboutToBeRemoved(pos, pos);
   d->resources.removeAt(pos);
   emit mediaRemoved(pos, pos);

   return true;
}

bool QMediaNetworkPlaylistProvider::clear()
{
   Q_D(QMediaNetworkPlaylistProvider);

   if (!d->resources.isEmpty()) {
      int lastPos = mediaCount() - 1;
      emit mediaAboutToBeRemoved(0, lastPos);
      d->resources.clear();
      emit mediaRemoved(0, lastPos);
   }

   return true;
}

void QMediaNetworkPlaylistProvider::shuffle()
{
   Q_D(QMediaNetworkPlaylistProvider);

   if (!d->resources.isEmpty()) {
      QList<QMediaContent> resources;

      while (!d->resources.isEmpty()) {
         resources.append(d->resources.takeAt(qrand() % d->resources.size()));
      }

      d->resources = resources;
      emit mediaChanged(0, mediaCount() - 1);
   }
}

void QMediaNetworkPlaylistProvider::_q_handleParserError(QPlaylistFileParser::ParserError error, const QString &errorMsg)
{
   Q_D(QMediaNetworkPlaylistProvider);
   d->_q_handleParserError(error, errorMsg);
}

void QMediaNetworkPlaylistProvider::_q_handleNewItem(const QVariant &content)
{
   Q_D(QMediaNetworkPlaylistProvider);
   d->_q_handleNewItem(content);
}