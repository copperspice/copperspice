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

#ifndef QDerivedString_P_H
#define QDerivedString_P_H

#include <qregularexpression.h>
#include <qxmlutils_p.h>
#include <qbuiltintypes_p.h>
#include <qpatternistlocale_p.h>
#include <qvalidationerror_p.h>
#include <qstringfwd.h>

namespace QPatternist {

template<TypeOfDerivedString DerivedType>
class DerivedString : public AtomicValue
{
 private:
   static ItemType::Ptr itemType() {
      switch (DerivedType) {
         case TypeNormalizedString:
            return BuiltinTypes::xsNormalizedString;
         case TypeToken:
            return BuiltinTypes::xsToken;
         case TypeLanguage:
            return BuiltinTypes::xsLanguage;
         case TypeNMTOKEN:
            return BuiltinTypes::xsNMTOKEN;
         case TypeName:
            return BuiltinTypes::xsName;
         case TypeNCName:
            return BuiltinTypes::xsNCName;
         case TypeID:
            return BuiltinTypes::xsID;
         case TypeIDREF:
            return BuiltinTypes::xsIDREF;
         case TypeENTITY:
            return BuiltinTypes::xsENTITY;
         case TypeString:
            return BuiltinTypes::xsString;
      }

      Q_ASSERT_X(false, Q_FUNC_INFO, "This line is not supposed to be reached.");
      return ItemType::Ptr();
   }

   const QString m_value;

   DerivedString(const QString &value)
      : m_value(value) {
   }

   static bool isNameChar(const QChar &ch) {
      return ch.isLetter()            ||
             ch.isDigit()             ||
             ch == QLatin1Char('.')   ||
             ch == QLatin1Char('-')   ||
             ch == QLatin1Char('_')   ||
             ch == QLatin1Char(':');
   }

   static bool isValidName(const QString &input) {
      if (input.isEmpty()) {
         return false;
      }

      const QChar first(input.at(0));

      if (first.isLetter()             ||
            first == QLatin1Char('_')    ||
            first == QLatin1Char(':')) {
         const int len = input.length();

         if (len == 1) {
            return true;
         }

         for (int i = 1; i < len; ++i) {
            if (!isNameChar(input.at(i))) {
               return false;
            }
         }

         return true;
      } else {
         return false;
      }
   }

   static bool isValidNMTOKEN(const QString &input) {
      const int len = input.length();

      if (len == 0) {
         return false;
      }

      for (int i = 0; i < len; ++i) {
         if (!isNameChar(input.at(i))) {
            return false;
         }
      }

      return true;
   }

   static QString attributeNormalize(const QString &input) {
      QString retval(input);
      const int len = retval.length();
      const QChar space(' ');

      for (int i = 0; i < len; ++i) {
         const QChar ati(retval.at(i));

         if (ati.isSpace() && ati != space) {
            retval.replace(i, 1, space);
         }
      }

      return retval;
   }

   static AtomicValue::Ptr error(const NamePool::Ptr &np, const QString &invalidValue) {
      return ValidationError::createError(QString("%1 is not a valid value for type %2.")
                  .formatArg(formatData(invalidValue)).formatArg(formatType(np, itemType())));
   }

 public:
   static AtomicValue::Ptr fromValue(const QString &value) {
      return AtomicValue::Ptr(new DerivedString(value));
   }

   static AtomicValue::Ptr fromLexical(const NamePool::Ptr &np, const QString &lexical) {
      switch (DerivedType) {
         case TypeString:
            return AtomicValue::Ptr(new DerivedString(lexical));

         case TypeNormalizedString:
            return AtomicValue::Ptr(new DerivedString(attributeNormalize(lexical)));

         case TypeToken:
            return AtomicValue::Ptr(new DerivedString(lexical.simplified()));

         case TypeLanguage: {
            const QString simplified(lexical.trimmed());

            const QRegularExpression validate("[a-zA-Z]{1,8}(-[a-zA-Z0-9]{1,8})*", QPatternOption::ExactMatchOption);
            Q_ASSERT(validate.isValid());

            if (validate.match(simplified).hasMatch()) {
               return AtomicValue::Ptr(new DerivedString(lexical.simplified()));
            } else {
               return error(np, simplified);
            }
         }
         case TypeNMTOKEN: {
            const QString trimmed(lexical.trimmed());

            if (isValidNMTOKEN(trimmed)) {
               return AtomicValue::Ptr(new DerivedString(trimmed));
            } else {
               return error(np, trimmed);
            }
         }
         case TypeName: {
            const QString simplified(lexical.simplified());

            if (isValidName(simplified)) {
               return AtomicValue::Ptr(new DerivedString(simplified));
            } else {
               return error(np, simplified);
            }
         }

         case TypeID:
         case TypeIDREF:
         case TypeENTITY:
         case TypeNCName: {
            const QString trimmed(lexical.trimmed());

            if (QXmlUtils::isNCName(trimmed)) {
               return AtomicValue::Ptr(new DerivedString(trimmed));
            } else {
               return error(np, trimmed);
            }
         }

         default: {
            Q_ASSERT_X(false, Q_FUNC_INFO, "This line is not supposed to be reached.");
            return AtomicValue::Ptr();
         }
      }
   }

   QString stringValue() const override {
      return m_value;
   }

   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const  override {
      return m_value.length() > 0;
   }

   ItemType::Ptr type() const override {
      return itemType();
   }
};
}

#endif
