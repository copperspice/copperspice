/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

//#define QNETWORKACCESSCACHEBACKEND_DEBUG

#include "qnetworkaccesscachebackend_p.h"
#include "qabstractnetworkcache.h"
#include "qfileinfo.h"
#include "qurlinfo.h"
#include "qdir.h"
#include "qcoreapplication.h"

QT_BEGIN_NAMESPACE

QNetworkAccessCacheBackend::QNetworkAccessCacheBackend()
   : QNetworkAccessBackend()
   , device(0)
{
}

QNetworkAccessCacheBackend::~QNetworkAccessCacheBackend()
{
}

void QNetworkAccessCacheBackend::open()
{
   if (operation() != QNetworkAccessManager::GetOperation || !sendCacheContents()) {
      QString msg = QCoreApplication::translate("QNetworkAccessCacheBackend", "Error opening %1")
                    .arg(this->url().toString());
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
   if (!nc) {
      return false;
   }

   QNetworkCacheMetaData item = nc->metaData(url());
   if (!item.isValid()) {
      return false;
   }

   QNetworkCacheMetaData::AttributesMap attributes = item.attributes();
   setAttribute(QNetworkRequest::HttpStatusCodeAttribute, attributes.value(QNetworkRequest::HttpStatusCodeAttribute));
   setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, attributes.value(QNetworkRequest::HttpReasonPhraseAttribute));

   // set the raw headers
   QNetworkCacheMetaData::RawHeaderList rawHeaders = item.rawHeaders();
   QNetworkCacheMetaData::RawHeaderList::ConstIterator it = rawHeaders.constBegin(),
                                                       end = rawHeaders.constEnd();
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

   // signal we're open
   metaDataChanged();

   if (operation() == QNetworkAccessManager::GetOperation) {
      QIODevice *contents = nc->data(url());
      if (!contents) {
         return false;
      }
      contents->setParent(this);
      writeDownstreamData(contents);
   }

#if defined(QNETWORKACCESSCACHEBACKEND_DEBUG)
   qDebug() << "Successfully sent cache:" << url();
#endif
   return true;
}

void QNetworkAccessCacheBackend::closeDownstreamChannel()
{
   if (operation() == QNetworkAccessManager::GetOperation) {
      device->close();
      delete device;
      device = 0;
   }
}

void QNetworkAccessCacheBackend::closeUpstreamChannel()
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function show not have been called!");
}

void QNetworkAccessCacheBackend::upstreamReadyRead()
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function show not have been called!");
}

void QNetworkAccessCacheBackend::downstreamReadyWrite()
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function show not have been called!");
}

QT_END_NAMESPACE

