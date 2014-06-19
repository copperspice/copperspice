/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef Patternist_SequenceGeneratingFNs_P_H
#define Patternist_SequenceGeneratingFNs_P_H

#include "qanyuri_p.h"
#include "qcontextnodechecker_p.h"
#include "qstaticbaseuricontainer_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
class IdFN : public ContextNodeChecker
{
 public:
   IdFN();
   typedef QPair<DynamicContext::Ptr, const QAbstractXmlNodeModel *> IDContext;

   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;

   inline Item mapToItem(const QString &id,
                         const IDContext &context) const;

   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

 private:
   typedef QExplicitlySharedDataPointer<const IdFN> ConstPtr;
   bool m_hasCreatedSorter;
};

class IdrefFN : public ContextNodeChecker
{
 public:
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
};

class DocFN : public StaticBaseUriContainer
{
 public:
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType);
   virtual SequenceType::Ptr staticType() const;

 private:
   SequenceType::Ptr m_type;
};

class DocAvailableFN : public StaticBaseUriContainer
{
 public:
   virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
};


class CollectionFN : public FunctionCall
{
 public:
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
};
}

QT_END_NAMESPACE

#endif
