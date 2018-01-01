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

#ifndef QPatternMatchingFNs_P_H
#define QPatternMatchingFNs_P_H

#include <qpatternplatform_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class MatchesFN : public PatternPlatform
{
 public:
   MatchesFN();
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class ReplaceFN : public PatternPlatform
{
 public:
   ReplaceFN();
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   /**
    * Overridden to attempt to pre-compile the replacement string.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

 private:
   /**
    * @short Centralizes the translation string.
    */
   static inline QString errorAtEnd(const char ch);

   /**
    * Reads the string in the third argument and converts it to a a QRegExp compatible
    * replacement string, containing sub-group references and so forth.
    */
   QString parseReplacement(const int captureCount, const DynamicContext::Ptr &context) const;

   QString m_replacementString;
};

class TokenizeFN : public PatternPlatform
{
 public:
   TokenizeFN();
   inline Item mapToItem(const QString &subject, const DynamicContext::Ptr &) const;
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;

 private:
   typedef QExplicitlySharedDataPointer<const TokenizeFN> ConstPtr;
};
}

QT_END_NAMESPACE

#endif
