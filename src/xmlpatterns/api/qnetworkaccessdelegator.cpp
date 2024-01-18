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

#include <qurl.h>
#include <qnetaccess_manager.h>

#include <qnetworkaccessdelegator_p.h>

using namespace QPatternist;

NetworkAccessDelegator::NetworkAccessDelegator(QNetworkAccessManager *const genericManager, QNetworkAccessManager *const variableURIManager)
   : m_genericManager(genericManager), m_variableURIManager(variableURIManager)
{
}

QNetworkAccessManager *NetworkAccessDelegator::managerFor(const QUrl &uri)
{
   /* Unfortunately we have to do it this way, QUrl::isParentOf() doesn't
    * understand URI schemes like this one. */

   const QString requestedUrl(uri.toString());

   /* On the topic of timeouts:
    *
    * Currently the schemes QNetworkAccessManager handles should/will do
    * timeouts for 4.4, but we need to do timeouts for our own. */

   if (requestedUrl.startsWith("tag:copperspice.com,2007:QtXmlPatterns:QIODeviceVariable:")) {
      return m_variableURIManager;

   } else {
      if (! m_genericManager) {
         m_genericManager = new QNetworkAccessManager(this);
      }

      return m_genericManager;
   }
}


