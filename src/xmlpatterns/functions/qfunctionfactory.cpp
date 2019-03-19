/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qfunctionsignature_p.h"

#include "qfunctionfactory_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

FunctionFactory::~FunctionFactory()
{
}

bool FunctionFactory::isAvailable(const NamePool::Ptr &np,
                                  const QXmlName name, const xsInteger arity)
{
   const FunctionSignature::Ptr sign(retrieveFunctionSignature(np, name));

   if (sign) {
      return arity == FunctionSignature::UnlimitedArity || sign->isArityValid(arity);
   } else {
      return false;
   }
}

bool FunctionFactory::hasSignature(const FunctionSignature::Ptr &signature) const
{
   const FunctionSignature::Hash signs(functionSignatures());
   const FunctionSignature::Hash::const_iterator end(signs.constEnd());
   FunctionSignature::Hash::const_iterator it(signs.constBegin());

   for (; it != end; ++it) {
      if (*(*it) == *signature) {
         return true;
      }
   }

   return false;
}
QT_END_NAMESPACE
