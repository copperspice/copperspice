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

//  right panel of the main window.

#include <messageeditor.h>
#include <messageeditorwidgets.h>
#include <similartext.h>
#include <phrasemodel.h>

#include <qapplication.h>
#include <qboxlayout.h>
#include <qclipboard.h>
#include <qdebug.h>
#include <qdockwidget.h>
#include <qheaderview.h>
#include <qkeyevent.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qtreeview.h>
#include <qvboxlayout.h>

#if 0

// Allow translators to provide localized names for QLocale::languageToString

static const char *language_strings[] = {
   cs_mark_tr("MessageEditor", "Russian"),
   cs_mark_tr("MessageEditor", "German"),
   cs_mark_tr("MessageEditor", "Japanese"),
   cs_mark_tr("MessageEditor", "French"),
   cs_mark_tr("MessageEditor", "Polish"),
   cs_mark_tr("MessageEditor", "Chinese")
};
#endif

//  Handles layout of dock windows and the editor page
MessageEditor::MessageEditor(MultiDataModel *dataModel, QMainWindow *parent)
   : QScrollArea(parent->centralWidget()), m_dataModel(dataModel), m_currentModel(-1),
     m_currentNumerus(-1), m_lengthVariants(false), m_fontSize(font().pointSize()),
     m_undoAvail(false), m_redoAvail(false), m_cutAvail(false),
     m_copyAvail(false), m_visualizeWhitespace(true),
     m_selectionHolder(nullptr), m_focusWidget(nullptr)
{
   setObjectName("scroll area");

   QPalette p;
   p.setBrush(QPalette::Window, p.brush(QPalette::Active, QPalette::Base));
   setPalette(p);

   setupEditorPage();

   // Signals
   connect(qApp->clipboard(), SIGNAL(dataChanged()),    SLOT(clipboardChanged()));
   connect(m_dataModel, SIGNAL(modelAppended()),        SLOT(messageModelAppended()));
   connect(m_dataModel, SIGNAL(modelDeleted(int)),      SLOT(messageModelDeleted(int)));
   connect(m_dataModel, SIGNAL(allModelsDeleted()),     SLOT(allModelsDeleted()));
   connect(m_dataModel, SIGNAL(languageChanged(int)),   SLOT(setTargetLanguage(int)));

   m_tabOrderTimer.setSingleShot(true);
   connect(&m_tabOrderTimer, SIGNAL(timeout()), SLOT(reallyFixTabOrder()));

   clipboardChanged();

   setWhatsThis(tr("This whole panel allows you to view and edit "
                   "the translation of some source text."));
   showNothing();
}

void MessageEditor::setupEditorPage()
{
   QFrame *editorPage = new QFrame;
   editorPage->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

   m_source = new FormWidget(tr("Source text"), false);
   m_source->setHideWhenEmpty(true);
   m_source->setWhatsThis(tr("This area shows the source text."));

   connect(m_source, &FormWidget::selectionChanged, this, &MessageEditor::selectionChanged);

   m_pluralSource = new FormWidget(tr("Source text (Plural)"), false);
   m_pluralSource->setHideWhenEmpty(true);
   m_pluralSource->setWhatsThis(tr("This area shows the plural form of the source text."));

   connect(m_pluralSource, &FormWidget::selectionChanged, this, &MessageEditor::selectionChanged);

   m_commentText = new FormWidget(tr("Developer comments"), false);
   m_commentText->setHideWhenEmpty(true);
   m_commentText->setObjectName("comment/context view");
   m_commentText->setWhatsThis(tr("This area shows a comment that"
               " may guide you, and the context in which the text occurs.") );

   connect(m_commentText, &FormWidget::selectionChanged, this, &MessageEditor::selectionChanged);

   QBoxLayout *subLayout = new QVBoxLayout;

   subLayout->setMargin(5);
   subLayout->addWidget(m_source);
   subLayout->addWidget(m_pluralSource);
   subLayout->addWidget(m_commentText);

   m_layout = new QVBoxLayout;
   m_layout->setSpacing(2);
   m_layout->setMargin(2);
   m_layout->addLayout(subLayout);
   m_layout->addStretch(1);
   editorPage->setLayout(m_layout);

   setWidget(editorPage);
   setWidgetResizable(true);
}

