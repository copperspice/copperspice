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

#include <qabstractspinbox_p.h>
#include <qspinbox.h>

#ifndef QT_NO_SPINBOX

#include <qlineedit.h>
#include <qlocale.h>
#include <qvalidator.h>
#include <qdebug.h>

#include <math.h>
#include <float.h>

QT_BEGIN_NAMESPACE

//#define QSPINBOX_QSBDEBUG
#ifdef QSPINBOX_QSBDEBUG
#  define QSBDEBUG qDebug
#else
#  define QSBDEBUG if (false) qDebug
#endif

class QSpinBoxPrivate : public QAbstractSpinBoxPrivate
{
   Q_DECLARE_PUBLIC(QSpinBox)

 public:
   QSpinBoxPrivate();
   void emitSignals(EmitPolicy ep, const QVariant &) override;

   QVariant valueFromText(const QString &n) const override;
   QString textFromValue(const QVariant &n) const override;
   QVariant validateAndInterpret(QString &input, int &pos, QValidator::State &state) const;

   inline void init() {
      Q_Q(QSpinBox);
      q->setInputMethodHints(Qt::ImhDigitsOnly);
      setLayoutItemMargins(QStyle::SE_SpinBoxLayoutItem);
   }
};

class QDoubleSpinBoxPrivate : public QAbstractSpinBoxPrivate
{
   Q_DECLARE_PUBLIC(QDoubleSpinBox)

 public:
   QDoubleSpinBoxPrivate();

   void emitSignals(EmitPolicy ep, const QVariant &) override;

   QVariant valueFromText(const QString &n) const override;
   QString textFromValue(const QVariant &n) const override;
   QVariant validateAndInterpret(QString &input, int &pos, QValidator::State &state) const;
   double round(double input) const;

   // variables
   int decimals;

   inline void init() {
      Q_Q(QDoubleSpinBox);
      q->setInputMethodHints(Qt::ImhFormattedNumbersOnly);
   }

   // When fiddling with the decimals property, we may lose precision in these properties.
   double actualMin;
   double actualMax;
};

QSpinBox::QSpinBox(QWidget *parent)
   : QAbstractSpinBox(*new QSpinBoxPrivate, parent)
{
   Q_D(QSpinBox);
   d->init();

   connect(this, static_cast<void (QSpinBox::*)(int)> (&QSpinBox::valueChanged), this, &QSpinBox::cs_valueChanged);
}

int QSpinBox::value() const
{
   Q_D(const QSpinBox);
   return d->value.toInt();
}

void QSpinBox::setValue(int value)
{
   Q_D(QSpinBox);
   d->setValue(QVariant(value), EmitIfChanged);
}


QString QSpinBox::prefix() const
{
   Q_D(const QSpinBox);
   return d->prefix;
}

void QSpinBox::setPrefix(const QString &prefix)
{
   Q_D(QSpinBox);

   d->prefix = prefix;
   d->updateEdit();

   d->cachedSizeHint = QSize();
   updateGeometry();
}



QString QSpinBox::suffix() const
{
   Q_D(const QSpinBox);

   return d->suffix;
}

void QSpinBox::setSuffix(const QString &suffix)
{
   Q_D(QSpinBox);

   d->suffix = suffix;
   d->updateEdit();

   d->cachedSizeHint = QSize();
   updateGeometry();
}

/*!
    \property QSpinBox::cleanText

    \brief the text of the spin box excluding any prefix, suffix,
    or leading or trailing whitespace.

    \sa text, QSpinBox::prefix, QSpinBox::suffix
*/

QString QSpinBox::cleanText() const
{
   Q_D(const QSpinBox);

   return d->stripped(d->edit->displayText());
}


/*!
    \property QSpinBox::singleStep
    \brief the step value

    When the user uses the arrows to change the spin box's value the
    value will be incremented/decremented by the amount of the
    singleStep. The default value is 1. Setting a singleStep value of
    less than 0 does nothing.
*/

int QSpinBox::singleStep() const
{
   Q_D(const QSpinBox);

   return d->singleStep.toInt();
}

