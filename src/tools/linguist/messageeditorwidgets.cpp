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

#include "messageeditorwidgets.h"
#include "messagehighlighter.h"

#include <translator.h>

#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollArea>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QToolButton>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE

ExpandingTextEdit::ExpandingTextEdit(QWidget *parent)
   : QTextEdit(parent)
{
   setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding));

   setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

   QAbstractTextDocumentLayout *docLayout = document()->documentLayout();
   connect(docLayout, SIGNAL(documentSizeChanged(QSizeF)), SLOT(updateHeight(QSizeF)));
   connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(reallyEnsureCursorVisible()));

   m_minimumHeight = qRound(docLayout->documentSize().height()) + frameWidth() * 2;
}

void ExpandingTextEdit::updateHeight(const QSizeF &documentSize)
{
   m_minimumHeight = qRound(documentSize.height()) + frameWidth() * 2;
   updateGeometry();
}

QSize ExpandingTextEdit::sizeHint() const
{
   return QSize(100, m_minimumHeight);
}

QSize ExpandingTextEdit::minimumSizeHint() const
{
   return QSize(100, m_minimumHeight);
}

void ExpandingTextEdit::reallyEnsureCursorVisible()
{
   QObject *ancestor = parent();
   while (ancestor) {
      QScrollArea *scrollArea = qobject_cast<QScrollArea *>(ancestor);
      if (scrollArea &&
            (scrollArea->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff &&
             scrollArea->horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)) {
         const QRect &r = cursorRect();
         const QPoint &c = mapTo(scrollArea->widget(), r.center());
         scrollArea->ensureVisible(c.x(), c.y());
         break;
      }
      ancestor = ancestor->parent();
   }
}

FormatTextEdit::FormatTextEdit(QWidget *parent)
   : ExpandingTextEdit(parent)
{
   setLineWrapMode(QTextEdit::WidgetWidth);
   setAcceptRichText(false);
   QTextOption option = document()->defaultTextOption();
   option.setFlags(option.flags()
                   | QTextOption::ShowLineAndParagraphSeparators
                   | QTextOption::ShowTabsAndSpaces);
   document()->setDefaultTextOption(option);

   // Do not set different background if disabled
   QPalette p = palette();
   p.setColor(QPalette::Disabled, QPalette::Base, p.color(QPalette::Active, QPalette::Base));
   setPalette(p);

   setEditable(true);

   m_highlighter = new MessageHighlighter(this);
}

FormatTextEdit::~FormatTextEdit()
{
   emit editorDestroyed();
}

void FormatTextEdit::setEditable(bool editable)
{
   // save default frame style
   static int framed = frameStyle();
   static Qt::FocusPolicy defaultFocus = focusPolicy();

   if (editable) {
      setFrameStyle(framed);
      setFocusPolicy(defaultFocus);
   } else {
      setFrameStyle(QFrame::NoFrame | QFrame::Plain);
      setFocusPolicy(Qt::NoFocus);
   }

   setReadOnly(!editable);
}

void FormatTextEdit::setPlainText(const QString &text, bool userAction)
{
   if (!userAction) {
      // Prevent contentsChanged signal
      bool oldBlockState = blockSignals(true);
      document()->setUndoRedoEnabled(false);
      ExpandingTextEdit::setPlainText(text);
      // highlighter is out of sync because of blocked signals
      m_highlighter->rehighlight();
      document()->setUndoRedoEnabled(true);
      blockSignals(oldBlockState);
   } else {
      ExpandingTextEdit::setPlainText(text);
   }
}

