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

#include "qcommonvalues_p.h"
#include "qpatternistlocale_p.h"
#include "qschemanumeric_p.h"
#include "qatomicstring_p.h"
#include "qtocodepointsiterator_p.h"

#include "qassemblestringfns_p.h"

using namespace QPatternist;

/*
 * Determines whether @p cp is a valid XML 1.0 character.
 *
 * @see <a href="http://www.w3.org/TR/REC-xml/#charsets">Extensible Markup
 * Language (XML) 1.0 (Third Edition)2.2 Characters</a>
 */
static inline bool isValidXML10Char(const qint32 cp)
{
   /* [2]     Char     ::=     #x9 | #xA | #xD | [#x20-#xD7FF] |
    *                          [#xE000-#xFFFD] | [#x10000-#x10FFFF]
    */
   return (cp == 0x9                       ||
           cp == 0xA                       ||
           cp == 0xD                       ||
           (0x20 <= cp && cp <= 0xD7FF)    ||
           (0xE000 <= cp && cp <= 0xFFFD)  ||
           (0x10000 <= cp && cp <= 0x10FFFF));
}

Item CodepointsToStringFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item::Iterator::Ptr it(m_operands.first()->evaluateSequence(context));

   if (!it) {
      return CommonValues::EmptyString;
   }

   QString retval;
   Item item(it->next());
   while (item) {
      const qint32 cp = static_cast<qint32>(item.as<Numeric>()->toInteger());

      if (!isValidXML10Char(cp)) {
         context->error(QtXmlPatterns::tr("%1 is not a valid XML 1.0 character.")
                        .formatArg(formatData(QLatin1String("0x") +
                                        QString::number(cp, 16))),
                        ReportContext::FOCH0001, this);

         return CommonValues::EmptyString;
      }
      retval.append(QChar(cp));
      item = it->next();
   }

   return AtomicString::fromValue(retval);
}

Item::Iterator::Ptr StringToCodepointsFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
   const Item item(m_operands.first()->evaluateSingleton(context));
   if (!item) {
      return CommonValues::emptyIterator;
   }

   const QString str(item.stringValue());
   if (str.isEmpty()) {
      return CommonValues::emptyIterator;
   } else {
      return Item::Iterator::Ptr(new ToCodepointsIterator(str));
   }
}