void QSpinBox::setSingleStep(int value)
{
   Q_D(QSpinBox);
   if (value >= 0) {
      d->singleStep = QVariant(value);
      d->updateEdit();
   }
}

/*!
    \property QSpinBox::minimum

    \brief the minimum value of the spin box

    When setting this property the \l maximum is adjusted
    if necessary to ensure that the range remains valid.

    The default minimum value is 0.

    \sa setRange()  specialValueText
*/

int QSpinBox::minimum() const
{
   Q_D(const QSpinBox);

   return d->minimum.toInt();
}

void QSpinBox::setMinimum(int minimum)
{
   Q_D(QSpinBox);
   const QVariant m(minimum);
   d->setRange(m, (d->variantCompare(d->maximum, m) > 0 ? d->maximum : m));
}

/*!
    \property QSpinBox::maximum

    \brief the maximum value of the spin box

    When setting this property the \l minimum is adjusted
    if necessary, to ensure that the range remains valid.

    The default maximum value is 99.

    \sa setRange() specialValueText

*/

int QSpinBox::maximum() const
{
   Q_D(const QSpinBox);

   return d->maximum.toInt();
}

void QSpinBox::setMaximum(int maximum)
{
   Q_D(QSpinBox);
   const QVariant m(maximum);
   d->setRange((d->variantCompare(d->minimum, m) < 0 ? d->minimum : m), m);
}

void QSpinBox::setRange(int minimum, int maximum)
{
   Q_D(QSpinBox);
   d->setRange(QVariant(minimum), QVariant(maximum));
}

QString QSpinBox::textFromValue(int value) const
{
   QString str = locale().toString(value);
   if (qAbs(value) >= 1000 || value == INT_MIN) {
      str.remove(locale().groupSeparator());
   }

   return str;
}

int QSpinBox::valueFromText(const QString &text) const
{
   Q_D(const QSpinBox);

   QString copy = text;
   int pos = d->edit->cursorPosition();
   QValidator::State state = QValidator::Acceptable;
   return d->validateAndInterpret(copy, pos, state).toInt();
}

/*!
  \reimp
*/
QValidator::State QSpinBox::validate(QString &text, int &pos) const
{
   Q_D(const QSpinBox);

   QValidator::State state;
   d->validateAndInterpret(text, pos, state);
   return state;
}


/*!
  \reimp
*/
void QSpinBox::fixup(QString &input) const
{
   input.remove(locale().groupSeparator());
}

QDoubleSpinBox::QDoubleSpinBox(QWidget *parent)
   : QAbstractSpinBox(*new QDoubleSpinBoxPrivate, parent)
{
   Q_D(QDoubleSpinBox);
   d->init();

   connect(this, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged),
           this, &QDoubleSpinBox::cs_valueChanged);
}

double QDoubleSpinBox::value() const
{
   Q_D(const QDoubleSpinBox);

   return d->value.toDouble();
}

void QDoubleSpinBox::setValue(double value)
{
   Q_D(QDoubleSpinBox);
   QVariant v(d->round(value));
   d->setValue(v, EmitIfChanged);
}
/*!
    \property QDoubleSpinBox::prefix
    \brief the spin box's prefix

    The prefix is prepended to the start of the displayed value.
    Typical use is to display a unit of measurement or a currency
    symbol. For example:

    \snippet doc/src/snippets/code/src_gui_widgets_qspinbox.cpp 4

    To turn off the prefix display, set this property to an empty
    string. The default is no prefix. The prefix is not displayed when
    value() == minimum() and specialValueText() is set.

    If no prefix is set, prefix() returns an empty string.

    \sa suffix(), setSuffix(), specialValueText(), setSpecialValueText()
*/

QString QDoubleSpinBox::prefix() const
{
   Q_D(const QDoubleSpinBox);

   return d->prefix;
}

void QDoubleSpinBox::setPrefix(const QString &prefix)
{
   Q_D(QDoubleSpinBox);

   d->prefix = prefix;
   d->updateEdit();
}

