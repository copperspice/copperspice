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

#include <qkeysequenceedit.h>
#include <qkeysequenceedit_p.h>

#include <qboxlayout.h>
#include <qlineedit.h>

#ifndef QT_NO_KEYSEQUENCEEDIT

// assumed by the following code
static_assert(QKeySequencePrivate::MaxKeyCount == 4, "Key sequence may not have more than four characters");

void QKeySequenceEditPrivate::init()
{
   Q_Q(QKeySequenceEdit);

   lineEdit = new QLineEdit(q);
   lineEdit->setObjectName("qt_keysequenceedit_lineedit");
   keyNum = 0;
   prevKey = -1;
   releaseTimer = 0;

   QVBoxLayout *layout = new QVBoxLayout(q);
   layout->setContentsMargins(0, 0, 0, 0);
   layout->addWidget(lineEdit);

   key[0] = key[1] = key[2] = key[3] = 0;

   lineEdit->setFocusProxy(q);
   lineEdit->installEventFilter(q);
   resetState();

   q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   q->setFocusPolicy(Qt::StrongFocus);
   q->setAttribute(Qt::WA_MacShowFocusRect, true);
   q->setAttribute(Qt::WA_InputMethodEnabled, false);

   // TODO: add clear button
}

int QKeySequenceEditPrivate::translateModifiers(Qt::KeyboardModifiers state, const QString &text)
{
   int result = 0;
   // The shift modifier only counts when it is not used to type a symbol
   // that is only reachable using the shift key anyway

   if ((state & Qt::ShiftModifier) && (text.isEmpty() || ! text.at(0).isPrint() ||
         text.at(0).isLetterOrNumber() || text.at(0).isSpace())) {
      result |= Qt::ShiftModifier;
   }

   if (state & Qt::ControlModifier) {
      result |= Qt::ControlModifier;
   }
   if (state & Qt::MetaModifier) {
      result |= Qt::MetaModifier;
   }
   if (state & Qt::AltModifier) {
      result |= Qt::AltModifier;
   }
   return result;
}

void QKeySequenceEditPrivate::resetState()
{
   Q_Q(QKeySequenceEdit);

   if (releaseTimer) {
      q->killTimer(releaseTimer);
      releaseTimer = 0;
   }

   prevKey = -1;
   lineEdit->setText(keySequence.toString(QKeySequence::NativeText));
   lineEdit->setPlaceholderText(QKeySequenceEdit::tr("Press shortcut"));
}

void QKeySequenceEditPrivate::finishEditing()
{
   Q_Q(QKeySequenceEdit);

   resetState();
   emit q->keySequenceChanged(keySequence);
   emit q->editingFinished();
}

QKeySequenceEdit::QKeySequenceEdit(QWidget *parent) :
   QWidget(*new QKeySequenceEditPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QKeySequenceEdit);
   d->init();
}

QKeySequenceEdit::QKeySequenceEdit(const QKeySequence &keySequence, QWidget *parent) :
   QWidget(*new QKeySequenceEditPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QKeySequenceEdit);
   d->init();
   setKeySequence(keySequence);
}

// internal
QKeySequenceEdit::QKeySequenceEdit(QKeySequenceEditPrivate &dd, QWidget *parent, Qt::WindowFlags flags)
   : QWidget(dd, parent, flags)
{
   Q_D(QKeySequenceEdit);
   d->init();
}

QKeySequenceEdit::~QKeySequenceEdit()
{
}

QKeySequence QKeySequenceEdit::keySequence() const
{
   Q_D(const QKeySequenceEdit);

   return d->keySequence;
}

void QKeySequenceEdit::setKeySequence(const QKeySequence &keySequence)
{
   Q_D(QKeySequenceEdit);

   d->resetState();

   if (d->keySequence == keySequence) {
      return;
   }

   d->keySequence = keySequence;

   d->key[0] = d->key[1] = d->key[2] = d->key[3] = 0;
   d->keyNum = keySequence.count();
   for (int i = 0; i < d->keyNum; ++i) {
      d->key[i] = keySequence[i];
   }

   d->lineEdit->setText(keySequence.toString(QKeySequence::NativeText));

   emit keySequenceChanged(keySequence);
}

void QKeySequenceEdit::clear()
{
   setKeySequence(QKeySequence());
}

bool QKeySequenceEdit::event(QEvent *e)
{
   switch (e->type()) {
      case QEvent::Shortcut:
         return true;

      case QEvent::ShortcutOverride:
         e->accept();
         return true;

      default:
         break;
   }

   return QWidget::event(e);
}

void QKeySequenceEdit::keyPressEvent(QKeyEvent *e)
{
   Q_D(QKeySequenceEdit);

   int nextKey = e->key();

   if (d->prevKey == -1) {
      clear();
      d->prevKey = nextKey;
   }

   d->lineEdit->setPlaceholderText(QString());
   if (nextKey == Qt::Key_Control
      || nextKey == Qt::Key_Shift
      || nextKey == Qt::Key_Meta
      || nextKey == Qt::Key_Alt) {
      return;
   }

   QString selectedText = d->lineEdit->selectedText();
   if (!selectedText.isEmpty() && selectedText == d->lineEdit->text()) {
      clear();
      if (nextKey == Qt::Key_Backspace) {
         return;
      }
   }

   if (d->keyNum >= QKeySequencePrivate::MaxKeyCount) {
      return;
   }

   nextKey |= d->translateModifiers(e->modifiers(), e->text());

   d->key[d->keyNum] = nextKey;
   d->keyNum++;

   QKeySequence key(d->key[0], d->key[1], d->key[2], d->key[3]);
   d->keySequence = key;
   QString text = key.toString(QKeySequence::NativeText);
   if (d->keyNum < QKeySequencePrivate::MaxKeyCount) {
      //: This text is an "unfinished" shortcut, expands like "Ctrl+A, ..."
      text = tr("%1, ...").formatArg(text);
   }
   d->lineEdit->setText(text);
   e->accept();
}

void QKeySequenceEdit::keyReleaseEvent(QKeyEvent *e)
{
   Q_D(QKeySequenceEdit);

   if (d->prevKey == e->key()) {
      if (d->keyNum < QKeySequencePrivate::MaxKeyCount) {
         d->releaseTimer = startTimer(1000);
      } else {
         d->finishEditing();
      }
   }
   e->accept();
}

void QKeySequenceEdit::timerEvent(QTimerEvent *e)
{
   Q_D(QKeySequenceEdit);
   if (e->timerId() == d->releaseTimer) {
      d->finishEditing();
      return;
   }

   QWidget::timerEvent(e);
}

#endif // QT_NO_KEYSEQUENCEEDIT


