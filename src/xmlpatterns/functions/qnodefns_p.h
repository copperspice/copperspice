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

#ifndef Patternist_NodeFNs_P_H
#define Patternist_NodeFNs_P_H

#include "qfunctioncall_p.h"
#include "qcastingplatform_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    class NameFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };
   
    class LocalNameFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };
   
    class NamespaceURIFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    class NumberFN : public FunctionCall,
                     public CastingPlatform<NumberFN, false>
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
  
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);
   
        inline ItemType::Ptr targetType() const
        {
            return BuiltinTypes::xsDouble;
        }
    };

  
    class LangFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

    private:
        static inline bool isLangMatch(const QString &candidate, const QString &toMatch);
    };

  
    class RootFN : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual SequenceType::Ptr staticType() const;
    };
}

QT_END_NAMESPACE

#endif