/*!
    \property QDoubleSpinBox::suffix
    \brief the suffix of the spin box

    The suffix is appended to the end of the displayed value. Typical
    use is to display a unit of measurement or a currency symbol. For
    example:

    \snippet doc/src/snippets/code/src_gui_widgets_qspinbox.cpp 5

    To turn off the suffix display, set this property to an empty
    string. The default is no suffix. The suffix is not displayed for
    the minimum() if specialValueText() is set.

    If no suffix is set, suffix() returns an empty string.

    \sa prefix(), setPrefix(), specialValueText(), setSpecialValueText()
*/

QString QDoubleSpinBox::suffix() const
{
   Q_D(const QDoubleSpinBox);

   return d->suffix;
}

void QDoubleSpinBox::setSuffix(const QString &suffix)
{
   Q_D(QDoubleSpinBox);

   d->suffix = suffix;
   d->updateEdit();

   d->cachedSizeHint = QSize();
   updateGeometry();
}

/*!
    \property QDoubleSpinBox::cleanText

    \brief the text of the spin box excluding any prefix, suffix,
    or leading or trailing whitespace.

    \sa text, QDoubleSpinBox::prefix, QDoubleSpinBox::suffix
*/

QString QDoubleSpinBox::cleanText() const
{
   Q_D(const QDoubleSpinBox);

   return d->stripped(d->edit->displayText());
}

/*!
    \property QDoubleSpinBox::singleStep
    \brief the step value

    When the user uses the arrows to change the spin box's value the
    value will be incremented/decremented by the amount of the
    singleStep. The default value is 1.0. Setting a singleStep value
    of less than 0 does nothing.
*/
double QDoubleSpinBox::singleStep() const
{
   Q_D(const QDoubleSpinBox);

   return d->singleStep.toDouble();
}

void QDoubleSpinBox::setSingleStep(double value)
{
   Q_D(QDoubleSpinBox);

   if (value >= 0) {
      d->singleStep = value;
      d->updateEdit();
   }
}

/*!
    \property QDoubleSpinBox::minimum

    \brief the minimum value of the spin box

    When setting this property the \l maximum is adjusted
    if necessary to ensure that the range remains valid.

    The default minimum value is 0.0.

    Note: The minimum value will be rounded to match the decimals
    property.

    \sa decimals, setRange() specialValueText
*/

double QDoubleSpinBox::minimum() const
{
   Q_D(const QDoubleSpinBox);

   return d->minimum.toDouble();
}

void QDoubleSpinBox::setMinimum(double minimum)
{
   Q_D(QDoubleSpinBox);
   d->actualMin = minimum;
   const QVariant m(d->round(minimum));
   d->setRange(m, (d->variantCompare(d->maximum, m) > 0 ? d->maximum : m));
}

/*!
    \property QDoubleSpinBox::maximum

    \brief the maximum value of the spin box

    When setting this property the \l minimum is adjusted
    if necessary, to ensure that the range remains valid.

    The default maximum value is 99.99.

    Note: The maximum value will be rounded to match the decimals
    property.

    \sa decimals, setRange()
*/

double QDoubleSpinBox::maximum() const
{
   Q_D(const QDoubleSpinBox);

   return d->maximum.toDouble();
}

void QDoubleSpinBox::setMaximum(double maximum)
{
   Q_D(QDoubleSpinBox);
   d->actualMax = maximum;
   const QVariant m(d->round(maximum));
   d->setRange((d->variantCompare(d->minimum, m) < 0 ? d->minimum : m), m);
}

/*!
    Convenience function to set the \a minimum and \a maximum values
    with a single function call.

    Note: The maximum and minimum values will be rounded to match the
    decimals property.

    \snippet doc/src/snippets/code/src_gui_widgets_qspinbox.cpp 6
    is equivalent to:
    \snippet doc/src/snippets/code/src_gui_widgets_qspinbox.cpp 7

    \sa minimum maximum
*/

void QDoubleSpinBox::setRange(double minimum, double maximum)
{
   Q_D(QDoubleSpinBox);
   d->actualMin = minimum;
   d->actualMax = maximum;
   d->setRange(QVariant(d->round(minimum)), QVariant(d->round(maximum)));
}