QPalette MessageEditor::paletteForModel(int model) const
{
   QBrush brush = m_dataModel->brushForModel(model);
   QPalette pal;

   if (m_dataModel->isModelWritable(model)) {
      pal.setBrush(QPalette::Window, brush);
   } else {
      QPixmap pm(brush.texture().size());
      pm.fill();
      QPainter p(&pm);
      p.fillRect(brush.texture().rect(), brush);
      pal.setBrush(QPalette::Window, pm);
   }
   return pal;
}

void MessageEditor::messageModelAppended()
{
   int model = m_editors.size();
   m_editors.append(MessageEditorData());
   MessageEditorData &ed = m_editors.last();
   ed.pluralEditMode = false;
   ed.fontSize = m_fontSize;
   ed.container = new QWidget;

   if (model > 0) {
      ed.container->setPalette(paletteForModel(model));
      ed.container->setAutoFillBackground(true);
      if (model == 1) {
         m_editors[0].container->setPalette(paletteForModel(0));
         m_editors[0].container->setAutoFillBackground(true);
      }
   }

   bool writable = m_dataModel->isModelWritable(model);
   ed.transCommentText = new FormWidget(QString(), true);
   ed.transCommentText->setEditingEnabled(writable);
   ed.transCommentText->setHideWhenEmpty(!writable);
   ed.transCommentText->setWhatsThis(tr("Enter comments for your own records."
           " They have no effect on the translated applications.") );

   ed.transCommentText->getEditor()->installEventFilter(this);
   ed.transCommentText->getEditor()->setVisualizeWhitespace(m_visualizeWhitespace);

   connect(ed.transCommentText, SIGNAL(selectionChanged(QTextEdit *)), SLOT(selectionChanged(QTextEdit *)));
   connect(ed.transCommentText, SIGNAL(textChanged(QTextEdit *)),      SLOT(emitTranslatorCommentChanged(QTextEdit *)));
   connect(ed.transCommentText, SIGNAL(textChanged(QTextEdit *)),      SLOT(resetHoverSelection()));
   connect(ed.transCommentText, SIGNAL(cursorPositionChanged()),       SLOT(resetHoverSelection()));

   fixTabOrder();

   QBoxLayout *box = new QVBoxLayout(ed.container);
   box->setMargin(5);
   box->addWidget(ed.transCommentText);
   box->addSpacing(ed.transCommentText->getEditor()->fontMetrics().height() / 2);
   m_layout->addWidget(ed.container);
   setTargetLanguage(model);
}

void MessageEditor::allModelsDeleted()
{
   for (const MessageEditorData & med : m_editors) {
      med.container->deleteLater();
   }

   m_editors.clear();
   m_currentModel = -1;
   // Do not emit activeModelChanged() - the main window will refresh anyway
   m_currentNumerus = -1;
   showNothing();
}

void MessageEditor::messageModelDeleted(int model)
{
   m_editors[model].container->deleteLater();
   m_editors.removeAt(model);
   if (model <= m_currentModel) {
      if (model < m_currentModel || m_currentModel == m_editors.size()) {
         --m_currentModel;
      }

      // Do not emit activeModelChanged() - the main window will refresh anyway
      if (m_currentModel >= 0) {
         if (m_currentNumerus >= m_editors[m_currentModel].transTexts.size()) {
            m_currentNumerus = m_editors[m_currentModel].transTexts.size() - 1;
         }

         activeEditor()->setFocus();
      } else {
         m_currentNumerus = -1;
      }
   }

   if (m_editors.size() == 1) {
      m_editors[0].container->setAutoFillBackground(false);
   } else {
      for (int i = model; i < m_editors.size(); ++i) {
         m_editors[i].container->setPalette(paletteForModel(i));
      }
   }
}

