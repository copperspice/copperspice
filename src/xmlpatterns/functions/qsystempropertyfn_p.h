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

#ifndef QSystemPropertyFN_P_H
#define QSystemPropertyFN_P_H

#include <qstaticnamespacescontainer_p.h>

namespace QPatternist {
class SystemPropertyFN : public StaticNamespacesContainer
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

 private:
   /**
    * Returns a string representation for @p property as defined
    * for the system properties in "XSL Transformations (XSLT)
    * Version 2.0, 16.6.5 system-property". Hence, this function
    * handles only the properties specified in the XSL namespace, and returns
    * an empty string if an unrecognized property is asked for.
    */
   static QString retrieveProperty(const QXmlName name);
};

}

#endif
