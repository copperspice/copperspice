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

#ifndef MESSAGEEDITOR_H
#define MESSAGEEDITOR_H

#include "messagemodel.h"

#include <QtCore/QLocale>
#include <QtCore/QTimer>

#include <QtGui/QFrame>
#include <QtGui/QScrollArea>

QT_BEGIN_NAMESPACE

class QBoxLayout;
class QMainWindow;
class QTextEdit;

class MessageEditor;
class FormatTextEdit;
class FormWidget;
class FormMultiWidget;

struct MessageEditorData {
   QWidget *container;
   FormWidget *transCommentText;
   QList<FormMultiWidget *> transTexts;
   QString invariantForm;
   QString firstForm;
   qreal fontSize;
   bool pluralEditMode;
};

class MessageEditor : public QScrollArea
{
   Q_OBJECT

 public:
   MessageEditor(MultiDataModel *dataModel, QMainWindow *parent = 0);

   void showNothing();
   void showMessage(const MultiDataIndex &index);
   void setNumerusForms(int model, const QStringList &numerusForms);
   bool eventFilter(QObject *, QEvent *);
   void setTranslation(int model, const QString &translation, int numerus);
   int activeModel() const {
      return (m_editors.count() != 1) ? m_currentModel : 0;
   }
   void setEditorFocus(int model);
   void setUnfinishedEditorFocus();
   bool focusNextUnfinished();

 signals:
   void translationChanged(const QStringList &translations);
   void translatorCommentChanged(const QString &comment);
   void activeModelChanged(int model);

   void undoAvailable(bool avail);
   void redoAvailable(bool avail);
   void cutAvailable(bool avail);
   void copyAvailable(bool avail);
   void pasteAvailable(bool avail);
   void beginFromSourceAvailable(bool enable);

 public slots:
   void undo();
   void redo();
   void cut();
   void copy();
   void paste();
   void selectAll();
   void beginFromSource();
   void setEditorFocus();
   void setTranslation(int latestModel, const QString &translation);
   void setLengthVariants(bool on);

 private slots:
   void editorCreated(QTextEdit *);
   void editorDestroyed();
   void selectionChanged(QTextEdit *);
   void resetHoverSelection();
   void emitTranslationChanged(QTextEdit *);
   void emitTranslatorCommentChanged(QTextEdit *);
   void updateCanPaste();
   void clipboardChanged();
   void messageModelAppended();
   void messageModelDeleted(int model);
   void allModelsDeleted();
   void setTargetLanguage(int model);
   void reallyFixTabOrder();

 private:
   void setupEditorPage();
   void setEditingEnabled(int model, bool enabled);
   bool focusNextUnfinished(int start);
   void resetSelection();
   void grabFocus(QWidget *widget);
   void trackFocus(QWidget *widget);
   void activeModelAndNumerus(int *model, int *numerus) const;
   QTextEdit *activeTranslation() const;
   QTextEdit *activeOr1stTranslation() const;
   QTextEdit *activeTransComment() const;
   QTextEdit *activeEditor() const;
   QTextEdit *activeOr1stEditor() const;
   MessageEditorData *modelForWidget(const QObject *o);
   int activeTranslationNumerus() const;
   QStringList translations(int model) const;
   void updateBeginFromSource();
   void updateUndoRedo();
   void updateCanCutCopy();
   void addPluralForm(int model, const QString &label, bool writable);
   void fixTabOrder();
   QPalette paletteForModel(int model) const;

   MultiDataModel *m_dataModel;

   MultiDataIndex m_currentIndex;
   int m_currentModel;
   int m_currentNumerus;

   bool m_lengthVariants;

   bool m_undoAvail;
   bool m_redoAvail;
   bool m_cutAvail;
   bool m_copyAvail;

   bool m_clipboardEmpty;

   QTextEdit *m_selectionHolder;
   QWidget *m_focusWidget;
   QBoxLayout *m_layout;
   FormWidget *m_source;
   FormWidget *m_pluralSource;
   FormWidget *m_commentText;
   QList<MessageEditorData> m_editors;

   QTimer m_tabOrderTimer;
};

QT_END_NAMESPACE

#endif // MESSAGEEDITOR_H
