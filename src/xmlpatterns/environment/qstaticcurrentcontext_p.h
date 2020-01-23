/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QStaticCurrentContext_P_H
#define QStaticCurrentContext_P_H

#include <qdelegatingstaticcontext_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class StaticCurrentContext : public DelegatingStaticContext
{
 public:
   StaticCurrentContext(const ItemType::Ptr &currentItemType,
                        const StaticContext::Ptr &context);
   /**
    * @returns the type passed in the constructor.
    */
   ItemType::Ptr currentItemType() const override;

 private:
   const ItemType::Ptr         m_currentItemType;
};
}

QT_END_NAMESPACE

#endif
