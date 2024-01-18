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

#ifndef QSchemaTypeFactory_P_H
#define QSchemaTypeFactory_P_H

#include <QSharedData>
#include <qreportcontext_p.h>
#include <qitemtype_p.h>
#include <qschematype_p.h>

namespace QPatternist {

class SchemaTypeFactory : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<SchemaTypeFactory> Ptr;

   SchemaTypeFactory();
   virtual ~SchemaTypeFactory();

   /**
    * @returns a schema type for name @p name. If no schema type exists for @p name, @c null
    * is returned
    */
   virtual SchemaType::Ptr createSchemaType(const QXmlName name) const = 0;

   /**
    * @returns a dictionary containing the types this factory serves. The key
    * is the type's QName in Clark name syntax.
    */
   virtual SchemaType::Hash types() const = 0;

 private:
   SchemaTypeFactory(const SchemaTypeFactory &) = delete;
   SchemaTypeFactory &operator=(const SchemaTypeFactory &) = delete;
};
}

#endif
