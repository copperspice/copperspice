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

#ifndef QSPINBOX_H
#define QSPINBOX_H

#include <qabstractspinbox.h>

#ifndef QT_NO_SPINBOX

class QSpinBoxPrivate;
class QDoubleSpinBoxPrivate;

class Q_GUI_EXPORT QSpinBox : public QAbstractSpinBox
{
   GUI_CS_OBJECT(QSpinBox)

   GUI_CS_PROPERTY_READ(suffix, suffix)
   GUI_CS_PROPERTY_WRITE(suffix, setSuffix)

   GUI_CS_PROPERTY_READ(prefix, prefix)
   GUI_CS_PROPERTY_WRITE(prefix, setPrefix)

   GUI_CS_PROPERTY_READ(cleanText, cleanText)

   GUI_CS_PROPERTY_READ(minimum, minimum)
   GUI_CS_PROPERTY_WRITE(minimum, setMinimum)

   GUI_CS_PROPERTY_READ(maximum, maximum)
   GUI_CS_PROPERTY_WRITE(maximum, setMaximum)

   GUI_CS_PROPERTY_READ(singleStep, singleStep)
   GUI_CS_PROPERTY_WRITE(singleStep, setSingleStep)

   GUI_CS_PROPERTY_READ(value, value)
   GUI_CS_PROPERTY_WRITE(value, setValue)
   GUI_CS_PROPERTY_NOTIFY(value, cs_valueChanged)
   GUI_CS_PROPERTY_USER(value, true)

   GUI_CS_PROPERTY_READ(displayIntegerBase, displayIntegerBase)
   GUI_CS_PROPERTY_WRITE(displayIntegerBase, setDisplayIntegerBase)

 public:
   explicit QSpinBox(QWidget *parent = nullptr);

   QSpinBox(const QSpinBox &) = delete;
   QSpinBox &operator=(const QSpinBox &) = delete;

   ~QSpinBox();

   int value() const;

   QString prefix() const;
   void setPrefix(const QString &prefix);

   QString suffix() const;
   void setSuffix(const QString &suffix);

   QString cleanText() const;

   int singleStep() const;
   void setSingleStep(int val);

   int minimum() const;
   void setMinimum(int min);

   int maximum() const;
   void setMaximum(int max);

   void setRange(int min, int max);

   int displayIntegerBase() const;
   void setDisplayIntegerBase(int base);

   GUI_CS_SLOT_1(Public, void setValue(int val))
   GUI_CS_SLOT_2(setValue)

   GUI_CS_SIGNAL_1(Public, void valueChanged(int newValue))
   GUI_CS_SIGNAL_OVERLOAD(valueChanged, (int), newValue)

   GUI_CS_SIGNAL_1(Public, void valueChanged(const QString &text))
   GUI_CS_SIGNAL_OVERLOAD(valueChanged, (const QString &), text)

   // wrapper for property
   GUI_CS_SIGNAL_1(Public, void cs_valueChanged(int newValue))
   GUI_CS_SIGNAL_2(cs_valueChanged, newValue)

 protected:
   bool event(QEvent *event) override;
   QValidator::State validate(QString &input, int &pos) const override;
   virtual int valueFromText(const QString &text) const;
   virtual QString textFromValue(int value) const;
   void fixup(QString &input) const override;

 private:
   Q_DECLARE_PRIVATE(QSpinBox)
};

class Q_GUI_EXPORT QDoubleSpinBox : public QAbstractSpinBox
{
   GUI_CS_OBJECT(QDoubleSpinBox)

   GUI_CS_PROPERTY_READ(prefix, prefix)
   GUI_CS_PROPERTY_WRITE(prefix, setPrefix)

   GUI_CS_PROPERTY_READ(suffix, suffix)
   GUI_CS_PROPERTY_WRITE(suffix, setSuffix)

   GUI_CS_PROPERTY_READ(cleanText, cleanText)

   GUI_CS_PROPERTY_READ(decimals, decimals)
   GUI_CS_PROPERTY_WRITE(decimals, setDecimals)

   GUI_CS_PROPERTY_READ(minimum, minimum)
   GUI_CS_PROPERTY_WRITE(minimum, setMinimum)

   GUI_CS_PROPERTY_READ(maximum, maximum)
   GUI_CS_PROPERTY_WRITE(maximum, setMaximum)

   GUI_CS_PROPERTY_READ(singleStep, singleStep)
   GUI_CS_PROPERTY_WRITE(singleStep, setSingleStep)

   GUI_CS_PROPERTY_READ(value, value)
   GUI_CS_PROPERTY_WRITE(value, setValue)
   GUI_CS_PROPERTY_NOTIFY(value, cs_valueChanged)
   GUI_CS_PROPERTY_USER(value, true)

 public:
   explicit QDoubleSpinBox(QWidget *parent = nullptr);

   QDoubleSpinBox(const QDoubleSpinBox &) = delete;
   QDoubleSpinBox &operator=(const QDoubleSpinBox &) = delete;

   ~QDoubleSpinBox();

   double value() const;

   QString prefix() const;
   void setPrefix(const QString &prefix);

   QString suffix() const;
   void setSuffix(const QString &suffix);

   QString cleanText() const;

   double singleStep() const;
   void setSingleStep(double val);

   double minimum() const;
   void setMinimum(double min);

   double maximum() const;
   void setMaximum(double max);

   void setRange(double min, double max);

   int decimals() const;
   void setDecimals(int prec);

   QValidator::State validate(QString &input, int &pos) const override;
   virtual double valueFromText(const QString &text) const;
   virtual QString textFromValue(double value) const;
   void fixup(QString &input) const override;

   GUI_CS_SLOT_1(Public, void setValue(double val))
   GUI_CS_SLOT_2(setValue)

   GUI_CS_SIGNAL_1(Public, void valueChanged(double newValue))
   GUI_CS_SIGNAL_OVERLOAD(valueChanged, (double), newValue)

   GUI_CS_SIGNAL_1(Public, void valueChanged(const QString &text))
   GUI_CS_SIGNAL_OVERLOAD(valueChanged, (const QString &), text)

   // wrapper for property
   GUI_CS_SIGNAL_1(Public, void cs_valueChanged(double newValue))
   GUI_CS_SIGNAL_2(cs_valueChanged, newValue)

 private:
   Q_DECLARE_PRIVATE(QDoubleSpinBox)
};

#endif

#endif // QSPINBOX_H