FormWidget::FormWidget(const QString &label, bool isEditable, QWidget *parent)
   : QWidget(parent),
     m_hideWhenEmpty(false)
{
   QVBoxLayout *layout = new QVBoxLayout;
   layout->setMargin(0);

   m_label = new QLabel(this);
   QFont fnt;
   fnt.setBold(true);
   m_label->setFont(fnt);
   m_label->setText(label);
   layout->addWidget(m_label);

   m_editor = new FormatTextEdit(this);
   m_editor->setEditable(isEditable);
   //m_textEdit->setWhatsThis(tr("This area shows text from an auxillary translation."));
   layout->addWidget(m_editor);

   setLayout(layout);

   connect(m_editor, SIGNAL(textChanged()), SLOT(slotTextChanged()));
   connect(m_editor, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
   connect(m_editor, SIGNAL(cursorPositionChanged()), SIGNAL(cursorPositionChanged()));
}

void FormWidget::slotTextChanged()
{
   emit textChanged(m_editor);
}

void FormWidget::slotSelectionChanged()
{
   emit selectionChanged(m_editor);
}

void FormWidget::setTranslation(const QString &text, bool userAction)
{
   m_editor->setPlainText(text, userAction);
   if (m_hideWhenEmpty) {
      setHidden(text.isEmpty());
   }
}

void FormWidget::setEditingEnabled(bool enable)
{
   // Use read-only state so that the text can still be copied
   m_editor->setReadOnly(!enable);
   m_label->setEnabled(enable);
}


class ButtonWrapper : public QWidget
{
   // no Q_OBJECT: no need to, and don't want the useless moc file

 public:
   ButtonWrapper(QWidget *wrapee, QWidget *relator) : m_wrapee(wrapee) {
      setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
      QBoxLayout *box = new QVBoxLayout;
      box->setMargin(0);
      setLayout(box);
      box->addWidget(wrapee, 0, Qt::AlignBottom);
      if (relator) {
         relator->installEventFilter(this);
      }
   }

 protected:
   virtual bool eventFilter(QObject *object, QEvent *event) {
      if (event->type() == QEvent::Resize) {
         QWidget *relator = static_cast<QWidget *>(object);
         setFixedHeight((relator->height() + layout()->spacing() + m_wrapee->height()) / 2);
      }
      return false;
   }

 private:
   QWidget *m_wrapee;
};

FormMultiWidget::FormMultiWidget(const QString &label, QWidget *parent)
   : QWidget(parent),
     m_hideWhenEmpty(false),
     m_multiEnabled(false),
     m_plusIcon(QIcon(QLatin1String(":/images/plus.png"))),  // make static
     m_minusIcon(QIcon(QLatin1String(":/images/minus.png")))
{
   m_label = new QLabel(this);
   QFont fnt;
   fnt.setBold(true);
   m_label->setFont(fnt);
   m_label->setText(label);

   m_plusButtons.append(
      new ButtonWrapper(makeButton(m_plusIcon, SLOT(plusButtonClicked())), 0));
}

QAbstractButton *FormMultiWidget::makeButton(const QIcon &icon, const char *slot)
{
   QAbstractButton *btn = new QToolButton(this);
   btn->setIcon(icon);
   btn->setFixedSize(icon.availableSizes().first() /* + something */);
   btn->setFocusPolicy(Qt::NoFocus);
   connect(btn, SIGNAL(clicked()), slot);
   return btn;
}

void FormMultiWidget::addEditor(int idx)
{
   FormatTextEdit *editor = new FormatTextEdit(this);
   m_editors.insert(idx, editor);

   m_minusButtons.insert(idx, makeButton(m_minusIcon, SLOT(minusButtonClicked())));
   m_plusButtons.insert(idx + 1,
                        new ButtonWrapper(makeButton(m_plusIcon, SLOT(plusButtonClicked())), editor));

   connect(editor, SIGNAL(textChanged()), SLOT(slotTextChanged()));
   connect(editor, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
   connect(editor, SIGNAL(cursorPositionChanged()), SIGNAL(cursorPositionChanged()));
   editor->installEventFilter(this);

   emit editorCreated(editor);
}

bool FormMultiWidget::eventFilter(QObject *watched, QEvent *event)
{
   int i = 0;
   while (m_editors.at(i) != watched)
      if (++i >= m_editors.count()) { // Happens when deleting an editor
         return false;
      }
   if (event->type() == QEvent::FocusOut) {
      m_minusButtons.at(i)->setToolTip(QString());
      m_plusButtons.at(i)->setToolTip(QString());
      m_plusButtons.at(i + 1)->setToolTip(QString());
   } else if (event->type() == QEvent::FocusIn) {
      m_minusButtons.at(i)->setToolTip(/*: translate, but don't change */ tr("Alt+Delete"));
      m_plusButtons.at(i)->setToolTip(/*: translate, but don't change */ tr("Shift+Alt+Insert"));
      m_plusButtons.at(i + 1)->setToolTip(/*: translate, but don't change */ tr("Alt+Insert"));
   } else if (event->type() == QEvent::KeyPress) {
      QKeyEvent *ke = static_cast<QKeyEvent *>(event);
      if (ke->modifiers() & Qt::AltModifier) {
         if (ke->key() == Qt::Key_Delete) {
            deleteEditor(i);
            return true;
         } else if (ke->key() == Qt::Key_Insert) {
            if (!(ke->modifiers() & Qt::ShiftModifier)) {
               ++i;
            }
            insertEditor(i);
            return true;
         }
      }
   }
   return false;
}

void FormMultiWidget::updateLayout()
{
   delete layout();

   QGridLayout *layout = new QGridLayout;
   layout->setMargin(0);
   setLayout(layout);

   bool variants = m_multiEnabled && m_label->isEnabled();

   layout->addWidget(m_label, 0, 0, 1, variants ? 3 : 1);

   for (int i = 0; i < m_plusButtons.count(); ++i) {
      if (variants) {
         layout->addWidget(m_plusButtons.at(i), 1 + i * 2, 0, 2, 1, Qt::AlignTop);
      }
      m_plusButtons.at(i)->setVisible(variants);
   }
   for (int j = 0; j < m_minusButtons.count(); ++j) {
      if (variants) {
         layout->addWidget(m_minusButtons.at(j), 2 + j * 2, 2, 2, 1, Qt::AlignVCenter);
      }
      m_minusButtons.at(j)->setVisible(variants);
   }
   for (int k = 0; k < m_editors.count(); ++k) {
      layout->addWidget(m_editors.at(k), 2 + k * 2, variants ? 1 : 0, 2, 1, Qt::AlignVCenter);
   }

   updateGeometry();
}

void FormMultiWidget::slotTextChanged()
{
   emit textChanged(static_cast<QTextEdit *>(sender()));
}

void FormMultiWidget::slotSelectionChanged()
{
   emit selectionChanged(static_cast<QTextEdit *>(sender()));
}

void FormMultiWidget::setTranslation(const QString &text, bool userAction)
{
   QStringList texts = text.split(QChar(Translator::BinaryVariantSeparator), QString::KeepEmptyParts);

   while (m_editors.count() > texts.count()) {
      delete m_minusButtons.takeLast();
      delete m_plusButtons.takeLast();
      delete m_editors.takeLast();
   }
   while (m_editors.count() < texts.count()) {
      addEditor(m_editors.count());
   }
   updateLayout();

   for (int i = 0; i < texts.count(); ++i)
      // XXX this will emit n textChanged signals
   {
      m_editors.at(i)->setPlainText(texts.at(i), userAction);
   }

   if (m_hideWhenEmpty) {
      setHidden(text.isEmpty());
   }
}

QString FormMultiWidget::getTranslation() const
{
   QString ret;
   for (int i = 0; i < m_editors.count(); ++i) {
      if (i) {
         ret += QChar(Translator::BinaryVariantSeparator);
      }
      ret += m_editors.at(i)->toPlainText();
   }
   return ret;
}

void FormMultiWidget::setEditingEnabled(bool enable)
{
   // Use read-only state so that the text can still be copied
   for (int i = 0; i < m_editors.count(); ++i) {
      m_editors.at(i)->setReadOnly(!enable);
   }
   m_label->setEnabled(enable);
   if (m_multiEnabled) {
      updateLayout();
   }
}

void FormMultiWidget::setMultiEnabled(bool enable)
{
   m_multiEnabled = enable;
   if (m_label->isEnabled()) {
      updateLayout();
   }
}

void FormMultiWidget::minusButtonClicked()
{
   int i = 0;
   while (m_minusButtons.at(i) != sender()) {
      ++i;
   }
   deleteEditor(i);
}

void FormMultiWidget::plusButtonClicked()
{
   QWidget *btn = static_cast<QAbstractButton *>(sender())->parentWidget();
   int i = 0;
   while (m_plusButtons.at(i) != btn) {
      ++i;
   }
   insertEditor(i);
}

void FormMultiWidget::deleteEditor(int idx)
{
   if (m_editors.count() == 1) {
      // Don't just clear(), so the undo history is not lost
      QTextCursor c = m_editors.first()->textCursor();
      c.select(QTextCursor::Document);
      c.removeSelectedText();
   } else {
      if (!m_editors.at(idx)->toPlainText().isEmpty()) {
         if (QMessageBox::question(topLevelWidget(), tr("Confirmation - Qt Linguist"),
                                   tr("Delete non-empty length variant?"),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
               != QMessageBox::Yes) {
            return;
         }
      }
      delete m_editors.takeAt(idx);
      delete m_minusButtons.takeAt(idx);
      delete m_plusButtons.takeAt(idx + 1);
      updateLayout();
      emit textChanged(m_editors.at((m_editors.count() == idx) ? idx - 1 : idx));
   }
}

void FormMultiWidget::insertEditor(int idx)
{
   addEditor(idx);
   updateLayout();
   emit textChanged(m_editors.at(idx));
}

QT_END_NAMESPACE
