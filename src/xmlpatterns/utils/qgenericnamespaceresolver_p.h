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

   static NamespaceResolver::Ptr defaultXQueryBindings();
   static NamespaceResolver::Ptr defaultXSLTBindings();

   Bindings bindings() const override;

 private:
   Bindings m_bindings;
};

}

#endif
