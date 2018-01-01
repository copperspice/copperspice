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

#ifndef QNodeFNs_P_H
#define QNodeFNs_P_H

#include <qfunctioncall_p.h>
#include <qcastingplatform_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class NameFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class LocalNameFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class NamespaceURIFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class NumberFN : public FunctionCall,
   public CastingPlatform<NumberFN, false>
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   ItemType::Ptr targetType() const {
      return BuiltinTypes::xsDouble;
   }
};

class LangFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

 private:
   static inline bool isLangMatch(const QString &candidate, const QString &toMatch);
};


class RootFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   SequenceType::Ptr staticType() const override;
};
}

QT_END_NAMESPACE

#endif
