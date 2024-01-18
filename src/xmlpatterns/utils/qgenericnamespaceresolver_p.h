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

#ifndef QGenericNamespaceResolver_P_H
#define QGenericNamespaceResolver_P_H

#include <qhash.h>

#include <qnamespaceresolver_p.h>

namespace QPatternist {
class GenericNamespaceResolver : public NamespaceResolver
{
 public:
   GenericNamespaceResolver(const Bindings &list);
   void addBinding(const QXmlName nb) override;

   QXmlName::NamespaceCode lookupNamespaceURI(const QXmlName::PrefixCode prefix) const override;

   /**
    * Returns a GenericNamespaceResolver containing the following bindings:
    *
    * - <tt>xml</tt> = <tt>http://www.w3.org/XML/1998/namespace</tt>
    * - <tt>xs</tt> = <tt>http://www.w3.org/2001/XMLSchema</tt>
    * - <tt>xsi</tt> = <tt>http://www.w3.org/2001/XMLSchema-instance</tt>
    * - <tt>fn</tt> = <tt>http://www.w3.org/2005/xpath-functions</tt>
    * - <tt>xdt</tt> = <tt>http://www.w3.org/2005/xpath-datatypes</tt>
    * - no prefix = empty namespace
    */
   static NamespaceResolver::Ptr defaultXQueryBindings();

   /**
    * Returns a GenericNamespaceResolver containing the following bindings:
    *
    * - <tt>xml</tt> = <tt>http://www.w3.org/XML/1998/namespace</tt>
    * - no prefix = empty namespace
    */
   static NamespaceResolver::Ptr defaultXSLTBindings();

   Bindings bindings() const override;

 private:
   /**
    * The key is the prefix, the value the namespace URI.
    */
   Bindings m_bindings;
};
}

#endif
