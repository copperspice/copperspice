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

#ifndef MESSAGEEDITORWIDGETS_H
#define MESSAGEEDITORWIDGETS_H

#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QMap>
#include <QTextEdit>
#include <QUrl>
#include <QWidget>

QT_BEGIN_NAMESPACE

class QAbstractButton;
class QAction;
class QContextMenuEvent;
class QKeyEvent;
class QMenu;
class QSizeF;
class QString;
class QVariant;

class MessageHighlighter;

/*
  Automatically adapt height to document contents
 */
class ExpandingTextEdit : public QTextEdit
{
   Q_OBJECT

 public:
   ExpandingTextEdit(QWidget *parent = nullptr);
   QSize sizeHint() const;
   QSize minimumSizeHint() const;

 private slots:
   void updateHeight(const QSizeF &documentSize);
   void reallyEnsureCursorVisible();

 private:
   int m_minimumHeight;
};

/*
  Format markup & control characters
*/
class FormatTextEdit : public ExpandingTextEdit
{
   Q_OBJECT
 public:
   FormatTextEdit(QWidget *parent = nullptr);
   ~FormatTextEdit();
   void setEditable(bool editable);

 signals:
   void editorDestroyed();

 public slots:
   void setPlainText(const QString &text, bool userAction);

 private:
   MessageHighlighter *m_highlighter;
};

/*
  Displays text field & associated label
*/
class FormWidget : public QWidget
{
   Q_OBJECT
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

 signals:
   void textChanged(QTextEdit *);
   void selectionChanged(QTextEdit *);
   void cursorPositionChanged();

 private slots:
   void slotSelectionChanged();
   void slotTextChanged();

 private:
   QLabel *m_label;
   FormatTextEdit *m_editor;
   bool m_hideWhenEmpty;
};

/*
  Displays text fields & associated label
*/
class FormMultiWidget : public QWidget
{
   Q_OBJECT
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

 signals:
   void editorCreated(QTextEdit *);
   void textChanged(QTextEdit *);
   void selectionChanged(QTextEdit *);
   void cursorPositionChanged();

 protected:
   bool eventFilter(QObject *watched, QEvent *event);

 private slots:
   void slotTextChanged();
   void slotSelectionChanged();
   void minusButtonClicked();
   void plusButtonClicked();

 private:
   void addEditor(int idx);
   void updateLayout();
   QAbstractButton *makeButton(const QIcon &icon, const char *slot);
   void insertEditor(int idx);
   void deleteEditor(int idx);

   QLabel *m_label;
   QList<FormatTextEdit *> m_editors;
   QList<QWidget *> m_plusButtons;
   QList<QAbstractButton *> m_minusButtons;
   bool m_hideWhenEmpty;
   bool m_multiEnabled;
   QIcon m_plusIcon, m_minusIcon;
};

QT_END_NAMESPACE

#endif // MESSAGEEDITORWIDGETS_H
