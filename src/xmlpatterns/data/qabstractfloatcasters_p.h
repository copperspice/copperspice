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

#ifndef QAbstractFloatCasters_P_H
#define QAbstractFloatCasters_P_H

#include <qabstractfloat_p.h>
#include <qatomiccaster_p.h>
#include <qschemanumeric_p.h>

namespace QPatternist {

template <const bool isDouble>
class NumericToAbstractFloatCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

template <const bool isDouble>
class StringToAbstractFloatCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

template <const bool isDouble>
class BooleanToAbstractFloatCaster : public AtomicCaster
{
 public:
   Item castFrom(const Item &from, const QExplicitlySharedDataPointer<DynamicContext> &context) const override;
};

#include "qabstractfloatcasters.cpp"

typedef NumericToAbstractFloatCaster<true> NumericToDoubleCaster;
typedef NumericToAbstractFloatCaster<false> NumericToFloatCaster;
typedef StringToAbstractFloatCaster<true> StringToDoubleCaster;
typedef StringToAbstractFloatCaster<false> StringToFloatCaster;
typedef BooleanToAbstractFloatCaster<true> BooleanToDoubleCaster;
typedef BooleanToAbstractFloatCaster<false> BooleanToFloatCaster;
}

#endif
