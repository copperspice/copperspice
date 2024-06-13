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

#ifndef MESSAGEEDITOR_H
#define MESSAGEEDITOR_H

#include <messagemodel.h>

#include <qframe.h>
#include <qlocale.h>
#include <qscrollarea.h>
#include <qtimer.h>

class QBoxLayout;
class QMainWindow;
class QTextEdit;

class FormMultiWidget;
class FormWidget;
class FormatTextEdit;
class MessageEditor;

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
   CS_OBJECT(MessageEditor)

 public:
   MessageEditor(MultiDataModel *dataModel, QMainWindow *parent = nullptr);

   void showNothing();
   void showMessage(const MultiDataIndex &index);
   void setNumerusForms(int model, const QStringList &numerusForms);
   bool eventFilter(QObject *, QEvent *) override;
   void setTranslationNumerus(int model, const QString &translation, int numerus);

   int activeModel() const {
      return (m_editors.count() != 1) ? m_currentModel : 0;
   }

   void setEditorFocusModel(int model);
   void setUnfinishedEditorFocus();
   bool focusNextUnfinished();

   void setVisualizeWhitespace(bool value);
   void setFontSize(const float fontSize);
   float fontSize();

   CS_SIGNAL_1(Public, void translationChanged(const QStringList & translations))
   CS_SIGNAL_2(translationChanged,translations)

   CS_SIGNAL_1(Public, void translatorCommentChanged(const QString & comment))
   CS_SIGNAL_2(translatorCommentChanged,comment)

   CS_SIGNAL_1(Public, void activeModelChanged(int model))
   CS_SIGNAL_2(activeModelChanged,model)

   CS_SIGNAL_1(Public, void undoAvailable(bool avail))
   CS_SIGNAL_2(undoAvailable,avail)

   CS_SIGNAL_1(Public, void redoAvailable(bool avail))
   CS_SIGNAL_2(redoAvailable,avail)

   CS_SIGNAL_1(Public, void cutAvailable(bool avail))
   CS_SIGNAL_2(cutAvailable,avail)

   CS_SIGNAL_1(Public, void copyAvailable(bool avail))
   CS_SIGNAL_2(copyAvailable,avail)

   CS_SIGNAL_1(Public, void pasteAvailable(bool avail))
   CS_SIGNAL_2(pasteAvailable,avail)

   CS_SIGNAL_1(Public, void beginFromSourceAvailable(bool enable))
   CS_SIGNAL_2(beginFromSourceAvailable,enable)

   CS_SLOT_1(Public, void undo())
   CS_SLOT_2(undo)

   CS_SLOT_1(Public, void redo())
   CS_SLOT_2(redo)

   CS_SLOT_1(Public, void cut())
   CS_SLOT_2(cut)

   CS_SLOT_1(Public, void copy())
   CS_SLOT_2(copy)

   CS_SLOT_1(Public, void paste())
   CS_SLOT_2(paste)

   CS_SLOT_1(Public, void selectAll())
   CS_SLOT_2(selectAll)

   CS_SLOT_1(Public, void beginFromSource())
   CS_SLOT_2(beginFromSource)

   CS_SLOT_1(Public, void setEditorFocus())
   CS_SLOT_2(setEditorFocus)

   CS_SLOT_1(Public, void setTranslation(int model, const QString &translation))
   CS_SLOT_2(setTranslation)

   CS_SLOT_1(Public, void setLengthVariants(bool on))
   CS_SLOT_2(setLengthVariants)

   CS_SLOT_1(Public, void increaseFontSize())
   CS_SLOT_2(increaseFontSize)

   CS_SLOT_1(Public, void decreaseFontSize())
   CS_SLOT_2(decreaseFontSize)

   CS_SLOT_1(Public, void resetFontSize())
   CS_SLOT_2(resetFontSize)

 private:
   CS_SLOT_1(Private, void editorCreated(QTextEdit *editor))
   CS_SLOT_2(editorCreated)

   CS_SLOT_1(Private, void editorDestroyed())
   CS_SLOT_2(editorDestroyed)

   CS_SLOT_1(Private, void selectionChanged(QTextEdit *editor))
   CS_SLOT_2(selectionChanged)

   CS_SLOT_1(Private, void resetHoverSelection())
   CS_SLOT_2(resetHoverSelection)

   CS_SLOT_1(Private, void emitTranslationChanged(QTextEdit *editor))
   CS_SLOT_2(emitTranslationChanged)

   CS_SLOT_1(Private, void emitTranslatorCommentChanged(QTextEdit *editor))
   CS_SLOT_2(emitTranslatorCommentChanged)

   CS_SLOT_1(Private, void updateCanPaste())
   CS_SLOT_2(updateCanPaste)

   CS_SLOT_1(Private, void clipboardChanged())
   CS_SLOT_2(clipboardChanged)

   CS_SLOT_1(Private, void messageModelAppended())
   CS_SLOT_2(messageModelAppended)

   CS_SLOT_1(Private, void messageModelDeleted(int model))
   CS_SLOT_2(messageModelDeleted)

   CS_SLOT_1(Private, void allModelsDeleted())
   CS_SLOT_2(allModelsDeleted)

   CS_SLOT_1(Private, void setTargetLanguage(int model))
   CS_SLOT_2(setTargetLanguage)

   CS_SLOT_1(Private, void reallyFixTabOrder())
   CS_SLOT_2(reallyFixTabOrder)

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
   void applyFontSize();

   MultiDataModel *m_dataModel;

   MultiDataIndex m_currentIndex;
   int m_currentModel;
   int m_currentNumerus;

   bool m_lengthVariants;
   float m_fontSize;

   bool m_undoAvail;
   bool m_redoAvail;
   bool m_cutAvail;
   bool m_copyAvail;

   bool m_clipboardEmpty;
   bool m_visualizeWhitespace;

   QTextEdit *m_selectionHolder;
   QWidget *m_focusWidget;
   QBoxLayout *m_layout;
   FormWidget *m_source;
   FormWidget *m_pluralSource;
   FormWidget *m_commentText;
   QList<MessageEditorData> m_editors;

   QTimer m_tabOrderTimer;
};

#endif
