/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QXsdUserSchemaType_P_H
#define QXsdUserSchemaType_P_H

#include <qnamedschemacomponent_p.h>
#include <qschematype_p.h>
#include <qxsdannotated_p.h>
#include <qcontainerfwd.h>

namespace QPatternist {

template<typename TSuperClass>
class XsdUserSchemaType : public TSuperClass, public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdUserSchemaType> Ptr;

   void setName(const QXmlName &name);
   QXmlName name(const NamePool::Ptr &namePool) const override;

   QString displayName(const NamePool::Ptr &namePool) const override;

   void setDerivationConstraints(const SchemaType::DerivationConstraints &constraints);
   SchemaType::DerivationConstraints derivationConstraints() const override;

 private:
   QXmlName m_name;
   SchemaType::DerivationConstraints m_derivationConstraints;
};

#include "qxsduserschematype.cpp"

}

#endif
