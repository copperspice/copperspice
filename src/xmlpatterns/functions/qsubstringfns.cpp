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

#include "qboolean_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"
#include "qatomicstring_p.h"

#include "qsubstringfns_p.h"

using namespace QPatternist;

Item ContainsFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item op1(m_operands.first()->evaluateSingleton(context));
   QString str1;

   if (op1) {
      str1 = op1.stringValue();
   }

   const Item op2(m_operands.at(1)->evaluateSingleton(context));
   QString str2;

   if (op2) {
      str2 = op2.stringValue();
   }

   if (str2.isEmpty()) {
      return CommonValues::BooleanTrue;
   }

   if (str1.isEmpty()) {
      return CommonValues::BooleanFalse;
   }

   return Boolean::fromValue(str1.contains(str2, caseSensitivity()));
}

Item StartsWithFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item op1(m_operands.first()->evaluateSingleton(context));
   QString str1;

   if (op1) {
      str1 = op1.stringValue();
   }

   const Item op2(m_operands.at(1)->evaluateSingleton(context));
   QString str2;

   if (op2) {
      str2 = op2.stringValue();
   }

   if (str2.isEmpty()) {
      return CommonValues::BooleanTrue;
   }

   if (str1.isEmpty()) {
      return CommonValues::BooleanFalse;
   }

   return Boolean::fromValue(str1.startsWith(str2, caseSensitivity()));
}

Item EndsWithFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item op1(m_operands.first()->evaluateSingleton(context));
   QString str1;

   if (op1) {
      str1 = op1.stringValue();
   }

   const Item op2(m_operands.at(1)->evaluateSingleton(context));
   QString str2;

   if (op2) {
      str2 = op2.stringValue();
   }

   if (str2.isEmpty()) {
      return CommonValues::BooleanTrue;
   }

   if (str1.isEmpty()) {
      return CommonValues::BooleanFalse;
   }

   return Boolean::fromValue(str1.endsWith(str2, caseSensitivity()));
}

Item SubstringBeforeFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item op1(m_operands.first()->evaluateSingleton(context));
   QString str1;

   if (op1) {
      str1 = op1.stringValue();
   }

   const Item op2(m_operands.at(1)->evaluateSingleton(context));
   QString str2;

   if (op2) {
      str2 = op2.stringValue();
   }

   const int pos = str1.indexOf(str2);
   if (pos == -1) {
      return CommonValues::EmptyString;
   }

   return AtomicString::fromValue(QString(str1.left(pos)));
}

Item SubstringAfterFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item op1(m_operands.first()->evaluateSingleton(context));
   QString str1;

   if (op1) {
      str1 = op1.stringValue();
   }

   const Item op2(m_operands.at(1)->evaluateSingleton(context));
   QString str2;

   if (op2) {
      str2 = op2.stringValue();
   }

   if (str2.isEmpty()) {
      if (op1) {
         return op1;
      } else {
         return CommonValues::EmptyString;
      }
   }

   const int pos = str1.indexOf(str2);

   if (pos == -1) {
      return CommonValues::EmptyString;
   }

   return AtomicString::fromValue(QString(str1.right(str1.length() - (pos + str2.length()))));
}
