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

#ifndef QVARIABLELOADER_P_H
#define QVARIABLELOADER_P_H

#include <QSet>
#include <QXmlQuery>
#include <QDebug>

#include <qdynamiccontext_p.h>
#include <qexternalvariableloader_p.h>

namespace QPatternist {

class VariableLoader : public ExternalVariableLoader
{
 public:
   typedef QHash<QXmlName, QVariant> BindingHash;
   typedef QExplicitlySharedDataPointer<VariableLoader> Ptr;

   inline VariableLoader(const NamePool::Ptr &np,
                  const VariableLoader::Ptr &previousLoader = VariableLoader::Ptr())
                  : m_namePool(np), m_previousLoader(previousLoader)
   {
   }

   QPatternist::SequenceType::Ptr announceExternalVariable(const QXmlName name,
                  const QPatternist::SequenceType::Ptr &declaredType) override;

   virtual QPatternist::Item::Iterator::Ptr evaluateSequence(const QXmlName name,
                  const QPatternist::DynamicContext::Ptr &) override;

   virtual QPatternist::Item evaluateSingleton(const QXmlName name,
                  const QPatternist::DynamicContext::Ptr &) override;

   void removeBinding(const QXmlName &name);
   bool hasBinding(const QXmlName &name) const;
   QVariant valueFor(const QXmlName &name) const;
   void addBinding(const QXmlName &name, const QVariant &value);

   bool isSameType(const QVariant &v1, const QVariant &v2) const;

   bool invalidationRequired(const QXmlName &name, const QVariant &variant) const;

 private:

   inline QPatternist::Item itemForName(const QXmlName &name) const;

   const NamePool::Ptr                 m_namePool;
   VariableLoader::Ptr                 m_previousLoader;
   BindingHash                         m_bindingHash;
};
}

CS_DECLARE_METATYPE(QXmlQuery)

#endif