void MessageEditor::addPluralForm(int model, const QString &label, bool writable)
{
   FormMultiWidget *transEditor = new FormMultiWidget(label);
   connect(transEditor, SIGNAL(editorCreated(QTextEdit *)), SLOT(editorCreated(QTextEdit *)));
   transEditor->setEditingEnabled(writable);
   transEditor->setHideWhenEmpty(!writable);

   if (!m_editors[model].transTexts.isEmpty()) {
      transEditor->setVisible(false);
   }

   transEditor->setMultiEnabled(m_lengthVariants);
   static_cast<QBoxLayout *>(m_editors[model].container->layout())->insertWidget(
      m_editors[model].transTexts.count(), transEditor);

   connect(transEditor, SIGNAL(selectionChanged(QTextEdit *)),
           SLOT(selectionChanged(QTextEdit *)));
   connect(transEditor, SIGNAL(textChanged(QTextEdit *)),
           SLOT(emitTranslationChanged(QTextEdit *)));
   connect(transEditor, SIGNAL(textChanged(QTextEdit *)), SLOT(resetHoverSelection()));
   connect(transEditor, SIGNAL(cursorPositionChanged()), SLOT(resetHoverSelection()));

   m_editors[model].transTexts << transEditor;
}

void MessageEditor::editorCreated(QTextEdit *te)
{
   QFont font;
   font.setPointSize(static_cast<int>(m_fontSize));
   FormMultiWidget *snd = static_cast<FormMultiWidget *>(sender());

   for (int model = 0; ; ++model) {
      MessageEditorData med = m_editors.at(model);
      med.transCommentText->getEditor()->setFont(font);

      if (med.transTexts.contains(snd)) {
            te->setFont(font);

            te->installEventFilter(this);

            if (m_visualizeWhitespace) {
                QTextOption option = te->document()->defaultTextOption();

                option.setFlags(option.flags()
                                | QTextOption::ShowLineAndParagraphSeparators
                                | QTextOption::ShowTabsAndSpaces);
                te->document()->setDefaultTextOption(option);
            }

         fixTabOrder();
         return;
      }
   }
}

void MessageEditor::editorDestroyed()
{
   if (m_selectionHolder == sender()) {
      resetSelection();
   }
}

void MessageEditor::fixTabOrder()
{
   m_tabOrderTimer.start(0);
}

void MessageEditor::reallyFixTabOrder()
{
   QWidget *prev = this;
   for (const MessageEditorData & med :  m_editors) {
      for (FormMultiWidget * fmw : med.transTexts) {

         for (QTextEdit * te : fmw->getEditors()) {
            setTabOrder(prev, te);
            prev = te;
         }
      }

      QTextEdit *te = med.transCommentText->getEditor();
      setTabOrder(prev, te);
      prev = te;
   }
}

/*! internal
    Returns all translations for an item.
    The number of translations is dependent on if we have a plural form or not.
    If we don't have a plural form, then this should only contain one item.
    Otherwise it will contain the number of numerus forms for the particular language.
*/
QStringList MessageEditor::translations(int model) const
{
   QStringList translations;
   for (int i = 0; i < m_editors[model].transTexts.count() &&
         m_editors[model].transTexts.at(i)->isVisible(); ++i) {
      translations << m_editors[model].transTexts[i]->getTranslation();
   }

   return translations;
}

static void clearSelection(QTextEdit *t)
{
   bool oldBlockState = t->blockSignals(true);
   QTextCursor c = t->textCursor();
   c.clearSelection();
   t->setTextCursor(c);
   t->blockSignals(oldBlockState);
}

void MessageEditor::selectionChanged(QTextEdit *te)
{
   if (te != m_selectionHolder) {
      if (m_selectionHolder) {
         clearSelection(m_selectionHolder);
         disconnect(this, SLOT(editorDestroyed()));
      }

      m_selectionHolder = (te->textCursor().hasSelection() ? te : nullptr);

      if (FormatTextEdit *fte = qobject_cast<FormatTextEdit *>(m_selectionHolder)) {
         connect(fte, SIGNAL(editorDestroyed()), SLOT(editorDestroyed()));
      }

      updateCanCutCopy();
   }
}

void MessageEditor::resetHoverSelection()
{
   if (m_selectionHolder && (m_selectionHolder == m_source->getEditor() || m_selectionHolder == m_pluralSource->getEditor())) {
      resetSelection();
   }
}

void MessageEditor::resetSelection()
{
   if (m_selectionHolder) {
      clearSelection(m_selectionHolder);
      disconnect(this, SLOT(editorDestroyed()));
      m_selectionHolder = nullptr;
      updateCanCutCopy();
   }
}

