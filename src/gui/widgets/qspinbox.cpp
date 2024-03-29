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

#include <qabstractspinbox_p.h>
#include <qspinbox.h>

#ifndef QT_NO_SPINBOX

#include <qlineedit.h>
#include <qlocale.h>
#include <qvalidator.h>

#include <float.h>

class QSpinBoxPrivate : public QAbstractSpinBoxPrivate
{
   Q_DECLARE_PUBLIC(QSpinBox)

 public:
   QSpinBoxPrivate();
   void emitSignals(EmitPolicy ep, const QVariant &) override;

   QVariant valueFromText(const QString &n) const override;
   QString textFromValue(const QVariant &n) const override;
   QVariant validateAndInterpret(QString &input, int &pos, QValidator::State &state) const;

   void init() {
      Q_Q(QSpinBox);
      q->setInputMethodHints(Qt::ImhDigitsOnly);
      setLayoutItemMargins(QStyle::SE_SpinBoxLayoutItem);
   }

   int displayIntegerBase;
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
}

QSpinBox::~QSpinBox() {}

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
   d->cachedMinimumSizeHint = QSize(); // minimumSizeHint cares about the prefix
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

QString QSpinBox::cleanText() const
{
   Q_D(const QSpinBox);

   return d->stripped(d->edit->displayText());
}

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

int QSpinBox::displayIntegerBase() const
{
   Q_D(const QSpinBox);
   return d->displayIntegerBase;
}

void QSpinBox::setDisplayIntegerBase(int base)
{
   Q_D(QSpinBox);

   if (base < 2 || base > 36) {
      qWarning("QSpinBox::setDisplayIntegerBase() Base (%d) is not a valid integer base", base);
      base = 10;
   }

   if (base != d->displayIntegerBase) {
      d->displayIntegerBase = base;
      d->updateEdit();
   }
}

