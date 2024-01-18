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

#ifndef QNamespaceResolver_P_H
#define QNamespaceResolver_P_H

#include <qcontainerfwd.h>
#include <qshareddata.h>
#include <qxmlname.h>

namespace QPatternist {

class NamespaceResolver : public QSharedData
{
 public:
   enum Constants {
      NoBinding = -1
   };

   typedef QExplicitlySharedDataPointer<NamespaceResolver> Ptr;

   /**
    * A list of namespace bindings. The key is the prefix, and the value is
    * the namespace URI.
    */
   typedef QHash<QXmlName::PrefixCode, QXmlName::NamespaceCode> Bindings;

   NamespaceResolver();
   virtual ~NamespaceResolver();

   /**
    * Adds the mapping from @p prefix to @p namespaceURI to
    * this NamespaceResolver. If this NamespaceResolver already contains
    * a binding involving @p prefix, the old binding is replaced.
    */
   virtual void addBinding(const QXmlName nb) = 0;

   /**
    * Resolves the @p prefix to the corresponding namespace URI. If no binding
    * exists for @p prefix, NoBinding is returned.
    *
    * @returns the namespace corresponding to @p prefix.
    */
   virtual QXmlName::NamespaceCode lookupNamespaceURI(const QXmlName::PrefixCode prefix) const = 0;

   /**
    * @returns all bindings this NamespaceResolver handles.
    */
   virtual Bindings bindings() const = 0;
};

}

#endif