void MessageEditor::activeModelAndNumerus(int *model, int *numerus) const
{
   for (int j = 0; j < m_editors.count(); ++j) {

      for (int i = 0; i < m_editors[j].transTexts.count(); ++i) {

         for (QTextEdit * te : m_editors[j].transTexts[i]->getEditors()) {
            if (m_focusWidget == te) {
               *model = j;
               *numerus = i;
               return;
            }
         }
      }

      if (m_focusWidget == m_editors[j].transCommentText->getEditor()) {
         *model = j;
         *numerus = -1;
         return;
      }
   }

   *model   = -1;
   *numerus = -1;
}

QTextEdit *MessageEditor::activeTranslation() const
{
   if (m_currentNumerus < 0) {
      return nullptr;
   }

   const QList<FormatTextEdit *> &editors = m_editors[m_currentModel].transTexts[m_currentNumerus]->getEditors();

   for (QTextEdit * te : editors) {
      if (te->hasFocus()) {
         return te;
      }
   }

   return editors.first();
}

QTextEdit *MessageEditor::activeOr1stTranslation() const
{
   if (m_currentNumerus < 0) {
      for (int i = 0; i < m_editors.size(); ++i)
         if (m_editors[i].container->isVisible()
               && !m_editors[i].transTexts.first()->getEditors().first()->isReadOnly()) {
            return m_editors[i].transTexts.first()->getEditors().first();
         }

      return nullptr;
   }

   return activeTranslation();
}

QTextEdit *MessageEditor::activeTransComment() const
{
   if (m_currentModel < 0 || m_currentNumerus >= 0) {
      return nullptr;
   }

   return m_editors[m_currentModel].transCommentText->getEditor();
}

QTextEdit *MessageEditor::activeEditor() const
{
   if (QTextEdit *te = activeTransComment()) {
      return te;
   }
   return activeTranslation();
}

QTextEdit *MessageEditor::activeOr1stEditor() const
{
   if (QTextEdit *te = activeTransComment()) {
      return te;
   }
   return activeOr1stTranslation();
}

void MessageEditor::setTargetLanguage(int model)
{
   const QStringList &numerusForms = m_dataModel->model(model)->numerusForms();
   const QString &langLocalized = m_dataModel->model(model)->localizedLanguage();

   for (int i = 0; i < numerusForms.count(); ++i) {
      const QString &label = tr("%1 translation (%2)").formatArgs(langLocalized, numerusForms[i]);

      if (!i) {
         m_editors[model].firstForm = label;
      }

      if (i >= m_editors[model].transTexts.count()) {
         addPluralForm(model, label, m_dataModel->isModelWritable(model));
      } else {
         m_editors[model].transTexts[i]->setLabel(label);
      }

      m_editors[model].transTexts[i]->setVisible(!i || m_editors[model].pluralEditMode);
      m_editors[model].transTexts[i]->setWhatsThis(
         tr("This is where you can enter or modify"
            " the translation of the above source text.") );
   }

   for (int j = m_editors[model].transTexts.count() - numerusForms.count(); j > 0; --j) {
      delete m_editors[model].transTexts.takeLast();
   }

   m_editors[model].invariantForm = tr("%1 translation").formatArg(langLocalized);
   m_editors[model].transCommentText->setLabel(tr("%1 translator comments").formatArg(langLocalized));
}

MessageEditorData *MessageEditor::modelForWidget(const QObject *o)
{
   for (int j = 0; j < m_editors.count(); ++j) {
      for (int i = 0; i < m_editors[j].transTexts.count(); ++i) {

         for (QTextEdit * item : m_editors[j].transTexts[i]->getEditors()) {
            if (item == o) {
               return &m_editors[j];
            }
         }
      }

      if (m_editors[j].transCommentText->getEditor() == o) {
         return &m_editors[j];
      }
   }

   return nullptr;
}

