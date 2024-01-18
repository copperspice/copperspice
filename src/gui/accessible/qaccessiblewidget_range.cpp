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

#include <qaccessiblewidget_range_p.h>

#include <qslider.h>
#include <qdial.h>
#include <qspinbox.h>
#include <qscrollbar.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qglobal.h>
#include <QDoubleSpinBox>
#include <QDial>
#include <qlineedit.h>
#include <qmath.h>
#include <qmath_p.h>

#include <qaccessiblewidget_simple_p.h> // let spinbox use line edit's interface

#ifndef QT_NO_ACCESSIBILITY

#ifndef QT_NO_SPINBOX
QAccessibleAbstractSpinBox::QAccessibleAbstractSpinBox(QWidget *w)
   : QAccessibleWidget(w, QAccessible::SpinBox), lineEdit(nullptr)
{
   Q_ASSERT(abstractSpinBox());
}

QAccessibleAbstractSpinBox::~QAccessibleAbstractSpinBox()
{
   delete lineEdit;
}

QAbstractSpinBox *QAccessibleAbstractSpinBox::abstractSpinBox() const
{
   return qobject_cast<QAbstractSpinBox *>(object());
}

QAccessibleInterface *QAccessibleAbstractSpinBox::lineEditIface() const
{
   // QAccessibleLineEdit is only used to forward the text functions
   if (! lineEdit) {
      lineEdit = new QAccessibleLineEdit(abstractSpinBox()->lineEdit());
   }

   return lineEdit;
}

QString QAccessibleAbstractSpinBox::text(QAccessible::Text t) const
{
   if (t == QAccessible::Value) {
      return abstractSpinBox()->text();
   }
   return QAccessibleWidget::text(t);
}

void *QAccessibleAbstractSpinBox::interface_cast(QAccessible::InterfaceType t)
{
   if (t == QAccessible::ValueInterface) {
      return static_cast<QAccessibleValueInterface *>(this);
   }
   if (t == QAccessible::TextInterface) {
      return static_cast<QAccessibleTextInterface *>(this);
   }
   if (t == QAccessible::EditableTextInterface) {
      return static_cast<QAccessibleEditableTextInterface *>(this);
   }
   return QAccessibleWidget::interface_cast(t);
}

QVariant QAccessibleAbstractSpinBox::currentValue() const
{
   return abstractSpinBox()->property("value");
}

void QAccessibleAbstractSpinBox::setCurrentValue(const QVariant &value)
{
   abstractSpinBox()->setProperty("value", value);
}

QVariant QAccessibleAbstractSpinBox::maximumValue() const
{
   return abstractSpinBox()->property("maximum");
}

QVariant QAccessibleAbstractSpinBox::minimumValue() const
{
   return abstractSpinBox()->property("minimum");
}

QVariant QAccessibleAbstractSpinBox::minimumStepSize() const
{
   return abstractSpinBox()->property("stepSize");
}

void QAccessibleAbstractSpinBox::addSelection(int startOffset, int endOffset)
{
   lineEditIface()->textInterface()->addSelection(startOffset, endOffset);
}

QString QAccessibleAbstractSpinBox::attributes(int offset, int *startOffset, int *endOffset) const
{
   return lineEditIface()->textInterface()->attributes(offset, startOffset, endOffset);
}

int QAccessibleAbstractSpinBox::cursorPosition() const
{
   return lineEditIface()->textInterface()->cursorPosition();
}

QRect QAccessibleAbstractSpinBox::characterRect(int offset) const
{
   return lineEditIface()->textInterface()->characterRect(offset);
}

int QAccessibleAbstractSpinBox::selectionCount() const
{
   return lineEditIface()->textInterface()->selectionCount();
}

int QAccessibleAbstractSpinBox::offsetAtPoint(const QPoint &point) const
{
   return lineEditIface()->textInterface()->offsetAtPoint(point);
}

void QAccessibleAbstractSpinBox::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
   lineEditIface()->textInterface()->selection(selectionIndex, startOffset, endOffset);
}

QString QAccessibleAbstractSpinBox::text(int startOffset, int endOffset) const
{
   return lineEditIface()->textInterface()->text(startOffset, endOffset);
}

