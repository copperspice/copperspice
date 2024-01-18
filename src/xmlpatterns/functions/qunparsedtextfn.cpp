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

#include "qanyuri_p.h"

#include "qunparsedtextfn_p.h"

using namespace QPatternist;

Item UnparsedTextFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(m_operands.count() == 1 || m_operands.count() == 2);
   const Item href(m_operands.first()->evaluateSingleton(context));
   if (!href) {
      return Item();
   }

   const QUrl mayRela(AnyURI::toQUrl<ReportContext::XTDE1170>(href.stringValue(),
                      context,
                      this));

   const QUrl uri(context->resolveURI(mayRela, staticBaseURI()));

   if (uri.hasFragment()) {
      context->error(QtXmlPatterns::tr("The URI cannot have a fragment"),
                     ReportContext::XTDE1170, this);
   }

   QString encoding;

   if (m_operands.count() == 2) {
      const Item encodingArg(m_operands.at(1)->evaluateSingleton(context));
      if (encodingArg) {
         encoding = encodingArg.stringValue();
      }
   }

   Q_ASSERT(uri.isValid() && !uri.isRelative());
   return context->resourceLoader()->openUnparsedText(uri, encoding, context, this);
}
