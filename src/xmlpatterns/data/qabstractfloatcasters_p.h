/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QAbstractFloatCasters_P_H
#define QAbstractFloatCasters_P_H

#include <qabstractfloat_p.h>
#include <qatomiccaster_p.h>
#include <qschemanumeric_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

template <const bool isDouble>
class NumericToAbstractFloatCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};

template <const bool isDouble>
class StringToAbstractFloatCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};

template <const bool isDouble>
class BooleanToAbstractFloatCaster : public AtomicCaster
{
 public:
   virtual Item castFrom(const Item &from,
                         const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};

#include "qabstractfloatcasters.cpp"

typedef NumericToAbstractFloatCaster<true> NumericToDoubleCaster;
typedef NumericToAbstractFloatCaster<false> NumericToFloatCaster;
typedef StringToAbstractFloatCaster<true> StringToDoubleCaster;
typedef StringToAbstractFloatCaster<false> StringToFloatCaster;
typedef BooleanToAbstractFloatCaster<true> BooleanToDoubleCaster;
typedef BooleanToAbstractFloatCaster<false> BooleanToFloatCaster;
}

QT_END_NAMESPACE

#endif