bool MessageEditor::eventFilter(QObject *o, QEvent *e)
{
   // handle copying from the source
   if (e->type() == QEvent::ShortcutOverride) {
      QKeyEvent *ke = static_cast<QKeyEvent *>(e);

      if (ke->modifiers() & Qt::ControlModifier) {
         if (ke->key() == Qt::Key_C) {
            if (m_source->getEditor()->textCursor().hasSelection()) {
               m_source->getEditor()->copy();
               return true;
            }
            if (m_pluralSource->getEditor()->textCursor().hasSelection()) {
               m_pluralSource->getEditor()->copy();
               return true;
            }
         } else if (ke->key() == Qt::Key_A) {
            return true;
         }
      }

   } else if (e->type() == QEvent::KeyPress) {
      // Ctrl-Tab is still passed through to the textedit and causes a tab to be inserted.
      QKeyEvent *ke = static_cast<QKeyEvent *>(e);

      if (ke->key() == Qt::Key_Tab && ! (ke->modifiers() & Qt::ControlModifier)) {
         focusNextChild();
         return true;
      }

   } else if (e->type() == QEvent::FocusIn) {
      QWidget *widget = static_cast<QWidget *>(o);
      if (widget != m_focusWidget) {
         trackFocus(widget);
      }
   }

   return QScrollArea::eventFilter(o, e);
}

void MessageEditor::grabFocus(QWidget *widget)
{
   if (widget != m_focusWidget) {
      widget->setFocus();
      trackFocus(widget);
   }
}

void MessageEditor::trackFocus(QWidget *widget)
{
   m_focusWidget = widget;

   int model, numerus;
   activeModelAndNumerus(&model, &numerus);
   if (model != m_currentModel || numerus != m_currentNumerus) {
      resetSelection();
      m_currentModel = model;
      m_currentNumerus = numerus;
      emit activeModelChanged(activeModel());
      updateBeginFromSource();
      updateUndoRedo();
      updateCanPaste();
   }
}

void MessageEditor::showNothing()
{
   m_source->clearTranslation();
   m_pluralSource->clearTranslation();
   m_commentText->clearTranslation();
   for (int j = 0; j < m_editors.count(); ++j) {
      setEditingEnabled(j, false);

      for (FormMultiWidget * widget : m_editors[j].transTexts) {
         widget->clearTranslation();
      }

      m_editors[j].transCommentText->clearTranslation();
   }
   emit pasteAvailable(false);
   updateBeginFromSource();
   updateUndoRedo();
}

void MessageEditor::showMessage(const MultiDataIndex &index)
{
   m_currentIndex = index;
   bool hadMsg    = false;

   for (int j = 0; j < m_editors.size(); ++j) {

      MessageEditorData &ed = m_editors[j];
      MessageItem *msgCargo = m_dataModel->getMessageItem(index, j);

      if (! msgCargo) {
         ed.container->hide();
         continue;
      }
      ed.container->show();

      if (! hadMsg) {

         // Source text form
         m_source->setTranslation(msgCargo->text());
         m_pluralSource->setTranslation(msgCargo->pluralText());

         // Use location from first non-obsolete message
         if (! msgCargo->fileName().isEmpty()) {
            QString toolTip = tr("'%1'\nLine: %2").formatArgs(msgCargo->fileName(), QString::number(msgCargo->lineNumber()));
            m_source->setToolTip(toolTip);

         } else {
            m_source->setToolTip("");
         }

         // Comment field
         QString commentText = msgCargo->comment().simplified();

         if (! msgCargo->extraComment().isEmpty()) {
            if (! commentText.isEmpty()) {
               commentText += "\n";
            }
            commentText += msgCargo->extraComment().simplified();
         }

         m_commentText->setTranslation(commentText);

         hadMsg = true;
      }

      setEditingEnabled(j, m_dataModel->isModelWritable(j)
                        && msgCargo->message().type() != TranslatorMessage::Type::Obsolete
                        && msgCargo->message().type() != TranslatorMessage::Type::Vanished);

      // Translation label
      ed.pluralEditMode = msgCargo->translations().count() > 1;
      ed.transTexts.first()->setLabel(ed.pluralEditMode ? ed.firstForm : ed.invariantForm);

      // Translation forms
      if (msgCargo->text().isEmpty() && ! msgCargo->context().isEmpty()) {
         for (int i = 0; i < ed.transTexts.size(); ++i) {
            ed.transTexts.at(i)->setVisible(false);
         }

      } else {
         QStringList normalizedTranslations = m_dataModel->model(j)->normalizedTranslations(*msgCargo);

         for (int i = 0; i < ed.transTexts.size(); ++i) {
            bool shouldShow = (i < normalizedTranslations.count());

            if (shouldShow) {
               setTranslationNumerus(j, normalizedTranslations.at(i), i);
            } else {
               setTranslationNumerus(j, QString(), i);
            }

            ed.transTexts.at(i)->setVisible(i == 0 || shouldShow);
         }
      }

      ed.transCommentText->setTranslation(msgCargo->translatorComment().trimmed(), false);
   }

   updateUndoRedo();
}

