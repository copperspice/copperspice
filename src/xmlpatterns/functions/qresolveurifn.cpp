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

#include "qanyuri_p.h"
#include "qliteral_p.h"
#include "qpatternistlocale_p.h"
#include "qatomicstring_p.h"
#include "qresolveurifn_p.h"

using namespace QPatternist;

Item ResolveURIFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item relItem(m_operands.first()->evaluateSingleton(context));

   if (relItem) {
      const QString base(m_operands.last()->evaluateSingleton(context).stringValue());
      const QString relative(relItem.stringValue());

      const QUrl baseURI(AnyURI::toQUrl<ReportContext::FORG0002, DynamicContext::Ptr>(base, context, this));
      const QUrl relativeURI(AnyURI::toQUrl<ReportContext::FORG0002, DynamicContext::Ptr>(relative, context, this));

      return toItem(AnyURI::fromValue(baseURI.resolved(relativeURI)));
   } else {
      return Item();
   }
}

Expression::Ptr ResolveURIFN::typeCheck(const StaticContext::Ptr &context,
                                        const SequenceType::Ptr &reqType)
{
   Q_ASSERT(m_operands.count() == 1 || m_operands.count() == 2);

   if (m_operands.count() == 1) {
      /* Our base URI is always well-defined. */
      m_operands.append(wrapLiteral(toItem(AnyURI::fromValue(context->baseURI())), context, this));
   }

   return FunctionCall::typeCheck(context, reqType);
}