/*!
     \property QDoubleSpinBox::decimals

     \brief the precision of the spin box, in decimals

     Sets how many decimals the spinbox will use for displaying and
     interpreting doubles.

     \warning The maximum value for \a decimals is DBL_MAX_10_EXP +
     DBL_DIG (ie. 323) because of the limitations of the double type.

     Note: The maximum, minimum and value might change as a result of
     changing this property.
*/

int QDoubleSpinBox::decimals() const
{
   Q_D(const QDoubleSpinBox);

   return d->decimals;
}

void QDoubleSpinBox::setDecimals(int decimals)
{
   Q_D(QDoubleSpinBox);
   d->decimals = qBound(0, decimals, DBL_MAX_10_EXP + DBL_DIG);

   setRange(d->actualMin, d->actualMax); // make sure values are rounded
   setValue(value());
}

QString QDoubleSpinBox::textFromValue(double value) const
{
   Q_D(const QDoubleSpinBox);
   QString str = locale().toString(value, 'f', d->decimals);

   if (qAbs(value) >= 1000.0) {
      str.remove(locale().groupSeparator());
   }

   return str;
}

double QDoubleSpinBox::valueFromText(const QString &text) const
{
   Q_D(const QDoubleSpinBox);

   QString copy = text;
   int pos = d->edit->cursorPosition();
   QValidator::State state = QValidator::Acceptable;
   return d->validateAndInterpret(copy, pos, state).toDouble();
}

/*!
  \reimp
*/
QValidator::State QDoubleSpinBox::validate(QString &text, int &pos) const
{
   Q_D(const QDoubleSpinBox);

   QValidator::State state;
   d->validateAndInterpret(text, pos, state);
   return state;
}


/*!
  \reimp
*/
void QDoubleSpinBox::fixup(QString &input) const
{
   input.remove(locale().groupSeparator());
}

// --- QSpinBoxPrivate ---

/*!
    \internal
    Constructs a QSpinBoxPrivate object
*/

QSpinBoxPrivate::QSpinBoxPrivate()
{
   minimum = QVariant((int)0);
   maximum = QVariant((int)99);
   value = minimum;
   singleStep = QVariant((int)1);
   type = QVariant::Int;
}

/*!
    \internal
    \reimp
*/

void QSpinBoxPrivate::emitSignals(EmitPolicy ep, const QVariant &old)
{
   Q_Q(QSpinBox);
   if (ep != NeverEmit) {
      pendingEmit = false;
      if (ep == AlwaysEmit || value != old) {
         emit q->valueChanged(edit->displayText());
         emit q->valueChanged(value.toInt());
      }
   }
}

/*!
    \internal
    \reimp
*/

QString QSpinBoxPrivate::textFromValue(const QVariant &value) const
{
   Q_Q(const QSpinBox);
   return q->textFromValue(value.toInt());
}
/*!
    \internal
    \reimp
*/

QVariant QSpinBoxPrivate::valueFromText(const QString &text) const
{
   Q_Q(const QSpinBox);

   return QVariant(q->valueFromText(text));
}


/*!
    \internal Multi purpose function that parses input, sets state to
    the appropriate state and returns the value it will be interpreted
    as.
*/

