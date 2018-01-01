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

#include "qstaticbaseuricontext_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

StaticBaseURIContext::StaticBaseURIContext(const QUrl &bURI,
      const StaticContext::Ptr &prevContext) : DelegatingStaticContext(prevContext)
   , m_baseURI(bURI)
{
   Q_ASSERT(m_baseURI.isValid());
   Q_ASSERT(!m_baseURI.isRelative());
   Q_ASSERT(prevContext);
}

QUrl StaticBaseURIContext::baseURI() const
{
   return m_baseURI;
}

QT_END_NAMESPACE
