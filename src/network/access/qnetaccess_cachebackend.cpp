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

#include <qnetaccess_cachebackend_p.h>

#include <qabstract_networkcache.h>
#include <qassert.h>
#include <qcoreapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qurlinfo_p.h>

QNetworkAccessCacheBackend::QNetworkAccessCacheBackend()
   : QNetworkAccessBackend()
{
}

QNetworkAccessCacheBackend::~QNetworkAccessCacheBackend()
{
}

void QNetworkAccessCacheBackend::open()
{
   if (operation() != QNetworkAccessManager::GetOperation || !sendCacheContents()) {
      QString msg = QCoreApplication::translate("QNetworkAccessCacheBackend", "Error opening %1").formatArg(this->url().toString());
      error(QNetworkReply::ContentNotFoundError, msg);

   } else {
      setAttribute(QNetworkRequest::SourceIsFromCacheAttribute, true);
   }

   finished();
}

bool QNetworkAccessCacheBackend::sendCacheContents()
{
   setCachingEnabled(false);
   QAbstractNetworkCache *nc = networkCache();
   if (! nc) {
      return false;
   }

   QNetworkCacheMetaData item = nc->metaData(url());
   if (!item.isValid()) {
      return false;
   }

   QNetworkCacheMetaData::AttributesMap attributes = item.attributes();
   setAttribute(QNetworkRequest::HttpStatusCodeAttribute,   attributes.value(QNetworkRequest::HttpStatusCodeAttribute));
   setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, attributes.value(QNetworkRequest::HttpReasonPhraseAttribute));

   // set the raw headers
   QNetworkCacheMetaData::RawHeaderList rawHeaders = item.rawHeaders();
   QNetworkCacheMetaData::RawHeaderList::const_iterator it = rawHeaders.constBegin(), end = rawHeaders.constEnd();

   for ( ; it != end; ++it) {
      if (it->first.toLower() == "cache-control" &&
            it->second.toLower().contains("must-revalidate")) {
         return false;
      }
      setRawHeader(it->first, it->second);
   }

   // handle a possible redirect
   QVariant redirectionTarget = attributes.value(QNetworkRequest::RedirectionTargetAttribute);
   if (redirectionTarget.isValid()) {
      setAttribute(QNetworkRequest::RedirectionTargetAttribute, redirectionTarget);
      redirectionRequested(redirectionTarget.toUrl());
   }

   // signal we are open
   metaDataChanged();

   if (operation() == QNetworkAccessManager::GetOperation) {
      QIODevice *contents = nc->data(url());

      if (! contents) {
         return false;
      }

      contents->setParent(this);
      writeDownstreamData(contents);
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "Cache successfully sent:" << url();
#endif

   return true;
}

void QNetworkAccessCacheBackend::closeDownstreamChannel()
{
}

void QNetworkAccessCacheBackend::closeUpstreamChannel()
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This method should not be called");
}

void QNetworkAccessCacheBackend::upstreamReadyRead()
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This method should not be called");
}

void QNetworkAccessCacheBackend::downstreamReadyWrite()
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This method should not be called");
}
