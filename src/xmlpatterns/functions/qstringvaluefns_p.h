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

#ifndef QStringValueFNs_P_H
#define QStringValueFNs_P_H

#include <QByteArray>
#include <qfunctioncall_p.h>

namespace QPatternist {

class ConcatFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class StringJoinFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   /**
    * Optimization: when the cardinality of the sequence of items to join
    * cannot be two or more, we have no effect and therefore rewrite
    * ourselves to our first operand.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;
};


class SubstringFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};


class StringLengthFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class NormalizeSpaceFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class NormalizeUnicodeFN : public FunctionCall
{
 public:
   /**
    * Initializes private data.
    */
   NormalizeUnicodeFN();
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

 private:
   int determineNormalizationForm(const DynamicContext::Ptr &context) const;
   QString::NormalizationForm m_normForm;
};

class UpperCaseFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class LowerCaseFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class TranslateFN : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};

class EncodeString : public FunctionCall
{
 public:
   /**
    * Evaluates its first operand. If it is the empty sequence, an empty string
    * is returned. Otherwise, the item's string value is returned percent encoded
    * as specified in this class's constructor.
    */
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

 protected:
   /**
    * Encodes its operand with QUrl::toPercentEncoding(), with @p includeChars as
    * the characters to encode, and @p excludeChars as the characters to not encode.
    */
   EncodeString(const QByteArray &excludeChars, const QByteArray &includeChars);

 private:
   const QByteArray m_excludeChars;
   const QByteArray m_includeChars;
};

class EncodeForURIFN : public EncodeString
{
 public:
   /**
    * Performs internal initialization.
    */
   EncodeForURIFN();

 private:
   static const char *const include;
};

class IriToURIFN : public EncodeString
{
 public:
   /**
    * Performs internal initialization.
    */
   IriToURIFN();

 private:
   static const char *const exclude;
};

class EscapeHtmlURIFN : public EncodeString
{
 public:
   /**
    * Performs internal initialization.
    */
   EscapeHtmlURIFN();

 private:
   static const char *const include;
   static const char *const exclude;
};

}

#endif
