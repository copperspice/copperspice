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

#include <qstringlist.h>

#include "qboolean_p.h"
#include "qcommonvalues_p.h"
#include "qitemmappingiterator_p.h"
#include "qpatternistlocale_p.h"
#include "qatomicstring_p.h"

#include "qpatternmatchingfns_p.h"

using namespace QPatternist;

MatchesFN::MatchesFN() : PatternPlatform(2)
{
}

Item MatchesFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const QRegularExpression regexp(pattern(context));
   QString input;

   const Item arg(m_operands.first()->evaluateSingleton(context));
   if (arg) {
      input = arg.stringValue();
   }

   return Boolean::fromValue(input.contains(regexp));
}

ReplaceFN::ReplaceFN() : PatternPlatform(3)
{
}

Item ReplaceFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const QRegularExpression regexp(pattern(context));
   QString input;

   const Item arg(m_operands.first()->evaluateSingleton(context));
   if (arg) {
      input = arg.stringValue();
   }

   const QString replacement(m_replacementString.isEmpty() ?
                  parseReplacement(regexp.captureCount(), context) : m_replacementString);

   return AtomicString::fromValue(input.replace(regexp, replacement));
}

QString ReplaceFN::errorAtEnd(const char ch)
{
   return QtXmlPatterns::tr("%1 must be followed by %2 or %3, not at the end of the replacement string.")
          .formatArg(formatKeyword(QLatin1Char(ch)))
          .formatArg(formatKeyword(QLatin1Char('\\')))
          .formatArg(formatKeyword(QLatin1Char('$')));
}

QString ReplaceFN::parseReplacement(const int, const DynamicContext::Ptr &context) const
{
   // TODO what if there is no groups, can one rewrite to the replacement then?
   const QString input(m_operands.at(2)->evaluateSingleton(context).stringValue());

   QString retval;
   const int len = input.length();

   for (int i = 0; i < len; ++i) {
      const QChar ch(input.at(i));

      switch (ch.toLatin1()) {
         case '$': {
            /* QRegularExpression uses '\' as opposed to '$' for marking sub groups. */
            retval.append(QLatin1Char('\\'));

            ++i;
            if (i == len) {
               context->error(errorAtEnd('$'), ReportContext::FORX0004, this);
               return QString();
            }

            const QChar nextCh(input.at(i));
            if (nextCh.isDigit()) {
               retval.append(nextCh);
            } else {
               context->error(QtXmlPatterns::tr("In the replacement string, %1 must be followed by at least one digit when not escaped.")
                              .formatArg(formatKeyword(QLatin1Char('$'))), ReportContext::FORX0004, this);
               return QString();
            }

            break;
         }
         case '\\': {
            ++i;
            if (i == len) {
               /* error, we've reached the end. */;
               context->error(errorAtEnd('\\'), ReportContext::FORX0004, this);
            }

            const QChar nextCh(input.at(i));
            if (nextCh == QLatin1Char('\\') || nextCh == QLatin1Char('$')) {
               retval.append(ch);
               break;
            } else {
               context->error(QtXmlPatterns::tr("In the replacement string, %1 can only be used to "
                                                "escape itself or %2, not %3")
                              .formatArg(formatKeyword(QLatin1Char('\\')))
                              .formatArg(formatKeyword(QLatin1Char('$')))
                              .formatArg(formatKeyword(nextCh)),
                              ReportContext::FORX0004, this);
               return QString();
            }
         }
         default:
            retval.append(ch);
      }
   }

   return retval;
}

Expression::Ptr ReplaceFN::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr me(PatternPlatform::compress(context));

   if (me != this) {
      return me;
   }

   if (m_operands.at(2)->is(IDStringValue)) {
      const int capt = captureCount();
      if (capt == -1) {
         return me;
      } else {
         m_replacementString = parseReplacement(captureCount(), context->dynamicContext());
      }
   }

   return me;
}

TokenizeFN::TokenizeFN() : PatternPlatform(2)
{
}

/**
 * Used by QAbstractXmlForwardIterator.
 */
static inline bool qIsForwardIteratorEnd(const QString &item)
{
   return item.isEmpty();
}

Item TokenizeFN::mapToItem(const QString &subject, const DynamicContext::Ptr &) const
{
   return AtomicString::fromValue(subject);
}

Item::Iterator::Ptr TokenizeFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
   const Item arg(m_operands.first()->evaluateSingleton(context));

   if (! arg) {
      return CommonValues::emptyIterator;
   }

   const QString input(arg.stringValue());

   if (input.isEmpty()) {
      return CommonValues::emptyIterator;
   }

   const QRegularExpression regExp(pattern(context));
   const QStringList result(input.split(regExp, QStringParser::KeepEmptyParts));

   return makeItemMappingIterator<Item>(ConstPtr(this), makeListIterator(result), DynamicContext::Ptr());
}

