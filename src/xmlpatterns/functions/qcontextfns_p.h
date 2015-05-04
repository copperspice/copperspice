/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QContextFNs_P_H
#define QContextFNs_P_H

#include <qfunctioncall_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class PositionFN : public FunctionCall
{
 public:
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
};

class LastFN : public FunctionCall
{
 public:
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
};

class ImplicitTimezoneFN : public FunctionCall
{
 public:
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
};


class CurrentDateTimeFN : public FunctionCall
{
 public:
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
};

class CurrentDateFN : public FunctionCall
{
 public:
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
};


class CurrentTimeFN : public FunctionCall
{
 public:
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
};


class DefaultCollationFN : public FunctionCall
{
 public:
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);
};


class StaticBaseURIFN : public FunctionCall
{
 public:
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);
};
}

QT_END_NAMESPACE

#endif
