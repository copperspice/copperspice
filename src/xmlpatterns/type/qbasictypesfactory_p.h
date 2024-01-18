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

#ifndef QBasicTypesFactory_P_H
#define QBasicTypesFactory_P_H

#include <QHash>
#include <qschematypefactory_p.h>

namespace QPatternist {

class BasicTypesFactory : public SchemaTypeFactory
{
 public:

   /**
    * Creates a primitive type for @p name. If @p name is not supported,
    * @c null is returned.
    * The intened supported types are the builtin primitive and derived types.
    * That is, the 19 W3C XML Schema types, and the additional 5 in the XPath Data MOdel.
    *
    * @note This does not handle user defined types, only builtin types.
    * @todo Update documentation, proportionally with progress.
    */
   SchemaType::Ptr createSchemaType(const QXmlName ) const override;

   SchemaType::Hash types() const override;

   /**
    * @returns the singleton instance of BasicTypesFactory.
    */
   static SchemaTypeFactory::Ptr self(const NamePool::Ptr &np);

 protected:
   /**
    * This constructor is protected. Use the static self() function
    * to retrieve a singleton instance.
    */
   BasicTypesFactory(const NamePool::Ptr &np);

 private:
   /**
    * A dictonary of builtin primitive and derived primitives.
    */
   SchemaType::Hash m_types;
};

}

#endif
