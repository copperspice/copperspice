/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include "qiodevicedelegate_p.h"
#include "quriloader_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

URILoader::URILoader(QObject *const parent,
                     const NamePool::Ptr &np,
                     const VariableLoader::Ptr &l) : QNetworkAccessManager(parent)
   , m_variableNS(QLatin1String("tag:copperspice.com,2007:QtXmlPatterns:QIODeviceVariable:"))
   , m_namePool(np)
   , m_variableLoader(l)
{
   Q_ASSERT(m_variableLoader);
}

QNetworkReply *URILoader::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
   const QString requestedUrl(req.url().toString());

   /* We got a QIODevice variable. */
   const QString name(requestedUrl.right(requestedUrl.length() - m_variableNS.length()));

   const QVariant variant(m_variableLoader->valueFor(m_namePool->allocateQName(QString(), name, QString())));

   if (!variant.isNull() && variant.userType() == qMetaTypeId<QIODevice *>()) {
      return new QIODeviceDelegate(qvariant_cast<QIODevice *>(variant));
   } else {
      /* If we're entering this code path, the variable URI identified a variable
       * which we don't have, which means we either have a bug, or the user had
       * crafted an invalid URI manually. */

      return QNetworkAccessManager::createRequest(op, req, outgoingData);
   }
}

QT_END_NAMESPACE

