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

#include <qdebug.h>
#include <qvalidator.h>

#ifndef QT_NO_VALIDATOR

#include <qlocale_p.h>

#include <limits.h>
#include <cmath>

#ifndef LLONG_MAX
#   define LLONG_MAX Q_INT64_C(0x7fffffffffffffff)
#endif

class QValidatorPrivate
{
   Q_DECLARE_PUBLIC(QValidator)

 public:
   virtual ~QValidatorPrivate() {}
   QLocale locale;

 protected:
   QValidator *q_ptr;
};

QValidator::QValidator(QObject *parent)
   : QObject(parent), d_ptr(new QValidatorPrivate)
{
   d_ptr->q_ptr = this;
}

QValidator::QValidator(QValidatorPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QValidator::~QValidator()
{
}

QLocale QValidator::locale() const
{
   Q_D(const QValidator);
   return d->locale;
}

void QValidator::setLocale(const QLocale &locale)
{
   Q_D(QValidator);

   if (d->locale != locale) {
      d->locale = locale;
      emit changed();
   }
}

void QValidator::fixup(QString &) const
{
}

QIntValidator::QIntValidator(QObject *parent)
   : QValidator(parent)
{
   b = INT_MIN;
   t = INT_MAX;
}

QIntValidator::QIntValidator(int minimum, int maximum, QObject *parent)
   : QValidator(parent)
{
   b = minimum;
   t = maximum;
}

QIntValidator::~QIntValidator()
{
}

static int numDigits(qint64 n)
{
   if (n == 0) {
      return 1;
   }

   return (int)std::log10(double(n)) + 1;
}

static qint64 pow10(int exp)
{
   qint64 result = 1;

   for (int i = 0; i < exp; ++i) {
      result *= 10;
   }

   return result;
}

QValidator::State QIntValidator::validate(QString &input, int &) const
{
   QByteArray buff;

   if (! locale().d->m_data->validateChars(input, QLocaleData::IntegerMode, &buff, -1,
         locale().numberOptions() & QLocale::RejectGroupSeparator)) {
      return Invalid;
   }

   if (buff.isEmpty()) {
      return Intermediate;
   }

   if (b >= 0 && buff.startsWith('-')) {
      return Invalid;
   }

   if (t < 0 && buff.startsWith('+')) {
      return Invalid;
   }

   if (buff.size() == 1 && (buff.at(0) == '+' || buff.at(0) == '-')) {
      return Intermediate;
   }

   bool ok;
   bool overflow;

   qint64 entered = QLocaleData::bytearrayToLongLong(buff.constData(), 10, &ok, &overflow);

   if (overflow || ! ok) {
      return Invalid;
   }

   if (entered >= b && entered <= t) {
      locale().toInt(input, &ok, 10);
      return ok ? Acceptable : Intermediate;
   }

   if (entered >= 0) {
      // the -entered < b condition is necessary to allow people to type
      // the minus last (e.g. for right-to-left languages)
      return (entered > t && -entered < b) ? Invalid : Intermediate;
   } else {
      return (entered < b) ? Invalid : Intermediate;
   }
}

void QIntValidator::fixup(QString &input) const
{
   QByteArray buff;

   if (! locale().d->m_data->validateChars(input, QLocaleData::IntegerMode, &buff, -1,
         locale().numberOptions() & QLocale::RejectGroupSeparator)) {
      return;
   }

   bool ok;
   bool overflow;

   qint64 entered = QLocaleData::bytearrayToLongLong(buff.constData(), 10, &ok, &overflow);

   if (ok && ! overflow) {
      input = locale().toString(entered);
   }
}

void QIntValidator::setRange(int bottom, int top)
{
   bool rangeChanged = false;
   if (b != bottom) {
      b = bottom;
      rangeChanged = true;
      emit bottomChanged(b);
   }

   if (t != top) {
      t = top;
      rangeChanged = true;
      emit topChanged(t);
   }

   if (rangeChanged) {
      emit changed();
   }
}

void QIntValidator::setBottom(int bottom)
{
   setRange(bottom, top());
}

void QIntValidator::setTop(int top)
{
   setRange(bottom(), top);
}

#ifndef QT_NO_REGEXP

class QDoubleValidatorPrivate : public QValidatorPrivate
{
   Q_DECLARE_PUBLIC(QDoubleValidator)

 public:
   QDoubleValidatorPrivate()
      : QValidatorPrivate(), notation(QDoubleValidator::ScientificNotation)
   { }

   QDoubleValidator::Notation notation;

   QValidator::State validateWithLocale(QString &input, QLocaleData::NumberMode numMode, const QLocale &locale) const;
};

QDoubleValidator::QDoubleValidator(QObject *parent)
   : QValidator(*new QDoubleValidatorPrivate, parent)
{
   b   = -HUGE_VAL;
   t   = HUGE_VAL;
   dec = 1000;
}

QDoubleValidator::QDoubleValidator(double bottom, double top, int decimals, QObject *parent)
   : QValidator(*new QDoubleValidatorPrivate, parent)
{
   b   = bottom;
   t   = top;
   dec = decimals;
}

QDoubleValidator::~QDoubleValidator()
{
}

QValidator::State QDoubleValidator::validate(QString &input, int &) const
{
   Q_D(const QDoubleValidator);

   QLocaleData::NumberMode numMode = QLocaleData::DoubleStandardMode;

   switch (d->notation) {
      case StandardNotation:
         numMode = QLocaleData::DoubleStandardMode;
         break;

      case ScientificNotation:
         numMode = QLocaleData::DoubleScientificMode;
         break;
   }

   return d->validateWithLocale(input, numMode, locale());
}

QValidator::State QDoubleValidatorPrivate::validateWithLocale(QString &input, QLocaleData::NumberMode numMode,
   const QLocale &locale) const
{
   Q_Q(const QDoubleValidator);

   QByteArray buff;

   if (! locale.d->m_data->validateChars(input, numMode, &buff, q->dec,
         locale.numberOptions() & QLocale::RejectGroupSeparator)) {
      return QValidator::Invalid;
   }

   if (buff.isEmpty()) {
      return QValidator::Intermediate;
   }

   if (q->b >= 0 && buff.startsWith('-')) {
      return QValidator::Invalid;
   }

   if (q->t < 0 && buff.startsWith('+')) {
      return QValidator::Invalid;
   }

   bool ok;
   bool overflow;
   double i = QLocaleData::bytearrayToDouble(buff.constData(), &ok, &overflow);

   if (overflow) {
      return QValidator::Invalid;
   }

   if (!ok) {
      return QValidator::Intermediate;
   }

   if (i >= q->b && i <= q->t) {
      return QValidator::Acceptable;
   }

   if (notation == QDoubleValidator::StandardNotation) {
      double max = qMax(qAbs(q->b), qAbs(q->t));

      if (max < double(LLONG_MAX)) {
         qint64 n = pow10(numDigits(qint64(max))) - 1;

         if (qAbs(i) > n) {
            return QValidator::Invalid;
         }
      }
   }

   return QValidator::Intermediate;
}

void QDoubleValidator::setRange(double minimum, double maximum, int decimals)
{
   bool rangeChanged = false;
   if (b != minimum) {
      b = minimum;
      rangeChanged = true;
      emit bottomChanged(b);
   }

   if (t != maximum) {
      t = maximum;
      rangeChanged = true;
      emit topChanged(t);
   }

   if (dec != decimals) {
      dec = decimals;
      rangeChanged = true;
      emit decimalsChanged(dec);
   }

   if (rangeChanged) {
      emit changed();
   }
}

void QDoubleValidator::setBottom(double bottom)
{
   setRange(bottom, top(), decimals());
}

void QDoubleValidator::setTop(double top)
{
   setRange(bottom(), top, decimals());
}

void QDoubleValidator::setDecimals(int decimals)
{
   setRange(bottom(), top(), decimals);
}

void QDoubleValidator::setNotation(Notation newNotation)
{
   Q_D(QDoubleValidator);

   if (d->notation != newNotation) {
      d->notation = newNotation;
      emit notationChanged(d->notation);
      emit changed();
   }
}

QDoubleValidator::Notation QDoubleValidator::notation() const
{
   Q_D(const QDoubleValidator);
   return d->notation;
}

QRegularExpressionValidator::QRegularExpressionValidator(QObject *parent)
   : QValidator(parent), m_regexp(".*")
{
}

QRegularExpressionValidator::QRegularExpressionValidator(const QRegularExpression &regExp, QObject *parent)
   : QValidator(parent)
{
   setRegularExpression(regExp);
}

QRegularExpressionValidator::~QRegularExpressionValidator()
{
}

QValidator::State QRegularExpressionValidator::validate(QString &input, int &pos) const
{
   if (m_regexp.pattern().isEmpty()) {
      return Acceptable;
   }

   QRegularExpressionMatch match = m_regexp.match(input, input.begin(), QMatchType::PartialPreferCompleteMatch);

   if (match.hasMatch()) {
      return Acceptable;

   } else if (input.isEmpty() || match.hasPartialMatch()) {
      return Intermediate;

   } else {
      pos = input.size();
      return Invalid;

   }
}

const QRegularExpression &QRegularExpressionValidator::regularExpression() const
{
   return m_regexp;
}

void QRegularExpressionValidator::setRegularExpression(const QRegularExpression &regExp)
{
   if (m_regexp.pattern() != regExp.pattern() ||
      (regExp.patternOptions() | QPatternOption::ExactMatchOption) != m_regexp.patternOptions()  ) {

      m_regexp = regExp;

      QPatternOptionFlags flags = m_regexp.patternOptions();
      flags |= QPatternOption::ExactMatchOption;
      m_regexp.setPatternOptions(flags);

      emit regularExpressionChanged(m_regexp);
   }
}

#endif // QT_NO_REGEXP

#endif // QT_NO_VALIDATOR
