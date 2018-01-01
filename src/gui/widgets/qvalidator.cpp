/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qdebug.h>
#include <qvalidator.h>

#ifndef QT_NO_VALIDATOR

#include <qlocale_p.h>
#include <limits.h>
#include <math.h>

QT_BEGIN_NAMESPACE

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
   // nothing
}

static int numDigits(qint64 n)
{
   if (n == 0) {
      return 1;
   }
   return (int)log10(double(n)) + 1;
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
   if (!locale().d()->validateChars(input, QLocalePrivate::IntegerMode, &buff)) {
      QLocale cl(QLocale::C);
      if (!cl.d()->validateChars(input, QLocalePrivate::IntegerMode, &buff)) {
         return Invalid;
      }
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

   bool ok, overflow;
   qint64 entered = QLocalePrivate::bytearrayToLongLong(buff.constData(), 10, &ok, &overflow);
   if (overflow || !ok) {
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

/*! \reimp */
void QIntValidator::fixup(QString &input) const
{
   QByteArray buff;
   if (!locale().d()->validateChars(input, QLocalePrivate::IntegerMode, &buff)) {
      QLocale cl(QLocale::C);
      if (!cl.d()->validateChars(input, QLocalePrivate::IntegerMode, &buff)) {
         return;
      }
   }
   bool ok, overflow;
   qint64 entered = QLocalePrivate::bytearrayToLongLong(buff.constData(), 10, &ok, &overflow);
   if (ok && !overflow) {
      input = locale().toString(entered);
   }
}

/*!
    Sets the range of the validator to only accept integers between \a
    bottom and \a top inclusive.
*/

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
      : QValidatorPrivate()
      , notation(QDoubleValidator::ScientificNotation) {
   }

   QDoubleValidator::Notation notation;

   QValidator::State validateWithLocale(QString &input, QLocalePrivate::NumberMode numMode, const QLocale &locale) const;
};

QDoubleValidator::QDoubleValidator(QObject *parent)
   : QValidator(*new QDoubleValidatorPrivate , parent)
{
   b = -HUGE_VAL;
   t = HUGE_VAL;
   dec = 1000;
}


QDoubleValidator::QDoubleValidator(double bottom, double top, int decimals, QObject *parent)
   : QValidator(*new QDoubleValidatorPrivate , parent)
{
   b = bottom;
   t = top;
   dec = decimals;
}

QDoubleValidator::~QDoubleValidator()
{
}

#ifndef LLONG_MAX
#   define LLONG_MAX Q_INT64_C(0x7fffffffffffffff)
#endif

QValidator::State QDoubleValidator::validate(QString &input, int &) const
{
   Q_D(const QDoubleValidator);

   QLocalePrivate::NumberMode numMode = QLocalePrivate::DoubleStandardMode;
   switch (d->notation) {
      case StandardNotation:
         numMode = QLocalePrivate::DoubleStandardMode;
         break;

      case ScientificNotation:
         numMode = QLocalePrivate::DoubleScientificMode;
         break;
   }

   State currentLocaleValidation = d->validateWithLocale(input, numMode, locale());

   if (currentLocaleValidation == Acceptable || locale().language() == QLocale::C) {
      return currentLocaleValidation;
   }

   State cLocaleValidation = d->validateWithLocale(input, numMode, QLocale(QLocale::C));
   return qMax(currentLocaleValidation, cLocaleValidation);
}

QValidator::State QDoubleValidatorPrivate::validateWithLocale(QString &input, QLocalePrivate::NumberMode numMode,
      const QLocale &locale) const
{
   Q_Q(const QDoubleValidator);

   QByteArray buff;
   if (! locale.d()->validateChars(input, numMode, &buff, q->dec)) {
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

   bool ok, overflow;
   double i = QLocalePrivate::bytearrayToDouble(buff.constData(), &ok, &overflow);
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
      if (max < LLONG_MAX) {
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

/*!
    \property QDoubleValidator::bottom
    \brief the validator's minimum acceptable value

    By default, this property contains a value of -infinity.

    \sa setRange()
*/

void QDoubleValidator::setBottom(double bottom)
{
   setRange(bottom, top(), decimals());
}


/*!
    \property QDoubleValidator::top
    \brief the validator's maximum acceptable value

    By default, this property contains a value of infinity.

    \sa setRange()
*/

void QDoubleValidator::setTop(double top)
{
   setRange(bottom(), top, decimals());
}

/*!
    \property QDoubleValidator::decimals
    \brief the validator's maximum number of digits after the decimal point

    By default, this property contains a value of 1000.

    \sa setRange()
*/

void QDoubleValidator::setDecimals(int decimals)
{
   setRange(bottom(), top(), decimals);
}

/*!
    \property QDoubleValidator::notation
    \since 4.3
    \brief the notation of how a string can describe a number

    By default, this property is set to ScientificNotation.

    \sa Notation
*/

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

QRegExpValidator::QRegExpValidator(QObject *parent)
   : QValidator(parent), r(QString::fromLatin1(".*"))
{
}

QRegExpValidator::QRegExpValidator(const QRegExp &rx, QObject *parent)
   : QValidator(parent), r(rx)
{
}

QRegExpValidator::~QRegExpValidator()
{
}

QValidator::State QRegExpValidator::validate(QString &input, int &pos) const
{
   if (r.exactMatch(input)) {
      return Acceptable;
   } else {
      if (const_cast<QRegExp &>(r).matchedLength() == input.size()) {
         return Intermediate;
      } else {
         pos = input.size();
         return Invalid;
      }
   }
}


void QRegExpValidator::setRegExp(const QRegExp &rx)
{
   if (r != rx) {
      r = rx;
      emit regExpChanged(r);
      emit changed();
   }
}

#endif

#ifndef QT_NO_REGEXP

class QRegularExpressionValidatorPrivate : public QValidatorPrivate
{
   Q_DECLARE_PUBLIC(QRegularExpressionValidator)

 public:
   QRegExp origRe; // the one set by the user
   QRegExp usedRe; // the one actually used
   void setRegularExpression(const QRegExp &re);
};

QRegularExpressionValidator::QRegularExpressionValidator(QObject *parent)
   : QValidator(*new QRegularExpressionValidatorPrivate, parent)
{
   // origRe in the private will be an empty QRegularExpression,
   // and therefore this validator will match any string.
}


QRegularExpressionValidator::QRegularExpressionValidator(const QRegExp &re, QObject *parent)
   : QValidator(*new QRegularExpressionValidatorPrivate, parent)
{
   Q_D(QRegularExpressionValidator);
   d->setRegularExpression(re);
}


QRegularExpressionValidator::~QRegularExpressionValidator()
{
}

QValidator::State QRegularExpressionValidator::validate(QString &input, int &pos) const
{
   Q_D(const QRegularExpressionValidator);

   // We want a validator with an empty QRegularExpression to match anything;
   // since we're going to do an exact match (by using d->usedRe), first check if the rx is empty
   // (and, if so, accept the input).
   if (d->origRe.pattern().isEmpty()) {
      return Acceptable;
   }

   if (d->usedRe.exactMatch(input)) {
      return Acceptable;
   } else if (d->usedRe.matchedLength() > 0) {
      return Intermediate;
   } else {
      pos = input.size();
      return Invalid;
   }
}

QRegExp QRegularExpressionValidator::regularExpression() const
{
   Q_D(const QRegularExpressionValidator);
   return d->origRe;
}

void QRegularExpressionValidator::setRegularExpression(const QRegExp &re)
{
   Q_D(QRegularExpressionValidator);
   d->setRegularExpression(re);
}

void QRegularExpressionValidatorPrivate::setRegularExpression(const QRegExp &re)
{
   Q_Q(QRegularExpressionValidator);

   if (origRe != re) {
      usedRe = origRe = re; // copies also the pattern options
      usedRe.setPattern(QString("\\A(?:") + re.pattern() + ")\\z");
      emit q->regularExpressionChanged(re);
   }
}

#endif // QT_NO_REGEXP

QT_END_NAMESPACE

#endif // QT_NO_VALIDATOR
