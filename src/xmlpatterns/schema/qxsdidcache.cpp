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

#include "qxsdidcache_p.h"

#include <QtCore/QReadLocker>
#include <QtCore/QWriteLocker>

QT_BEGIN_NAMESPACE

using namespace QPatternist;

void XsdIdCache::addId(const QString &id)
{
   const QWriteLocker locker(&m_lock);
   Q_ASSERT(!m_ids.contains(id));

   m_ids.insert(id);
}

bool XsdIdCache::hasId(const QString &id) const
{
   const QReadLocker locker(&m_lock);

   return m_ids.contains(id);
}

QT_END_NAMESPACE