void MessageEditor::setTranslationNumerus(int model, const QString &translation, int numerus)
{
   MessageEditorData &ed = m_editors[model];

   if (numerus >= ed.transTexts.count()) {
      numerus = 0;
   }

   FormMultiWidget *transForm = ed.transTexts[numerus];
   transForm->setTranslation(translation, false);

   updateBeginFromSource();
}

void MessageEditor::setTranslation(int model, const QString &translation)
{
   int numerus;

   if (m_currentNumerus < 0) {
      numerus = 0;

   } else {
      model   = m_currentModel;
      numerus = m_currentNumerus;
   }

   FormMultiWidget *transForm = m_editors[model].transTexts[numerus];
   transForm->getEditors().first()->setFocus();
   transForm->setTranslation(translation, true);

   updateBeginFromSource();
}

void MessageEditor::setEditingEnabled(int model, bool enabled)
{
   MessageEditorData &ed = m_editors[model];

   for (FormMultiWidget * widget : ed.transTexts) {
      widget->setEditingEnabled(enabled);
   }

   ed.transCommentText->setEditingEnabled(enabled);

   updateCanPaste();
}

void MessageEditor::setLengthVariants(bool on)
{
   m_lengthVariants = on;
   for (const MessageEditorData & ed : m_editors)  {
      for (FormMultiWidget * widget : ed.transTexts) {
         widget->setMultiEnabled(on);
      }
   }
}

void MessageEditor::undo()
{
   activeEditor()->document()->undo();
}

void MessageEditor::redo()
{
   activeEditor()->document()->redo();
}

void MessageEditor::updateUndoRedo()
{
   bool newUndoAvail = false;
   bool newRedoAvail = false;
   if (QTextEdit *te = activeEditor()) {
      QTextDocument *doc = te->document();
      newUndoAvail = doc->isUndoAvailable();
      newRedoAvail = doc->isRedoAvailable();
   }

   if (newUndoAvail != m_undoAvail) {
      m_undoAvail = newUndoAvail;
      emit undoAvailable(newUndoAvail);
   }

   if (newRedoAvail != m_redoAvail) {
      m_redoAvail = newRedoAvail;
      emit redoAvailable(newRedoAvail);
   }
}

void MessageEditor::cut()
{
   m_selectionHolder->cut();
}

void MessageEditor::copy()
{
   m_selectionHolder->copy();
}

void MessageEditor::updateCanCutCopy()
{
   bool newCopyState = false;
   bool newCutState = false;

   if (m_selectionHolder) {
      newCopyState = true;
      newCutState = !m_selectionHolder->isReadOnly();
   }

   if (newCopyState != m_copyAvail) {
      m_copyAvail = newCopyState;
      emit copyAvailable(m_copyAvail);
   }

   if (newCutState != m_cutAvail) {
      m_cutAvail = newCutState;
      emit cutAvailable(m_cutAvail);
   }
}

void MessageEditor::paste()
{
   activeEditor()->paste();
}

void MessageEditor::updateCanPaste()
{
   QTextEdit *te;
   emit pasteAvailable(!m_clipboardEmpty && (te = activeEditor()) && !te->isReadOnly());
}

void MessageEditor::clipboardChanged()
{
   // this is expensive, so move it out of the common path in updateCanPaste
   m_clipboardEmpty = qApp->clipboard()->text().isEmpty();
   updateCanPaste();
}

void MessageEditor::selectAll()
{
   // make sure we don't select the selection of a translator textedit,
   // if we really want the source text editor to be selected.
   QTextEdit *te;

   if ((te = m_source->getEditor())->underMouse()
         || (te = m_pluralSource->getEditor())->underMouse()
         || ((te = activeEditor()) && te->hasFocus())) {
      te->selectAll();
   }
}

void MessageEditor::emitTranslationChanged(QTextEdit *widget)
{
   grabFocus(widget); // DND proofness
   updateBeginFromSource();
   updateUndoRedo();
   emit translationChanged(translations(m_currentModel));
}

