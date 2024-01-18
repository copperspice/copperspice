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
   static inline ItemType::Ptr itemType() {
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

   inline DerivedString(const QString &value) : m_value(value) {
   }

   /**
    * @short This is an incomplete test for whether @p ch conforms to
    * the XML 1.0 NameChar production.
    */
   static inline bool isNameChar(const QChar &ch) {
      return ch.isLetter()            ||
             ch.isDigit()             ||
             ch == QLatin1Char('.')   ||
             ch == QLatin1Char('-')   ||
             ch == QLatin1Char('_')   ||
             ch == QLatin1Char(':');
   }

   /**
    * @returns @c true if @p input is a valid @c xs:Name.
    * @see <a href="http://www.w3.org/TR/REC-xml/#NT-Name">Extensible
    * Markup Language (XML) 1.0 (Fourth Edition), [5] Name</a>
    */
   static inline bool isValidName(const QString &input) {
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

         /* Since we've checked the first character above, we start at
          * position 1. */
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

   /**
    * @returns @c true if @p input conforms to the XML 1.0 @c Nmtoken product.
    *
    * @see <a
    * href="http://www.w3.org/TR/2000/WD-xml-2e-20000814#NT-Nmtoken">Extensible
    * Markup Language (XML) 1.0 (Second Edition), [7] Nmtoken</a>
    */
   static inline bool isValidNMTOKEN(const QString &input) {
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

   /**
    * @short Performs attribute value normalization as if @p input was not
    * from a @c CDATA section.
    *
    * Each whitespace character in @p input that's not a space, such as tab
    * or new line character, is replaced with a space. This algorithm
    * differs from QString::simplified() in that it doesn't collapse
    * subsequent whitespace characters to a single one, or remove trailing
    * and leading space.
    *
    * @see <a href="http://www.w3.org/TR/REC-xml/#AVNormalize">Extensible
    * Markup Language (XML) 1.0 (Second Edition), 3.3.3 [E70]Attribute-Value Normalization</a>
    */
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

   /**
    * @note This function doesn't perform any cleanup/normalizaiton of @p
    * value. @p value must be a canonical value space of the type.
    *
    * If you want cleanup to be performed and/or the lexical space
    * checked, use fromLexical().
    */
   static AtomicValue::Ptr fromValue(const QString &value) {
      return AtomicValue::Ptr(new DerivedString(value));
   }

   /**
    * Constructs an instance from the lexical
    * representation @p lexical.
    */
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
            /* We treat xs:ID, xs:ENTITY, xs:IDREF and xs:NCName in the exact same
             * way, except for the type annotation.
             *
             * We use trimmed() instead of simplified() because it's
             * faster and whitespace isn't allowed between
             * non-whitespace characters anyway, for these types. */
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
