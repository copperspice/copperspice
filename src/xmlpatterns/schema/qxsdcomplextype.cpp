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

#include "qxsdcomplextype_p.h"

using namespace QPatternist;

void XsdComplexType::OpenContent::setMode(Mode mode)
{
   m_mode = mode;
}

XsdComplexType::OpenContent::Mode XsdComplexType::OpenContent::mode() const
{
   return m_mode;
}

void XsdComplexType::OpenContent::setWildcard(const XsdWildcard::Ptr &wildcard)
{
   m_wildcard = wildcard;
}

XsdWildcard::Ptr XsdComplexType::OpenContent::wildcard() const
{
   return m_wildcard;
}

void XsdComplexType::ContentType::setVariety(Variety variety)
{
   m_variety = variety;
}

XsdComplexType::ContentType::Variety XsdComplexType::ContentType::variety() const
{
   return m_variety;
}

void XsdComplexType::ContentType::setParticle(const XsdParticle::Ptr &particle)
{
   m_particle = particle;
}

XsdParticle::Ptr XsdComplexType::ContentType::particle() const
{
   return m_particle;
}

void XsdComplexType::ContentType::setOpenContent(const OpenContent::Ptr &content)
{
   m_openContent = content;
}

XsdComplexType::OpenContent::Ptr XsdComplexType::ContentType::openContent() const
{
   return m_openContent;
}

void XsdComplexType::ContentType::setSimpleType(const AnySimpleType::Ptr &type)
{
   m_simpleType = type;
}

AnySimpleType::Ptr XsdComplexType::ContentType::simpleType() const
{
   return m_simpleType;
}


XsdComplexType::XsdComplexType()
   : m_isAbstract(false)
   , m_contentType(new ContentType())
{
   m_contentType->setVariety(ContentType::Empty);
}

void XsdComplexType::setIsAbstract(bool abstract)
{
   m_isAbstract = abstract;
}

bool XsdComplexType::isAbstract() const
{
   return m_isAbstract;
}

QString XsdComplexType::displayName(const NamePool::Ptr &np) const
{
   return np->displayName(name(np));
}

void XsdComplexType::setWxsSuperType(const SchemaType::Ptr &type)
{
   m_superType = type.data();
}

SchemaType::Ptr XsdComplexType::wxsSuperType() const
{
   return SchemaType::Ptr(m_superType);
}

void XsdComplexType::setContext(const NamedSchemaComponent::Ptr &component)
{
   m_context = component.data();
}

NamedSchemaComponent::Ptr XsdComplexType::context() const
{
   return NamedSchemaComponent::Ptr(m_context);
}

void XsdComplexType::setContentType(const ContentType::Ptr &type)
{
   m_contentType = type;
}

XsdComplexType::ContentType::Ptr XsdComplexType::contentType() const
{
   return m_contentType;
}

void XsdComplexType::setAttributeUses(const XsdAttributeUse::List &attributeUses)
{
   m_attributeUses = attributeUses;
}

void XsdComplexType::addAttributeUse(const XsdAttributeUse::Ptr &attributeUse)
{
   m_attributeUses.append(attributeUse);
}

XsdAttributeUse::List XsdComplexType::attributeUses() const
{
   return m_attributeUses;
}

void XsdComplexType::setAttributeWildcard(const XsdWildcard::Ptr &wildcard)
{
   m_attributeWildcard = wildcard;
}

XsdWildcard::Ptr XsdComplexType::attributeWildcard() const
{
   return m_attributeWildcard;
}

XsdComplexType::TypeCategory XsdComplexType::category() const
{
   return ComplexType;
}

void XsdComplexType::setDerivationMethod(DerivationMethod method)
{
   m_derivationMethod = method;
}

XsdComplexType::DerivationMethod XsdComplexType::derivationMethod() const
{
   return m_derivationMethod;
}

void XsdComplexType::setProhibitedSubstitutions(const BlockingConstraints &substitutions)
{
   m_prohibitedSubstitutions = substitutions;
}

XsdComplexType::BlockingConstraints XsdComplexType::prohibitedSubstitutions() const
{
   return m_prohibitedSubstitutions;
}

void XsdComplexType::setAssertions(const XsdAssertion::List &assertions)
{
   m_assertions = assertions;
}

void XsdComplexType::addAssertion(const XsdAssertion::Ptr &assertion)
{
   m_assertions.append(assertion);
}

XsdAssertion::List XsdComplexType::assertions() const
{
   return m_assertions;
}

bool XsdComplexType::isDefinedBySchema() const
{
   return true;
}

