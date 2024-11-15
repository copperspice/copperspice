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

#ifndef QNamedSchemaComponent_P_H
#define QNamedSchemaComponent_P_H

#include <qxmlname.h>

#include <qnamepool_p.h>
#include <qschemacomponent_p.h>

namespace QPatternist {

class NamedSchemaComponent : public SchemaComponent
{
 public:
   typedef QExplicitlySharedDataPointer<NamedSchemaComponent> Ptr;

   enum BlockingConstraint {
      RestrictionConstraint  = 1,
      ExtensionConstraint    = 2,
      SubstitutionConstraint = 4
   };
   using BlockingConstraints = QFlags<BlockingConstraint>;

   NamedSchemaComponent();

   virtual ~NamedSchemaComponent();

   void setName(const QXmlName &name);
   virtual QXmlName name(const NamePool::Ptr &namePool) const;

   virtual QString displayName(const NamePool::Ptr &namePool) const;

 private:
   QXmlName m_name;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NamedSchemaComponent::BlockingConstraints)

}

#endif