QString QAccessibleAbstractSpinBox::textBeforeOffset(int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset,
   int *endOffset) const
{
   return lineEditIface()->textInterface()->textBeforeOffset(offset, boundaryType, startOffset, endOffset);
}

QString QAccessibleAbstractSpinBox::textAfterOffset(int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset,
   int *endOffset) const
{
   return lineEditIface()->textInterface()->textAfterOffset(offset, boundaryType, startOffset, endOffset);
}

QString QAccessibleAbstractSpinBox::textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset,
   int *endOffset) const
{
   return lineEditIface()->textInterface()->textAtOffset(offset, boundaryType, startOffset, endOffset);
}

void QAccessibleAbstractSpinBox::removeSelection(int selectionIndex)
{
   lineEditIface()->textInterface()->removeSelection(selectionIndex);
}

void QAccessibleAbstractSpinBox::setCursorPosition(int position)
{
   lineEditIface()->textInterface()->setCursorPosition(position);
}

void QAccessibleAbstractSpinBox::setSelection(int selectionIndex, int startOffset, int endOffset)
{
   lineEditIface()->textInterface()->setSelection(selectionIndex, startOffset, endOffset);
}

int QAccessibleAbstractSpinBox::characterCount() const
{
   return lineEditIface()->textInterface()->characterCount();
}

void QAccessibleAbstractSpinBox::scrollToSubstring(int startIndex, int endIndex)
{
   lineEditIface()->textInterface()->scrollToSubstring(startIndex, endIndex);
}

void QAccessibleAbstractSpinBox::deleteText(int startOffset, int endOffset)
{
   lineEditIface()->editableTextInterface()->deleteText(startOffset, endOffset);
}

void QAccessibleAbstractSpinBox::insertText(int offset, const QString &text)
{
   lineEditIface()->editableTextInterface()->insertText(offset, text);
}

void QAccessibleAbstractSpinBox::replaceText(int startOffset, int endOffset, const QString &text)
{
   lineEditIface()->editableTextInterface()->replaceText(startOffset, endOffset, text);
}

QAccessibleSpinBox::QAccessibleSpinBox(QWidget *w)
   : QAccessibleAbstractSpinBox(w)
{
   Q_ASSERT(spinBox() != nullptr);

   const QMetaObject &metaObj = QSpinBox::staticMetaObject();

   int signalIndex1          = metaObj.indexOfSignal(cs_mp_cast<int>(&QSpinBox::valueChanged));
   QMetaMethod signalMethod1 = metaObj.method(signalIndex1);
   addControllingSignal(signalMethod1);

   int signalIndex2          = metaObj.indexOfSignal(cs_mp_cast<const QString &>(&QSpinBox::valueChanged));
   QMetaMethod signalMethod2 = metaObj.method(signalIndex2);
   addControllingSignal(signalMethod2);
}

QSpinBox *QAccessibleSpinBox::spinBox() const
{
   return qobject_cast<QSpinBox *>(object());
}


// ================================== QAccessibleDoubleSpinBox ==================================
QAccessibleDoubleSpinBox::QAccessibleDoubleSpinBox(QWidget *widget)
   : QAccessibleAbstractSpinBox(widget)
{
   Q_ASSERT(qobject_cast<QDoubleSpinBox *>(widget));

   const QMetaObject &metaObj = QDoubleSpinBox::staticMetaObject();

   int signalIndex1          = metaObj.indexOfSignal(cs_mp_cast<double>(&QDoubleSpinBox::valueChanged));
   QMetaMethod signalMethod1 = metaObj.method(signalIndex1);
   addControllingSignal(signalMethod1);

   int signalIndex2          = metaObj.indexOfSignal(cs_mp_cast<const QString &>(&QDoubleSpinBox::valueChanged));
   QMetaMethod signalMethod2 = metaObj.method(signalIndex2);
   addControllingSignal(signalMethod2);
}

QDoubleSpinBox *QAccessibleDoubleSpinBox::doubleSpinBox() const
{
   return static_cast<QDoubleSpinBox *>(object());
}