QVariant QSpinBoxPrivate::validateAndInterpret(QString &input, int &pos,
      QValidator::State &state) const
{
   if (cachedText == input && !input.isEmpty()) {
      state = cachedState;
      QSBDEBUG() << "cachedText was '" << cachedText << "' state was "
                 << state << " and value was " << cachedValue;

      return cachedValue;
   }
   const int max = maximum.toInt();
   const int min = minimum.toInt();

   QString copy = stripped(input, &pos);
   QSBDEBUG() << "input" << input << "copy" << copy;
   state = QValidator::Acceptable;
   int num = min;

   if (max != min && (copy.isEmpty()
                      || (min < 0 && copy == QLatin1String("-"))
                      || (min >= 0 && copy == QLatin1String("+")))) {
      state = QValidator::Intermediate;
      QSBDEBUG() << __FILE__ << __LINE__ << "num is set to" << num;
   } else if (copy.startsWith(QLatin1Char('-')) && min >= 0) {
      state = QValidator::Invalid; // special-case -0 will be interpreted as 0 and thus not be invalid with a range from 0-100
   } else {
      bool ok = false;
      num = locale.toInt(copy, &ok, 10);
      if (!ok && copy.contains(locale.groupSeparator()) && (max >= 1000 || min <= -1000)) {
         QString copy2 = copy;
         copy2.remove(locale.groupSeparator());
         num = locale.toInt(copy2, &ok, 10);
      }
      QSBDEBUG() << __FILE__ << __LINE__ << "num is set to" << num;
      if (!ok) {
         state = QValidator::Invalid;
      } else if (num >= min && num <= max) {
         state = QValidator::Acceptable;
      } else if (max == min) {
         state = QValidator::Invalid;
      } else {
         if ((num >= 0 && num > max) || (num < 0 && num < min)) {
            state = QValidator::Invalid;
            QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
         } else {
            state = QValidator::Intermediate;
            QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Intermediate";
         }
      }
   }
   if (state != QValidator::Acceptable) {
      num = max > 0 ? min : max;
   }
   input = prefix + copy + suffix;
   cachedText = input;
   cachedState = state;
   cachedValue = QVariant((int)num);

   QSBDEBUG() << "cachedText is set to '" << cachedText << "' state is set to "
              << state << " and value is set to " << cachedValue;
   return cachedValue;
}

// --- QDoubleSpinBoxPrivate ---

/*!
    \internal
    Constructs a QSpinBoxPrivate object
*/

QDoubleSpinBoxPrivate::QDoubleSpinBoxPrivate()
{
   actualMin = 0.0;
   actualMax = 99.99;
   minimum = QVariant(actualMin);
   maximum = QVariant(actualMax);
   value = minimum;
   singleStep = QVariant(1.0);
   decimals = 2;
   type = QVariant::Double;
}

/*!
    \internal
    \reimp
*/

void QDoubleSpinBoxPrivate::emitSignals(EmitPolicy ep, const QVariant &old)
{
   Q_Q(QDoubleSpinBox);
   if (ep != NeverEmit) {
      pendingEmit = false;
      if (ep == AlwaysEmit || value != old) {
         emit q->valueChanged(edit->displayText());
         emit q->valueChanged(value.toDouble());
      }
   }
}


/*!
    \internal
    \reimp
*/
QVariant QDoubleSpinBoxPrivate::valueFromText(const QString &f) const
{
   Q_Q(const QDoubleSpinBox);
   return QVariant(q->valueFromText(f));
}

/*!
    \internal
    Rounds to a double value that is restricted to decimals.
    E.g. // decimals = 2

    round(5.555) => 5.56
    */

double QDoubleSpinBoxPrivate::round(double value) const
{
   QString tmp = QString::number(value, 'f', decimals);
   return tmp.toDouble();
}


/*!
    \internal Multi purpose function that parses input, sets state to
    the appropriate state and returns the value it will be interpreted
    as.
*/