QString QSpinBox::textFromValue(int value) const
{
   Q_D(const QSpinBox);

   QString str;

   if (d->displayIntegerBase != 10) {
      str = QString::number(qAbs(value), d->displayIntegerBase);
      if (value < 0) {
         str.prepend('-');
      }

   } else {
      str = locale().toString(value);
      if (!d->showGroupSeparator && (qAbs(value) >= 1000 || value == INT_MIN)) {
         str.remove(locale().groupSeparator());
      }
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

QValidator::State QSpinBox::validate(QString &text, int &pos) const
{
   Q_D(const QSpinBox);

   QValidator::State state;
   d->validateAndInterpret(text, pos, state);
   return state;
}

void QSpinBox::fixup(QString &input) const
{
   if (! isGroupSeparatorShown()) {
      input.remove(locale().groupSeparator());
   }
}

QDoubleSpinBox::QDoubleSpinBox(QWidget *parent)
   : QAbstractSpinBox(*new QDoubleSpinBoxPrivate, parent)
{
   Q_D(QDoubleSpinBox);
   d->init();
}

QDoubleSpinBox::~QDoubleSpinBox()
{
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

QString QDoubleSpinBox::cleanText() const
{
   Q_D(const QDoubleSpinBox);

   return d->stripped(d->edit->displayText());
}

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

void QDoubleSpinBox::setRange(double minimum, double maximum)
{
   Q_D(QDoubleSpinBox);
   d->actualMin = minimum;
   d->actualMax = maximum;
   d->setRange(QVariant(d->round(minimum)), QVariant(d->round(maximum)));
}

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

   if (!d->showGroupSeparator && qAbs(value) >= 1000.0) {
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

QValidator::State QDoubleSpinBox::validate(QString &text, int &pos) const
{
   Q_D(const QDoubleSpinBox);

   QValidator::State state;
   d->validateAndInterpret(text, pos, state);
   return state;
}

void QDoubleSpinBox::fixup(QString &input) const
{
   input.remove(locale().groupSeparator());
}

QSpinBoxPrivate::QSpinBoxPrivate()
{
   minimum = QVariant((int)0);
   maximum = QVariant((int)99);
   value = minimum;
   displayIntegerBase = 10;
   singleStep = QVariant((int)1);
   type = QVariant::Int;
}

void QSpinBoxPrivate::emitSignals(EmitPolicy ep, const QVariant &old)
{
   Q_Q(QSpinBox);

   if (ep != NeverEmit) {
      pendingEmit = false;

      if (ep == AlwaysEmit || value != old) {
         emit q->valueChanged(edit->displayText());

         emit q->cs_valueChanged(value.toInt());
         emit q->valueChanged(value.toInt());
      }
   }
}

QString QSpinBoxPrivate::textFromValue(const QVariant &value) const
{
   Q_Q(const QSpinBox);
   return q->textFromValue(value.toInt());
}

QVariant QSpinBoxPrivate::valueFromText(const QString &text) const
{
   Q_Q(const QSpinBox);

   return QVariant(q->valueFromText(text));
}

QVariant QSpinBoxPrivate::validateAndInterpret(QString &input, int &pos, QValidator::State &state) const
{
   if (cachedText == input && !input.isEmpty()) {
      state = cachedState;
      return cachedValue;
   }

   const int max = maximum.toInt();
   const int min = minimum.toInt();

   QString copy = stripped(input, &pos);

   state   = QValidator::Acceptable;
   int num = min;

   if (max != min && (copy.isEmpty() || (min < 0 && copy  == "-") || (max >= 0 && copy == "+"))) {
      state = QValidator::Intermediate;

   } else if (copy.startsWith('-') && min >= 0) {
      // special-case -0 will be interpreted as 0 and thus not be invalid with a range from 0-100
      state = QValidator::Invalid;

   } else {
      bool ok = false;

      if (displayIntegerBase != 10) {
         num = copy.toInteger<int>(&ok, displayIntegerBase);

      } else {
         num = locale.toInt(copy, &ok);

         if (! ok && copy.contains(locale.groupSeparator()) && (max >= 1000 || min <= -1000)) {
            QString copy2 = copy;
            copy2.remove(locale.groupSeparator());
            num = locale.toInt(copy2, &ok, 10);
         }
      }

      if (!ok) {
         state = QValidator::Invalid;

      } else if (num >= min && num <= max) {
         state = QValidator::Acceptable;

      } else if (max == min) {
         state = QValidator::Invalid;

      } else {
         if ((num >= 0 && num > max) || (num < 0 && num < min)) {
            state = QValidator::Invalid;

         } else {
            state = QValidator::Intermediate;

         }
      }
   }

   if (state != QValidator::Acceptable) {
      num = max > 0 ? min : max;
   }

   input = prefix + copy + suffix;
   cachedText  = input;
   cachedState = state;
   cachedValue = QVariant((int)num);

   return cachedValue;
}

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

void QDoubleSpinBoxPrivate::emitSignals(EmitPolicy ep, const QVariant &old)
{
   Q_Q(QDoubleSpinBox);

   if (ep != NeverEmit) {
      pendingEmit = false;

      if (ep == AlwaysEmit || value != old) {
         emit q->valueChanged(edit->displayText());

         emit q->cs_valueChanged(value.toDouble());
         emit q->valueChanged(value.toDouble());
      }
   }
}

QVariant QDoubleSpinBoxPrivate::valueFromText(const QString &f) const
{
   Q_Q(const QDoubleSpinBox);
   return QVariant(q->valueFromText(f));
}

double QDoubleSpinBoxPrivate::round(double value) const
{
   QString tmp = QString::number(value, 'f', decimals);
   return tmp.toDouble();
}

QVariant QDoubleSpinBoxPrivate::validateAndInterpret(QString &input, int &pos,
   QValidator::State &state) const
{
   if (cachedText == input && !input.isEmpty()) {
      state = cachedState;

      return cachedValue;
   }
   const double max = maximum.toDouble();
   const double min = minimum.toDouble();

   QString copy = stripped(input, &pos);

   int len    = copy.size();
   double num = min;

   const bool plus  = max >= 0;
   const bool minus = min <= 0;

   switch (len) {
      case 0:
         state = max != min ? QValidator::Intermediate : QValidator::Invalid;
         goto end;

      case 1:
         if (copy.at(0) == locale.decimalPoint() || (plus && copy.at(0) == '+') || (minus && copy.at(0) == '-')) {
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
      state = QValidator::Invalid;
      goto end;

   } else if (len > 1) {
      const int dec = copy.indexOf(locale.decimalPoint());

      if (dec != -1) {

         // typing a delimiter when on the delimiter should be treated as typing right arrow
         if (dec + 1 < copy.size() && copy.at(dec + 1) == locale.decimalPoint() && pos == dec + 1) {
            copy.remove(dec + 1, 1);
         }

         if (copy.size() - dec > decimals + 1) {
            state = QValidator::Invalid;
            goto end;
         }

         for (int i = dec + 1; i < copy.size(); ++i) {
            if (copy.at(i).isSpace() || copy.at(i) == locale.groupSeparator()) {

               state = QValidator::Invalid;
               goto end;
            }
         }
      } else {
         const QChar last = copy.at(len - 1);
         const QChar secondLast = copy.at(len - 2);

         if ((last == locale.groupSeparator() || last.isSpace())
               && (secondLast == locale.groupSeparator() || secondLast.isSpace())) {
            state = QValidator::Invalid;

            goto end;

         } else if (last.isSpace() && (!locale.groupSeparator().isSpace() || secondLast.isSpace())) {
            state = QValidator::Invalid;
            goto end;
         }
      }
   }

   {
      bool ok = false;
      num = locale.toDouble(copy, &ok);

      if (!ok) {
         if (locale.groupSeparator().isPrint()) {
            if (max < 1000 && min > -1000 && copy.contains(locale.groupSeparator())) {
               state = QValidator::Invalid;
               goto end;
            }

            const int len = copy.size();
            for (int i = 0; i < len - 1; ++i) {
               if (copy.at(i) == locale.groupSeparator() && copy.at(i + 1) == locale.groupSeparator()) {
                  state = QValidator::Invalid;
                  goto end;
               }
            }

            QString copy2 = copy;
            copy2.remove(locale.groupSeparator());
            num = locale.toDouble(copy2, &ok);

            if (!ok) {
               state = QValidator::Invalid;
               goto end;
            }
         }
      }

      if (!ok) {
         state = QValidator::Invalid;

      } else if (num >= min && num <= max) {
         state = QValidator::Acceptable;

      } else if (max == min) { // when max and min is the same the only non-Invalid input is max (or min)
         state = QValidator::Invalid;

      } else {
         if ((num >= 0 && num > max) || (num < 0 && num < min)) {
            state = QValidator::Invalid;

         } else {
            state = QValidator::Intermediate;

         }
      }
   }

end:
   if (state != QValidator::Acceptable) {
      num = max > 0 ? min : max;
   }

   input       = prefix + copy + suffix;
   cachedText  = input;
   cachedState = state;
   cachedValue = QVariant(num);

   return QVariant(num);
}

QString QDoubleSpinBoxPrivate::textFromValue(const QVariant &f) const
{
   Q_Q(const QDoubleSpinBox);
   return q->textFromValue(f.toDouble());
}

bool QSpinBox::event(QEvent *event)
{
   Q_D(QSpinBox);

#ifdef Q_OS_DARWIN
   if (event->type() == QEvent::StyleChange || event->type() == QEvent::MacSizeChange) {

#else
   if (event->type() == QEvent::StyleChange) {

#endif
      d->setLayoutItemMargins(QStyle::SE_SpinBoxLayoutItem);
   }

   return QAbstractSpinBox::event(event);
}

#endif // QT_NO_SPINBOX