QString QAccessibleDoubleSpinBox::text(QAccessible::Text textType) const
{
   if (textType == QAccessible::Value) {
      return doubleSpinBox()->textFromValue(doubleSpinBox()->value());
   }

   return QAccessibleWidget::text(textType);
}

#endif // QT_NO_SPINBOX


#ifndef QT_NO_SCROLLBAR

QAccessibleScrollBar::QAccessibleScrollBar(QWidget *w)
   : QAccessibleAbstractSlider(w, QAccessible::ScrollBar)
{
   Q_ASSERT(scrollBar());

   const QMetaObject &metaObj = QScrollBar::staticMetaObject();

   int signalIndex          = metaObj.indexOfSignal(&QScrollBar::valueChanged);
   QMetaMethod signalMethod = metaObj.method(signalIndex);
   addControllingSignal(signalMethod);
}

QScrollBar *QAccessibleScrollBar::scrollBar() const
{
   return qobject_cast<QScrollBar *>(object());
}

QString QAccessibleScrollBar::text(QAccessible::Text t) const
{
   if (t == QAccessible::Value) {
      return QString::number(scrollBar()->value());
   }

   return QAccessibleAbstractSlider::text(t);
}

#endif // QT_NO_SCROLLBAR


#ifndef QT_NO_SLIDER

QAccessibleSlider::QAccessibleSlider(QWidget *w)
   : QAccessibleAbstractSlider(w)
{
   Q_ASSERT(slider());

   const QMetaObject &metaObj = QSlider::staticMetaObject();

   int signalIndex          = metaObj.indexOfSignal(&QSlider::valueChanged);
   QMetaMethod signalMethod = metaObj.method(signalIndex);
   addControllingSignal(signalMethod);
}

QSlider *QAccessibleSlider::slider() const
{
   return qobject_cast<QSlider *>(object());
}

QString QAccessibleSlider::text(QAccessible::Text t) const
{
   if (t == QAccessible::Value) {
      return QString::number(slider()->value());
   }

   return QAccessibleAbstractSlider::text(t);
}

QAccessibleAbstractSlider::QAccessibleAbstractSlider(QWidget *w, QAccessible::Role r)
   : QAccessibleWidget(w, r)
{
   Q_ASSERT(qobject_cast<QAbstractSlider *>(w));
}

void *QAccessibleAbstractSlider::interface_cast(QAccessible::InterfaceType t)
{
   if (t == QAccessible::ValueInterface) {
      return static_cast<QAccessibleValueInterface *>(this);
   }
   return QAccessibleWidget::interface_cast(t);
}

QVariant QAccessibleAbstractSlider::currentValue() const
{
   return abstractSlider()->value();
}

void QAccessibleAbstractSlider::setCurrentValue(const QVariant &value)
{
   abstractSlider()->setValue(value.toInt());
}

QVariant QAccessibleAbstractSlider::maximumValue() const
{
   return abstractSlider()->maximum();
}

QVariant QAccessibleAbstractSlider::minimumValue() const
{
   return abstractSlider()->minimum();
}

QVariant QAccessibleAbstractSlider::minimumStepSize() const
{
   return abstractSlider()->singleStep();
}

QAbstractSlider *QAccessibleAbstractSlider::abstractSlider() const
{
   return static_cast<QAbstractSlider *>(object());
}

#endif // QT_NO_SLIDER


#ifndef QT_NO_DIAL

QAccessibleDial::QAccessibleDial(QWidget *widget)
   : QAccessibleAbstractSlider(widget, QAccessible::Dial)
{
   Q_ASSERT(qobject_cast<QDial *>(widget));

   const QMetaObject &metaObj = QDial::staticMetaObject();

   int signalIndex          = metaObj.indexOfSignal(&QDial::valueChanged);
   QMetaMethod signalMethod = metaObj.method(signalIndex);
   addControllingSignal(signalMethod);
}

QString QAccessibleDial::text(QAccessible::Text textType) const
{
   if (textType == QAccessible::Value) {
      return QString::number(dial()->value());
   }

   return QAccessibleAbstractSlider::text(textType);
}

QDial *QAccessibleDial::dial() const
{
   return static_cast<QDial *>(object());
}
#endif // QT_NO_DIAL

#endif // QT_NO_ACCESSIBILITY

