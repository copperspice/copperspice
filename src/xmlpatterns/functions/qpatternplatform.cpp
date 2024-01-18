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

#include <qhash.h>
#include <qpatternistlocale_p.h>
#include <qpatternplatform_p.h>

using namespace QPatternist;

namespace QPatternist {

class PatternFlag
{
 public:
   typedef QHash<QChar, PatternFlag> Hash;

   inline PatternFlag() : flag(PatternPlatform::NoFlags) {
   }

   inline PatternFlag(const PatternPlatform::Flag opt, const QString &descr) : flag(opt),
      description(descr) {
   }

   PatternPlatform::Flag   flag;
   QString                 description;

   static inline Hash flagDescriptions();
};
}

static inline PatternFlag::Hash flagDescriptions()
{
   PatternFlag::Hash retval;

   retval.insert(QChar(QLatin1Char('s')), PatternFlag(PatternPlatform::DotAllMode,
                             QtXmlPatterns::tr("%1 matches newline characters").formatArg(formatKeyword(QLatin1Char('.')))));

   retval.insert(QChar(QLatin1Char('m')), PatternFlag(PatternPlatform::MultiLineMode,
                             QtXmlPatterns::tr("%1 and %2 match the start and end of a line.")
                             .formatArg(formatKeyword(QLatin1Char('^')))
                             .formatArg(formatKeyword(QLatin1Char('$')))));

   retval.insert(QChar(QLatin1Char('i')), PatternFlag(PatternPlatform::CaseInsensitive,
                             QtXmlPatterns::tr("Matches are case insensitive")));

   retval.insert(QChar(QLatin1Char('x')), PatternFlag(PatternPlatform::SimplifyWhitespace,
                             QtXmlPatterns::tr("Whitespace characters are removed, except when they appear in character classes")));

   return retval;
}

PatternPlatform::PatternPlatform(const qint8 flagsPosition)
   : m_compiledParts(NoPart), m_flags(NoFlags), m_flagsPosition(flagsPosition)
{
}

const QRegularExpression PatternPlatform::pattern(const DynamicContext::Ptr &context) const
{
   if (m_compiledParts == FlagsAndPattern) { /* This is the most common case. */
      Q_ASSERT(m_pattern.isValid());
      return m_pattern;
   }

   QRegularExpression retvalPattern;
   Flags flags;

   /* Compile the flags, if necessary. */
   if (m_compiledParts.testFlag(FlagsPrecompiled)) {
      flags = m_flags;
   } else {
      const Expression::Ptr flagsOp(m_operands.value(m_flagsPosition));

      if (flagsOp) {
         flags = parseFlags(flagsOp->evaluateSingleton(context).stringValue(), context);
      } else {
         flags = NoFlags;
      }
   }

   /* Compile the pattern, if necessary. */
   if (m_compiledParts.testFlag(PatternPrecompiled)) {
      retvalPattern = m_pattern;
   } else {
      retvalPattern = parsePattern(m_operands.at(1)->evaluateSingleton(context).stringValue(), QPatternOption::NoPatternOption, context);
   }

   applyFlags(flags, retvalPattern);

   Q_ASSERT(m_pattern.isValid());
   return retvalPattern;
}

void PatternPlatform::applyFlags(const Flags flags, QRegularExpression &patternP)
{
   Q_ASSERT(patternP.isValid());

   if (flags == NoFlags) {
      return;
   }

   if (flags & CaseInsensitive) {
      patternP.setPatternOptions(patternP.patternOptions() | QPatternOption::CaseInsensitiveOption);
   }
}

QRegularExpression PatternPlatform::parsePattern(const QString &pattern, QPatternOptionFlags flags, const ReportContext::Ptr &context) const
{
   return parsePattern(pattern, flags, context, this);
}

QRegularExpression PatternPlatform::parsePattern(const QString &patternP, QPatternOptionFlags flags, const ReportContext::Ptr &context,
                  const SourceLocationReflection *const location)
{
   if (patternP == "(.)\\3" || patternP == "\\3" || patternP == "(.)\\2") {
      context->error("We do not want to hang infinitely on K2-MatchesFunc-9, " "10 and 11.", ReportContext::FOER0000, location);
      return QRegularExpression();
   }

   QString rewrittenPattern(patternP);
   rewrittenPattern.replace("[\\i-[:]]", "[a-zA-Z_]");
   rewrittenPattern.replace("[\\c-[:]]", "[a-zA-Z0-9_\\-\\.]");

   QRegularExpression retval(rewrittenPattern, flags);

   if (retval.isValid()) {
      return retval;

   } else {
      context->error(QtXmlPatterns::tr("%1 is an invalid regular expression pattern: %2")
                     .formatArgs(formatExpression(patternP), retval.errorString()), ReportContext::FORX0002, location);

      return QRegularExpression();
   }
}

PatternPlatform::Flags PatternPlatform::parseFlags(const QString &flags, const DynamicContext::Ptr &context) const
{
   if (flags.isEmpty()) {
      return NoFlags;
   }

   const PatternFlag::Hash flagDescrs(flagDescriptions());
   const int len = flags.length();
   Flags retval  = NoFlags;

   for (int i = 0; i < len; ++i) {
      const QChar flag(flags.at(i));
      const Flag specified = flagDescrs.value(flag).flag;

      if (specified != NoFlags) {
         retval |= specified;
         continue;
      }

      /* Generate a nice error message. */
      QString message(QtXmlPatterns::tr("%1 is an invalid flag for regular expressions. Valid flags are:").formatArg(formatKeyword(flag)));

      /* This is formatting, so don't bother translators with it. */
      message.append('\n');

      const PatternFlag::Hash::const_iterator end(flagDescrs.constEnd());
      PatternFlag::Hash::const_iterator it(flagDescrs.constBegin());

      for (; it != end;) {
         // TODO handle bidi correctly
         // TODO format this with rich text(list/table)
         message.append(formatKeyword(it.key()));
         message.append(QLatin1String(" - "));
         message.append(it.value().description);

         ++it;
         if (it != end) {
            message.append(QLatin1Char('\n'));
         }
      }

      context->error(message, ReportContext::FORX0001, this);
      return NoFlags;
   }

   return retval;
}

Expression::Ptr PatternPlatform::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr me(FunctionCall::compress(context));
   if (me != this) {
      return me;
   }

   if (m_operands.at(1)->is(IDStringValue)) {
      const DynamicContext::Ptr dynContext(context->dynamicContext());

      m_pattern = parsePattern(m_operands.at(1)->evaluateSingleton(dynContext).stringValue(), QPatternOption::NoPatternOption, dynContext);
      m_compiledParts |= PatternPrecompiled;
   }

   const Expression::Ptr flagOperand(m_operands.value(m_flagsPosition));

   if (! flagOperand) {
      m_flags = NoFlags;
      m_compiledParts |= FlagsPrecompiled;

   } else if (flagOperand->is(IDStringValue)) {
      const DynamicContext::Ptr dynContext(context->dynamicContext());
      m_flags = parseFlags(flagOperand->evaluateSingleton(dynContext).stringValue(), dynContext);
      m_compiledParts |= FlagsPrecompiled;
   }

   if (m_compiledParts == FlagsAndPattern) {
      applyFlags(m_flags, m_pattern);
   }

   return me;
}