void MessageEditor::emitTranslatorCommentChanged(QTextEdit *widget)
{
   grabFocus(widget); // DND proofness
   updateUndoRedo();
   emit translatorCommentChanged(m_editors[m_currentModel].transCommentText->getTranslation());
}

void MessageEditor::updateBeginFromSource()
{
   bool overwrite = false;

   if (QTextEdit *activeEditor = activeTranslation()) {
      overwrite = !activeEditor->isReadOnly() && activeEditor->toPlainText().trimmed().isEmpty();
   }

   emit beginFromSourceAvailable(overwrite);
}

void MessageEditor::beginFromSource()
{
   MessageItem *msgCargo = m_dataModel->getMessageItem(m_currentIndex, m_currentModel);

   setTranslation(m_currentModel, m_currentNumerus > 0 &&
               ! msgCargo->pluralText().isEmpty() ? msgCargo->pluralText() : msgCargo->text());
}

void MessageEditor::setEditorFocus()
{
   if (! widget()->hasFocus())
      if (QTextEdit *activeEditor = activeOr1stEditor()) {
         activeEditor->setFocus();
      }
}

void MessageEditor::setEditorFocusModel(int model)
{
   if (m_currentModel != model) {

      if (model < 0) {
         resetSelection();

         m_currentNumerus = -1;
         m_currentModel   = -1;
         m_focusWidget    = nullptr;

         emit activeModelChanged(activeModel());
         updateBeginFromSource();
         updateUndoRedo();
         updateCanPaste();

      } else {
         m_editors[model].transTexts.first()->getEditors().first()->setFocus();
      }
   }
}

bool MessageEditor::focusNextUnfinished(int start)
{
   for (int j = start; j < m_editors.count(); ++j) {

      if (m_dataModel->isModelWritable(j)) {
         MessageItem *msgCargo = m_dataModel->getMessageItem(m_currentIndex, j);

         if (msgCargo != nullptr) {
            if (msgCargo->type() == TranslatorMessage::Type::Unfinished) {
               m_editors[j].transTexts.first()->getEditors().first()->setFocus();
               return true;
            }
         }
      }
   }

   return false;
}

void MessageEditor::setUnfinishedEditorFocus()
{
   focusNextUnfinished(0);
}

bool MessageEditor::focusNextUnfinished()
{
   return focusNextUnfinished(m_currentModel + 1);
}

void MessageEditor::setVisualizeWhitespace(bool value)
{
   m_visualizeWhitespace = value;
   m_source->getEditor()->setVisualizeWhitespace(value);
   m_pluralSource->getEditor()->setVisualizeWhitespace(value);
   m_commentText->getEditor()->setVisualizeWhitespace(value);

   for (const MessageEditorData &med : m_editors) {
      med.transCommentText->getEditor()->setVisualizeWhitespace(value);

      for (FormMultiWidget *widget : med.transTexts) {
         for (FormatTextEdit *te : widget->getEditors()) {
             te->setVisualizeWhitespace(value);
         }
      }
   }
}

void MessageEditor::setFontSize(const float fontSize)
{
    if (m_fontSize != fontSize) {
        m_fontSize = fontSize;
        applyFontSize();
    }
}

float MessageEditor::fontSize()
{
    return m_fontSize;
}

void MessageEditor::applyFontSize()
{
    QFont font;
    font.setPointSize(static_cast<int>(m_fontSize));

    m_source->getEditor()->setFont(font);
    m_pluralSource->getEditor()->setFont(font);
    m_commentText->getEditor()->setFont(font);

    for (MessageEditorData med : m_editors) {
        for (int i = 0; i < med.transTexts.count(); ++i) {
            for (QTextEdit *te : med.transTexts[i]->getEditors()) {
                te->setFont(font);
            }
        }
        med.transCommentText->getEditor()->setFont(font);
    }
}

void MessageEditor::increaseFontSize()
{
   if (m_fontSize >= 32) {
      return;
   }

   m_fontSize *= 1.2f;
   applyFontSize();
}

void MessageEditor::decreaseFontSize()
{
    if (m_fontSize > 8) {
       m_fontSize /= 1.2f;
       applyFontSize();
    }
}

void MessageEditor::resetFontSize()
{
    m_fontSize = font().pointSize();
    applyFontSize();
}
