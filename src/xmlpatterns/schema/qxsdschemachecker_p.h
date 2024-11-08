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

#ifndef QXsdSchemaChecker_P_H
#define QXsdSchemaChecker_P_H

#include <qschematype_p.h>
#include <qxsdattribute_p.h>
#include <qxsdattributegroup_p.h>
#include <qxsdelement_p.h>
#include <qxsdmodelgroup_p.h>
#include <qxsdnotation_p.h>
#include <qxsdschema_p.h>
#include <qxsdsimpletype_p.h>

#include <QExplicitlySharedDataPointer>

namespace QPatternist {

class XsdSchemaContext;
class XsdSchemaParserContext;

class XsdSchemaChecker : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<XsdSchemaChecker> Ptr;

   XsdSchemaChecker(const QExplicitlySharedDataPointer<XsdSchemaContext> &context,
                    const XsdSchemaParserContext *parserContext);

   ~XsdSchemaChecker();

   void basicCheck();
   void check();
   void checkConstrainingFacets();
   void addComponentLocationHash(const QHash<NamedSchemaComponent::Ptr, QSourceLocation> &hash);

 private:
   void checkSimpleRestrictionBaseType();
   void checkBasicCircularInheritances();
   void checkCircularInheritances();
   void checkInheritanceRestrictions();
   void checkBasicSimpleTypeConstraints();
   void checkSimpleTypeConstraints();
   void checkBasicComplexTypeConstraints();
   void checkComplexTypeConstraints();
   void checkSimpleDerivationRestrictions();
   void checkConstrainingFacets(const XsdFacet::Hash &facets, const XsdSimpleType::Ptr &simpleType);
   void checkDuplicatedAttributeUses();

   void checkElementConstraints();
   void checkAttributeConstraints();
   void checkAttributeUseConstraints();

   typedef QHash<QXmlName, SchemaType::Ptr> DuplicatedElementMap;
   typedef QHash<XsdWildcard::NamespaceConstraint::Variety, XsdWildcard::Ptr> DuplicatedWildcardMap;

   void checkElementDuplicates();

   void checkElementDuplicates(const XsdParticle::Ptr &particle, DuplicatedElementMap &elementMap, DuplicatedWildcardMap &wildcardMap);

   void setupAllowedAtomicFacets();
   QSourceLocation sourceLocation(const NamedSchemaComponent::Ptr &component) const;
   QSourceLocation sourceLocationForType(const SchemaType::Ptr &type) const;

   bool isValidValue(const QString &value, const AnySimpleType::Ptr &type, QString &errorMsg) const;

   XsdFacet::Hash facetsForType(const SchemaType::Ptr &type) const;

   bool hasDuplicatedAttributeUses(const XsdAttributeUse::List &list, XsdAttribute::Ptr &conflictingAttribute) const;
   bool hasMultipleIDAttributeUses(const XsdAttributeUse::List &list) const;
   bool hasConstraintIDAttributeUse(const XsdAttributeUse::List &list, XsdAttribute::Ptr &conflictingAttribute) const;

   bool particleEqualsRecursively(const XsdParticle::Ptr &particle, const XsdParticle::Ptr &otherParticle) const;
   bool isValidParticleExtension(const XsdParticle::Ptr &extension, const XsdParticle::Ptr &base) const;
   bool elementSequenceAccepted(const XsdModelGroup::Ptr &sequence, const XsdParticle::Ptr &particle) const;

   QExplicitlySharedDataPointer<XsdSchemaContext>       m_context;
   NamePool::Ptr                                        m_namePool;
   XsdSchema::Ptr                                       m_schema;
   QHash<QXmlName, QSet<XsdFacet::Type> >               m_allowedAtomicFacets;
   QHash<NamedSchemaComponent::Ptr, QSourceLocation>    m_componentLocationHash;
};

}

#endif
