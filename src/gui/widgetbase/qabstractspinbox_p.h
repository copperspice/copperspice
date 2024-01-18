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

#ifndef QABSTRACTSPINBOX_P_H
#define QABSTRACTSPINBOX_P_H

#include <qabstractspinbox.h>

#ifndef QT_NO_SPINBOX

#include <qlineedit.h>
#include <qstyleoption.h>
#include <qvalidator.h>
#include <qdatetime.h>
#include <qvariant.h>

#include <qwidget_p.h>
#include <qdatetime_p.h>

QVariant operator+(const QVariant &arg1, const QVariant &arg2);
QVariant operator-(const QVariant &arg1, const QVariant &arg2);
QVariant operator*(const QVariant &arg1, double multiplier);

double operator/(const QVariant &arg1, const QVariant &arg2);

enum EmitPolicy {
   EmitIfChanged,
   AlwaysEmit,
   NeverEmit
};

enum Button {
   None = 0x000,
   Keyboard = 0x001,
   Mouse = 0x002,
   Wheel = 0x004,
   ButtonMask = 0x008,
   Up = 0x010,
   Down = 0x020,
   DirectionMask = 0x040
};
class QSpinBoxValidator;
class QAbstractSpinBoxPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QAbstractSpinBox)

 public:
   QAbstractSpinBoxPrivate();
   ~QAbstractSpinBoxPrivate();

   void init();
   void reset();
   void updateState(bool up, bool fromKeyboard = false);
   QString stripped(const QString &text, int *pos = nullptr) const;
   bool specialValue() const;
   virtual QVariant getZeroVariant() const;
   virtual void setRange(const QVariant &min, const QVariant &max);
   void setValue(const QVariant &val, EmitPolicy ep, bool updateEdit = true);
   virtual QVariant bound(const QVariant &val, const QVariant &old = QVariant(), int steps = 0) const;
   virtual void updateEdit();

   virtual void emitSignals(EmitPolicy ep, const QVariant &old);
   virtual void interpret(EmitPolicy ep);
   virtual QString textFromValue(const QVariant &n) const;
   virtual QVariant valueFromText(const QString &input) const;

   void _q_editorTextChanged(const QString &);
   virtual void _q_editorCursorPositionChanged(int oldpos, int newpos);

   virtual QStyle::SubControl newHoverControl(const QPoint &pos);
   bool updateHoverControl(const QPoint &pos);

   virtual void clearCache() const;
   virtual void updateEditFieldGeometry();

   static int variantCompare(const QVariant &arg1, const QVariant &arg2);
   static QVariant variantBound(const QVariant &min, const QVariant &value, const QVariant &max);

   QLineEdit *edit;
   QString prefix, suffix, specialValueText;
   QVariant value, minimum, maximum, singleStep;
   QVariant::Type type;
   int spinClickTimerId, spinClickTimerInterval, spinClickThresholdTimerId, spinClickThresholdTimerInterval;
   int effectiveSpinRepeatRate;
   uint buttonState;
   mutable QString cachedText;
   mutable QVariant cachedValue;
   mutable QValidator::State cachedState;
   mutable QSize cachedSizeHint, cachedMinimumSizeHint;
   uint pendingEmit : 1;
   uint readOnly : 1;
   uint wrapping : 1;
   uint ignoreCursorPositionChanged : 1;
   uint frame : 1;
   uint accelerate : 1;
   uint keyboardTracking : 1;
   uint cleared : 1;
   uint ignoreUpdateEdit : 1;
   QAbstractSpinBox::CorrectionMode correctionMode;
   int acceleration;
   QStyle::SubControl hoverControl;
   QRect hoverRect;
   QAbstractSpinBox::ButtonSymbols buttonSymbols;
   QSpinBoxValidator *validator;
   uint showGroupSeparator : 1;
   int wheelDeltaRemainder;
};

class QSpinBoxValidator : public QValidator
{
 public:
   QSpinBoxValidator(QAbstractSpinBox *qptr, QAbstractSpinBoxPrivate *dptr);
   QValidator::State validate(QString &input, int &) const override;
   void fixup(QString &) const override;

 private:
   QAbstractSpinBox *qptr;
   QAbstractSpinBoxPrivate *dptr;
};

#endif // QT_NO_SPINBOX

#endif