QVariant QDoubleSpinBoxPrivate::validateAndInterpret(QString &input, int &pos,
      QValidator::State &state) const
{
   if (cachedText == input && !input.isEmpty()) {
      state = cachedState;
      QSBDEBUG() << "cachedText was '" << cachedText << "' state was "
                 << state << " and value was " << cachedValue;
      return cachedValue;
   }
   const double max = maximum.toDouble();
   const double min = minimum.toDouble();

   QString copy = stripped(input, &pos);
   QSBDEBUG() << "input" << input << "copy" << copy;
   int len = copy.size();
   double num = min;
   const bool plus = max >= 0;
   const bool minus = min <= 0;

   switch (len) {
      case 0:
         state = max != min ? QValidator::Intermediate : QValidator::Invalid;
         goto end;
      case 1:
         if (copy.at(0) == locale.decimalPoint()
               || (plus && copy.at(0) == QLatin1Char('+'))
               || (minus && copy.at(0) == QLatin1Char('-'))) {
            state = QValidator::Intermediate;
            goto end;
         }
         break;
      case 2:
         if (copy.at(1) == locale.decimalPoint()
               && ((plus && copy.at(0) == QLatin1Char('+')) || (minus && copy.at(0) == QLatin1Char('-')))) {
            state = QValidator::Intermediate;
            goto end;
         }
         break;
      default:
         break;
   }

   if (copy.at(0) == locale.groupSeparator()) {
      QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
      state = QValidator::Invalid;
      goto end;
   } else if (len > 1) {
      const int dec = copy.indexOf(locale.decimalPoint());
      if (dec != -1) {
         if (dec + 1 < copy.size() && copy.at(dec + 1) == locale.decimalPoint() && pos == dec + 1) {
            copy.remove(dec + 1, 1); // typing a delimiter when you are on the delimiter
         } // should be treated as typing right arrow

         if (copy.size() - dec > decimals + 1) {
            QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
            state = QValidator::Invalid;
            goto end;
         }
         for (int i = dec + 1; i < copy.size(); ++i) {
            if (copy.at(i).isSpace() || copy.at(i) == locale.groupSeparator()) {
               QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
               state = QValidator::Invalid;
               goto end;
            }
         }
      } else {
         const QChar &last = copy.at(len - 1);
         const QChar &secondLast = copy.at(len - 2);
         if ((last == locale.groupSeparator() || last.isSpace())
               && (secondLast == locale.groupSeparator() || secondLast.isSpace())) {
            state = QValidator::Invalid;
            QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
            goto end;
         } else if (last.isSpace() && (!locale.groupSeparator().isSpace() || secondLast.isSpace())) {
            state = QValidator::Invalid;
            QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
            goto end;
         }
      }
   }

   {
      bool ok = false;
      num = locale.toDouble(copy, &ok);
      QSBDEBUG() << __FILE__ << __LINE__ << locale << copy << num << ok;

      if (!ok) {
         if (locale.groupSeparator().isPrint()) {
            if (max < 1000 && min > -1000 && copy.contains(locale.groupSeparator())) {
               state = QValidator::Invalid;
               QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
               goto end;
            }

            const int len = copy.size();
            for (int i = 0; i < len - 1; ++i) {
               if (copy.at(i) == locale.groupSeparator() && copy.at(i + 1) == locale.groupSeparator()) {
                  QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
                  state = QValidator::Invalid;
                  goto end;
               }
            }

            QString copy2 = copy;
            copy2.remove(locale.groupSeparator());
            num = locale.toDouble(copy2, &ok);
            QSBDEBUG() << locale.groupSeparator() << num << copy2 << ok;

            if (!ok) {
               state = QValidator::Invalid;
               QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
               goto end;
            }
         }
      }

      if (!ok) {
         state = QValidator::Invalid;
         QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
      } else if (num >= min && num <= max) {
         state = QValidator::Acceptable;
         QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Acceptable";
      } else if (max == min) { // when max and min is the same the only non-Invalid input is max (or min)
         state = QValidator::Invalid;
         QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
      } else {
         if ((num >= 0 && num > max) || (num < 0 && num < min)) {
            state = QValidator::Invalid;
            QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Invalid";
         } else {
            state = QValidator::Intermediate;
            QSBDEBUG() << __FILE__ << __LINE__ << "state is set to Intermediate";
         }
      }
   }

end:
   if (state != QValidator::Acceptable) {
      num = max > 0 ? min : max;
   }

   input = prefix + copy + suffix;
   cachedText = input;
   cachedState = state;
   cachedValue = QVariant(num);
   return QVariant(num);
}

/*
    \internal
    \reimp
*/

QString QDoubleSpinBoxPrivate::textFromValue(const QVariant &f) const
{
   Q_Q(const QDoubleSpinBox);
   return q->textFromValue(f.toDouble());
}

/*! \reimp */
bool QSpinBox::event(QEvent *event)
{
   Q_D(QSpinBox);

   if (event->type() == QEvent::StyleChange

#ifdef Q_OS_MAC
         || event->type() == QEvent::MacSizeChange
#endif
      ) {
      d->setLayoutItemMargins(QStyle::SE_SpinBoxLayoutItem);
   }
   return QAbstractSpinBox::event(event);
}

QT_END_NAMESPACE

#endif // QT_NO_SPINBOX
