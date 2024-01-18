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

#ifndef QPatternPlatform_P_H
#define QPatternPlatform_P_H

#include <QFlags>
#include <qregularexpression.h>
#include <qfunctioncall_p.h>

namespace QPatternist {

class PatternPlatform : public FunctionCall
{
 public:
   /**
    * @see <a href="http://www.w3.org/TR/xpath-functions/#flags">XQuery 1.0 and
    * XPath 2.0 Functions and Operators, 7.6.1.1 Flags</a>
    */
   enum Flag {
      /**
       * No flags are set. Default behavior is used.
       */
      NoFlags             = 0,

      /**
       * Flag @c s
       */
      DotAllMode          = 1,

      /**
       * Flag @c m
       */
      MultiLineMode       = 2,

      /**
       * Flag @c i
       */
      CaseInsensitive     = 4,

      /**
       * Flag @c x
       */
      SimplifyWhitespace  = 8
   };
   typedef QFlags<Flag> Flags;

   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   /**
    * Retrieves the pattern supplied in the arguments, taking care of compiling it,
    * settings its flags, and everything else required for getting it ready to use. If an error
    * occurs, an appropriate error is raised via @p context.
    */
   const QRegularExpression pattern(const DynamicContext::Ptr &context) const;

   /**
    * @returns the number of captures, also called parenthesized sub-expressions, the pattern has.
    *
    * If the pattern isn't precompiled, -1 is returned.
    */
   inline int captureCount() const;

   /**
    * @short Parses pattern
    */
   static QRegularExpression parsePattern(const QString &pattern, QPatternOptionFlags flags, const ReportContext::Ptr &context,
                  const SourceLocationReflection *const location);


 protected:
   /**
    * @short This constructor is protected, because this class is supposed to be sub-classed.
    *
    * @param flagsPosition an index position specifying the operand containing the pattern
    * flags.
    */
   PatternPlatform(const qint8 flagsPosition);

 private:
   /**
    * Enum telling whether the flags, pattern, or both
    * have been compiled at compile time.
    */
   enum PreCompiledPart {
      NoPart          = 0,
      PatternPrecompiled     = 1,
      FlagsPrecompiled       = 2,
      FlagsAndPattern = PatternPrecompiled | FlagsPrecompiled

   };
   typedef QFlags<PreCompiledPart> PreCompiledParts;

   inline QRegularExpression parsePattern(const QString &pattern, QPatternOptionFlags flags, const ReportContext::Ptr &context) const;

   PatternPlatform(const PatternPlatform &) = delete;
   PatternPlatform &operator=(const PatternPlatform &) = delete;

   Flags parseFlags(const QString &flags, const DynamicContext::Ptr &context) const;

   static void applyFlags(const Flags flags, QRegularExpression &pattern);

   /**
    * The parts that have been pre-compiled at compile time.
    */
   PreCompiledParts    m_compiledParts;
   Flags               m_flags;
   QRegularExpression  m_pattern;
   const qint8         m_flagsPosition;
};

inline int PatternPlatform::captureCount() const
{
   if (m_compiledParts.testFlag(PatternPrecompiled)) {
      return m_pattern.captureCount();
   } else {
      return -1;
   }
}

Q_DECLARE_OPERATORS_FOR_FLAGS(PatternPlatform::Flags)
}

#endif
