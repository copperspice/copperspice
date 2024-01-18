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

#ifndef QSequenceGeneratingFNs_P_H
#define QSequenceGeneratingFNs_P_H

#include <qanyuri_p.h>
#include <qcontextnodechecker_p.h>
#include <qstaticbaseuricontainer_p.h>

namespace QPatternist {

class IdFN : public ContextNodeChecker
{
 public:
   IdFN();
   typedef QPair<DynamicContext::Ptr, const QAbstractXmlNodeModel *> IDContext;

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;

   inline Item mapToItem(const QString &id, const IDContext &context) const;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

 private:
   typedef QExplicitlySharedDataPointer<const IdFN> ConstPtr;
   bool m_hasCreatedSorter;
};

class IdrefFN : public ContextNodeChecker
{
 public:
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
};

class DocFN : public StaticBaseUriContainer
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   SequenceType::Ptr staticType() const override;

 private:
   SequenceType::Ptr m_type;
};

class DocAvailableFN : public StaticBaseUriContainer
{
 public:
   bool evaluateEBV(const DynamicContext::Ptr &context) const override;
};


class CollectionFN : public FunctionCall
{
 public:
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
};
}

#endif
