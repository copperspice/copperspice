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

#ifndef RANGECONTROLS_H
#define RANGECONTROLS_H

#include <qaccessiblewidget.h>

#ifndef QT_NO_ACCESSIBILITY

class QAbstractSpinBox;
class QAbstractSlider;
class QScrollBar;
class QSlider;
class QSpinBox;
class QDoubleSpinBox;
class QDial;
class QAccessibleLineEdit;

#ifndef QT_NO_SPINBOX

class QAccessibleAbstractSpinBox : public QAccessibleWidget, public QAccessibleValueInterface,
            public QAccessibleTextInterface, public QAccessibleEditableTextInterface
{
 public:
   explicit QAccessibleAbstractSpinBox(QWidget *w);
   virtual ~QAccessibleAbstractSpinBox();

   QString text(QAccessible::Text t) const override;
   void *interface_cast(QAccessible::InterfaceType t) override;

   // QAccessibleValueInterface
   QVariant currentValue() const override;
   void setCurrentValue(const QVariant &value) override;
   QVariant maximumValue() const override;
   QVariant minimumValue() const override;
   QVariant minimumStepSize() const override;

   // QAccessibleTextInterface
   void addSelection(int startOffset, int endOffset) override;
   QString attributes(int offset, int *startOffset, int *endOffset) const override;
   int cursorPosition() const override;
   QRect characterRect(int offset) const override;
   int selectionCount() const override;
   int offsetAtPoint(const QPoint &point) const override;
   void selection(int selectionIndex, int *startOffset, int *endOffset) const override;

   QString text(int startOffset, int endOffset) const override;
   QString textBeforeOffset (int offset, QAccessible::TextBoundaryType boundaryType,
      int *endOffset, int *startOffset) const override;
   QString textAfterOffset(int offset, QAccessible::TextBoundaryType boundaryType,
      int *startOffset, int *endOffset) const override;
   QString textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType,
      int *startOffset, int *endOffset) const override;
   void removeSelection(int selectionIndex) override;
   void setCursorPosition(int position) override;
   void setSelection(int selectionIndex, int startOffset, int endOffset) override;
   int characterCount() const override;
   void scrollToSubstring(int startIndex, int endIndex) override;

   // QAccessibleEditableTextInterface
   void deleteText(int startOffset, int endOffset) override;
   void insertText(int offset, const QString &text) override;
   void replaceText(int startOffset, int endOffset, const QString &text) override;

 protected:
   QAbstractSpinBox *abstractSpinBox() const;
   QAccessibleInterface *lineEditIface() const;

 private:
   mutable QAccessibleLineEdit *lineEdit;
};

class QAccessibleSpinBox : public QAccessibleAbstractSpinBox
{
 public:
   explicit QAccessibleSpinBox(QWidget *w);

 protected:
   QSpinBox *spinBox() const;
};

class QAccessibleDoubleSpinBox : public QAccessibleAbstractSpinBox
{
 public:
   explicit QAccessibleDoubleSpinBox(QWidget *widget);

   QString text(QAccessible::Text t) const override;

   using QAccessibleAbstractSpinBox::text;

 protected:
   QDoubleSpinBox *doubleSpinBox() const;
};
#endif // QT_NO_SPINBOX

class QAccessibleAbstractSlider: public QAccessibleWidget, public QAccessibleValueInterface
{
 public:
   explicit QAccessibleAbstractSlider(QWidget *w, QAccessible::Role r = QAccessible::Slider);
   void *interface_cast(QAccessible::InterfaceType t) override;

   // QAccessibleValueInterface
   QVariant currentValue() const override;
   void setCurrentValue(const QVariant &value) override;
   QVariant maximumValue() const override;
   QVariant minimumValue() const override;
   QVariant minimumStepSize() const override;

 protected:
   QAbstractSlider *abstractSlider() const;
};

#ifndef QT_NO_SCROLLBAR
class QAccessibleScrollBar : public QAccessibleAbstractSlider
{
 public:
   explicit QAccessibleScrollBar(QWidget *w);
   QString text(QAccessible::Text t) const override;

 protected:
   QScrollBar *scrollBar() const;
};
#endif

#ifndef QT_NO_SLIDER
class QAccessibleSlider : public QAccessibleAbstractSlider
{
 public:
   explicit QAccessibleSlider(QWidget *w);
   QString text(QAccessible::Text t) const override;

 protected:
   QSlider *slider() const;
};
#endif

#ifndef QT_NO_DIAL
class QAccessibleDial : public QAccessibleAbstractSlider
{
 public:
   explicit QAccessibleDial(QWidget *w);

   QString text(QAccessible::Text textType) const override;

 protected:
   QDial *dial() const;
};
#endif

#endif // QT_NO_ACCESSIBILITY

#endif
