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

#ifndef MESSAGEEDITORWIDGETS_H
#define MESSAGEEDITORWIDGETS_H

class MessageHighlighter;

#include <qicon.h>
#include <qimage.h>
#include <qlabel.h>
#include <qmap.h>
#include <qstringfwd.h>
#include <qtextedit.h>
#include <qurl.h>
#include <qwidget.h>

class QAbstractButton;
class QAction;
class QContextMenuEvent;
class QKeyEvent;
class QMenu;
class QSizeF;
class QVariant;

// Automatically adapt height to document contents

class ExpandingTextEdit : public QTextEdit
{
   CS_OBJECT(ExpandingTextEdit)

 public:
   ExpandingTextEdit(QWidget *parent = nullptr);
   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

 private:
   int m_minimumHeight;

   CS_SLOT_1(Private, void updateHeight(const QSizeF & documentSize))
   CS_SLOT_2(updateHeight)

   CS_SLOT_1(Private, void reallyEnsureCursorVisible())
   CS_SLOT_2(reallyEnsureCursorVisible)
};

class FormatTextEdit : public ExpandingTextEdit
{
   CS_OBJECT(FormatTextEdit)

 public:
   FormatTextEdit(QWidget *parent = nullptr);
   ~FormatTextEdit();

   void setEditable(bool editable);

   CS_SIGNAL_1(Public, void editorDestroyed())
   CS_SIGNAL_2(editorDestroyed)

   CS_SLOT_1(Public, void setPlainText(const QString & text,bool userAction))
   CS_SLOT_2(setPlainText)

   CS_SLOT_1(Public, void setVisualizeWhitespace(bool value))
   CS_SLOT_2(setVisualizeWhitespace)

 private:
   MessageHighlighter *m_highlighter;
};

class FormWidget : public QWidget
{
   CS_OBJECT(FormWidget)

 public:
   FormWidget(const QString &label, bool isEditable, QWidget *parent = nullptr);

   void setLabel(const QString &label) {
      m_label->setText(label);
   }

   void setTranslation(const QString &text, bool userAction = false);
   void clearTranslation() {
      setTranslation(QString(), false);
   }

   QString getTranslation() {
      return m_editor->toPlainText();
   }

   void setEditingEnabled(bool enable);
   void setHideWhenEmpty(bool optional) {
      m_hideWhenEmpty = optional;
   }

   FormatTextEdit *getEditor() {
      return m_editor;
   }

   CS_SIGNAL_1(Public, void textChanged(QTextEdit *editor))
   CS_SIGNAL_2(textChanged, editor)

   CS_SIGNAL_1(Public, void selectionChanged(QTextEdit *editor))
   CS_SIGNAL_2(selectionChanged, editor)

   CS_SIGNAL_1(Public, void cursorPositionChanged())
   CS_SIGNAL_2(cursorPositionChanged)

 private:
   QLabel *m_label;
   FormatTextEdit *m_editor;
   bool m_hideWhenEmpty;

   // slots
   void slotSelectionChanged();
   void slotTextChanged();
};

class FormMultiWidget : public QWidget
{
   CS_OBJECT(FormMultiWidget)

 public:
   FormMultiWidget(const QString &label, QWidget *parent = nullptr);
   void setLabel(const QString &label) {
      m_label->setText(label);
   }

   void setTranslation(const QString &text, bool userAction = false);
   void clearTranslation() {
      setTranslation(QString(), false);
   }

   QString getTranslation() const;
   void setEditingEnabled(bool enable);
   void setMultiEnabled(bool enable);
   void setHideWhenEmpty(bool optional) {
      m_hideWhenEmpty = optional;
   }
   const QList<FormatTextEdit *> &getEditors() const {
      return m_editors;
   }

   CS_SIGNAL_1(Public, void editorCreated(QTextEdit *editor))
   CS_SIGNAL_2(editorCreated, editor)

   CS_SIGNAL_1(Public, void textChanged(QTextEdit *editor))
   CS_SIGNAL_2(textChanged, editor)

   CS_SIGNAL_1(Public, void selectionChanged(QTextEdit *editor))
   CS_SIGNAL_2(selectionChanged, editor)

   CS_SIGNAL_1(Public, void cursorPositionChanged())
   CS_SIGNAL_2(cursorPositionChanged)

 protected:
   bool eventFilter(QObject *watched, QEvent *event) override;

 private:
   void addEditor(int idx);
   void updateLayout();
   QAbstractButton *makeButton(const QIcon &icon, const QString &slotMethod);
   void insertEditor(int idx);
   void deleteEditor(int idx);

   // slots
   void slotSelectionChanged();
   void slotTextChanged();

   QLabel *m_label;
   QList<FormatTextEdit *> m_editors;
   QList<QWidget *> m_plusButtons;
   QList<QAbstractButton *> m_minusButtons;
   bool m_hideWhenEmpty;
   bool m_multiEnabled;
   QIcon m_plusIcon, m_minusIcon;

   CS_SLOT_1(Private, void minusButtonClicked())
   CS_SLOT_2(minusButtonClicked)

   CS_SLOT_1(Private, void plusButtonClicked())
   CS_SLOT_2(plusButtonClicked)
};

#endif
