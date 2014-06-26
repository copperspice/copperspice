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

#ifndef QXsdUserSchemaType_P_H
#define QXsdUserSchemaType_P_H

#include <qnamedschemacomponent_p.h>
#include <qschematype_p.h>
#include <qxsdannotated_p.h>

template<typename N, typename M> class QHash;
template<typename N> class QList;

QT_BEGIN_NAMESPACE

namespace QPatternist {
template<typename TSuperClass>
class XsdUserSchemaType : public TSuperClass, public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdUserSchemaType> Ptr;

   /**
    * Sets the @p name of the type.
    */
   void setName(const QXmlName &name);

   /**
    * Returns the name of the type.
    *
    * @param namePool The pool the name belongs to.
    */
   virtual QXmlName name(const NamePool::Ptr &namePool) const;

   /**
    * Returns the display name of the type.
    *
    * @param namePool The pool the name belongs to.
    */
   virtual QString displayName(const NamePool::Ptr &namePool) const;

   /**
    * Sets the derivation @p constraints of the type.
    */
   void setDerivationConstraints(const SchemaType::DerivationConstraints &constraints);

   /**
    * Returns the derivation constraints of the type.
    */
   SchemaType::DerivationConstraints derivationConstraints() const;

 private:
   QXmlName m_name;
   SchemaType::DerivationConstraints m_derivationConstraints;
};

#include "qxsduserschematype.cpp"
}

QT_END_NAMESPACE


#endif
