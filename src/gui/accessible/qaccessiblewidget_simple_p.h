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

#ifndef SIMPLEWIDGETS_H
#define SIMPLEWIDGETS_H

#include <qcoreapplication.h>
#include <qaccessiblewidget.h>

#ifndef QT_NO_ACCESSIBILITY

class QAbstractButton;
class QLineEdit;
class QToolButton;
class QGroupBox;
class QProgressBar;

class QAccessibleButton : public QAccessibleWidget
{
   Q_DECLARE_TR_FUNCTIONS(QAccessibleButton)

 public:
   QAccessibleButton(QWidget *w);

   QString text(QAccessible::Text t) const override;
   QAccessible::State state() const override;
   QRect rect() const override;
   QAccessible::Role role() const override;

   QStringList actionNames() const override;
   void doAction(const QString &actionName) override;
   QStringList keyBindingsForAction(const QString &actionName) const override;

 protected:
   QAbstractButton *button() const;
};

#ifndef QT_NO_TOOLBUTTON
class QAccessibleToolButton : public QAccessibleButton
{
 public:
   QAccessibleToolButton(QWidget *w);

   QAccessible::State state() const override;
   QAccessible::Role role() const override;

   int childCount() const override;
   QAccessibleInterface *child(int index) const override;

   // QAccessibleActionInterface
   QStringList actionNames() const override;
   void doAction(const QString &actionName) override;

 protected:
   QToolButton *toolButton() const;

   bool isSplitButton() const;
};
#endif // QT_NO_TOOLBUTTON

class QAccessibleDisplay : public QAccessibleWidget, public QAccessibleImageInterface
{
 public:
   explicit QAccessibleDisplay(QWidget *w, QAccessible::Role role = QAccessible::StaticText);

   QString text(QAccessible::Text t) const override;
   QAccessible::Role role() const override;

   QVector<QPair<QAccessibleInterface *, QAccessible::Relation>>relations(QAccessible::Relation match = QAccessible::AllRelations) const
      override;
   void *interface_cast(QAccessible::InterfaceType t) override;

   // QAccessibleImageInterface
   QString imageDescription() const override;
   QSize imageSize() const override;
   QPoint imagePosition() const override;
};

#ifndef QT_NO_GROUPBOX
class QAccessibleGroupBox : public QAccessibleWidget
{
 public:
   explicit QAccessibleGroupBox(QWidget *w);

   QAccessible::State state() const override;
   QAccessible::Role role() const override;
   QString text(QAccessible::Text t) const override;

   QVector<QPair<QAccessibleInterface *, QAccessible::Relation>>relations(QAccessible::Relation match = QAccessible::AllRelations) const
      override;

   //QAccessibleActionInterface
   QStringList actionNames() const override;
   void doAction(const QString &actionName) override;
   QStringList keyBindingsForAction(const QString &) const override;

 private:
   QGroupBox *groupBox() const;
};
#endif

#ifndef QT_NO_LINEEDIT
class QAccessibleLineEdit : public QAccessibleWidget, public QAccessibleTextInterface, public QAccessibleEditableTextInterface
{
 public:
   explicit QAccessibleLineEdit(QWidget *o, const QString &name = QString());

   QString text(QAccessible::Text t) const override;
   void setText(QAccessible::Text t, const QString &text) override;
   QAccessible::State state() const override;
   void *interface_cast(QAccessible::InterfaceType t) override;

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
      int *startOffset, int *endOffset) const override;
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
   QLineEdit *lineEdit() const;
   friend class QAccessibleAbstractSpinBox;
};
#endif // QT_NO_LINEEDIT

#ifndef QT_NO_PROGRESSBAR
class QAccessibleProgressBar : public QAccessibleDisplay, public QAccessibleValueInterface
{
 public:
   explicit QAccessibleProgressBar(QWidget *o);
   void *interface_cast(QAccessible::InterfaceType t) override;

   // QAccessibleValueInterface
   QVariant currentValue() const override;
   QVariant maximumValue() const override;
   QVariant minimumValue() const override;
   QVariant minimumStepSize() const override;
   void setCurrentValue(const QVariant &) override {}

 protected:
   QProgressBar *progressBar() const;
};
#endif

class QWindowContainer;

class QAccessibleWindowContainer : public QAccessibleWidget
{
 public:
   QAccessibleWindowContainer(QWidget *w);
   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *child) const override;
   QAccessibleInterface *child(int i) const override;

 private:
   QWindowContainer *container() const;
};

#endif // QT_NO_ACCESSIBILITY

#endif
