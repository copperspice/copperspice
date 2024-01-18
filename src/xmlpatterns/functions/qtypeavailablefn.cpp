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

#include "qboolean_p.h"
#include "qqnameconstructor_p.h"

#include "qtypeavailablefn_p.h"

using namespace QPatternist;

Item TypeAvailableFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const QString lexQName(m_operands.first()->evaluateSingleton(context).stringValue());

   const QXmlName name
   (QNameConstructor::expandQName<DynamicContext::Ptr,
    ReportContext::XTDE1428,
    ReportContext::XTDE1428>(lexQName,
                             context,
                             staticNamespaces(),
                             this));


   return Boolean::fromValue(m_schemaTypeFactory->types().contains(name));
}

Expression::Ptr TypeAvailableFN::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   m_schemaTypeFactory = context->schemaDefinitions();
   return StaticNamespacesContainer::typeCheck(context, reqType);
}
