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
   enum Flag {
      NoFlags             = 0,
      DotAllMode          = 1,
      MultiLineMode       = 2,
      CaseInsensitive     = 4,
      SimplifyWhitespace  = 8
   };
   typedef QFlags<Flag> Flags;

   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   const QRegularExpression pattern(const DynamicContext::Ptr &context) const;

   inline int captureCount() const;

   static QRegularExpression parsePattern(const QString &pattern, QPatternOptionFlags flags, const ReportContext::Ptr &context,
                  const SourceLocationReflection *const location);


 protected:
   PatternPlatform(const qint8 flagsPosition);

 private:
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
