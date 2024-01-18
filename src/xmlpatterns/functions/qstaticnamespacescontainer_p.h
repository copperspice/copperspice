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

#ifndef QStaticNamespacesContainer_P_H
#define QStaticNamespacesContainer_P_H

#include <qfunctioncall_p.h>

namespace QPatternist {

class StaticNamespacesContainer : public FunctionCall
{
 public:
   /**
    * Reimplemented to store data from the @p context.
    */
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

 protected:
   /**
    * Before typeCheck(), behavior of this function is undefined. After
    * typeCheck(), this function guarantees to return a valid pointer.
    */
   const NamespaceResolver::Ptr &staticNamespaces() const {
      Q_ASSERT(m_resolver);
      return m_resolver;
   }

   /**
    * This constructor only exists to ensure this class is subclassed.
    */
   StaticNamespacesContainer() {
   }

 private:
   NamespaceResolver::Ptr m_resolver;
};

}

#endif
