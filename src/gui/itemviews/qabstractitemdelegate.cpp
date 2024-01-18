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

#include <qabstractitemdelegate.h>

#ifndef QT_NO_ITEMVIEWS

#include <qabstractitemmodel.h>
#include <qabstractitemview.h>
#include <qfontmetrics.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qevent.h>
#include <qstring.h>
#include <qdebug.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qplaintextedit.h>
#include <qapplication.h>
#include <qtextengine_p.h>
#include <qabstractitemdelegate_p.h>

#include <qplatform_integration.h>
#include <qplatform_drag.h>
#include <qguiapplication_p.h>
#include <qdnd_p.h>


QAbstractItemDelegate::QAbstractItemDelegate(QObject *parent)
   : QObject(parent), d_ptr(new QAbstractItemDelegatePrivate)
{
   d_ptr->q_ptr = this;
}

QAbstractItemDelegate::QAbstractItemDelegate(QAbstractItemDelegatePrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QAbstractItemDelegate::~QAbstractItemDelegate()
{
}

QWidget *QAbstractItemDelegate::createEditor(QWidget *,
   const QStyleOptionViewItem &, const QModelIndex &) const
{
   return nullptr;
}

void QAbstractItemDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
  (void) index;

   editor->deleteLater();
}

void QAbstractItemDelegate::setEditorData(QWidget *, const QModelIndex &) const
{
   // does nothing
}

void QAbstractItemDelegate::setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const
{
   // does nothing
}

void QAbstractItemDelegate::updateEditorGeometry(QWidget *,
   const QStyleOptionViewItem &,
   const QModelIndex &) const
{
   // does nothing
}


bool QAbstractItemDelegate::editorEvent(QEvent *,
   QAbstractItemModel *,
   const QStyleOptionViewItem &,
   const QModelIndex &)
{
   return false;
}



QString QAbstractItemDelegate::elidedText(const QFontMetrics &fontMetrics, int width,
   Qt::TextElideMode mode, const QString &text)
{
   return fontMetrics.elidedText(text, mode, width);
}



bool QAbstractItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
   const QStyleOptionViewItem &option, const QModelIndex &index)
{
   Q_D(QAbstractItemDelegate);

   if (!event || !view) {
      return false;
   }

   switch (event->type()) {

#ifndef QT_NO_TOOLTIP
      case QEvent::ToolTip: {
         QHelpEvent *he = static_cast<QHelpEvent *>(event);
         const int precision = inherits("QItemDelegate") ? 10 : 6; // keep in sync with DBL_DIG in qitemdelegate.cpp
         const QString tooltip = d->textForRole(Qt::ToolTipRole, index.data(Qt::ToolTipRole), option.locale, precision);

         if (!tooltip.isEmpty()) {
            QToolTip::showText(he->globalPos(), tooltip, view);
            return true;
         }
         break;
      }
#endif

#ifndef QT_NO_WHATSTHIS
      case QEvent::QueryWhatsThis: {
         if (index.data(Qt::WhatsThisRole).isValid()) {
            return true;
         }
         break;
      }

      case QEvent::WhatsThis: {
         QHelpEvent *he = static_cast<QHelpEvent *>(event);
         const int precision = inherits("QItemDelegate") ? 10 : 6; // keep in sync with DBL_DIG in qitemdelegate.cpp
         const QString whatsthis = d->textForRole(Qt::WhatsThisRole, index.data(Qt::WhatsThisRole), option.locale, precision);

         if (!whatsthis.isEmpty()) {
            QWhatsThis::showText(he->globalPos(), whatsthis, view);
            return true;
         }
         break;
      }
#endif

      default:
         break;
   }

   return false;
}

QVector<int> QAbstractItemDelegate::paintingRoles() const
{
   return QVector<int>();
}

QAbstractItemDelegatePrivate::QAbstractItemDelegatePrivate()
{
}

static bool editorHandlesKeyEvent(QWidget *editor, const QKeyEvent *event)
{
#ifndef QT_NO_TEXTEDIT
   if (qobject_cast<QTextEdit *>(editor) || qobject_cast<QPlainTextEdit *>(editor)) {
      switch (event->key()) {
         case Qt::Key_Tab:
         case Qt::Key_Backtab:
         case Qt::Key_Enter:
         case Qt::Key_Return:
            return true;
         default:
            break;
      }
   }
#endif // QT_NO_TEXTEDIT

   return false;
}

bool QAbstractItemDelegatePrivate::editorEventFilter(QObject *object, QEvent *event)
{
   Q_Q(QAbstractItemDelegate);

   QWidget *editor = qobject_cast<QWidget *>(object);
   if (!editor) {
      return false;
   }
   if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (editorHandlesKeyEvent(editor, keyEvent)) {
         return false;
      }

      if (keyEvent->matches(QKeySequence::Cancel)) {
         // don't commit data
         emit q->closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
         return true;
      }

      switch (keyEvent->key()) {
         case Qt::Key_Tab:
            if (tryFixup(editor)) {
               emit q->commitData(editor);
               emit q->closeEditor(editor, QAbstractItemDelegate::EditNextItem);
            }
            return true;
         case Qt::Key_Backtab:
            if (tryFixup(editor)) {
               emit q->commitData(editor);
               emit q->closeEditor(editor, QAbstractItemDelegate::EditPreviousItem);
            }
            return true;
         case Qt::Key_Enter:
         case Qt::Key_Return:
            // We want the editor to be able to process the key press before
            // committing the data (e.g. so it can do validation/fixup of the input)

            if (! tryFixup(editor)) {
               return true;
            }

            QMetaObject::invokeMethod(q, "_q_commitDataAndCloseEditor", Qt::QueuedConnection, Q_ARG(QWidget *, editor));
            return false;

         default:
            return false;
      }
   } else if (event->type() == QEvent::FocusOut || (event->type() == QEvent::Hide && editor->isWindow())) {
      //the Hide event will take care of he editors that are in fact complete dialogs
      if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
         QWidget *w = QApplication::focusWidget();
         while (w) { // don't worry about focus changes internally in the editor
            if (w == editor) {
               return false;
            }
            w = w->parentWidget();
         }
#ifndef QT_NO_DRAGANDDROP
         // The window may lose focus during an drag operation.
         // i.e when dragging involves the taskbar on Windows.
         QPlatformDrag *platformDrag = QGuiApplicationPrivate::instance()->platformIntegration()->drag();
         if (platformDrag && platformDrag->currentDrag()) {
            return false;
         }
#endif
         if (tryFixup(editor)) {
            emit q->commitData(editor);
         }

         emit q->closeEditor(editor, QAbstractItemDelegate::NoHint);
      }
   } else if (event->type() == QEvent::ShortcutOverride) {
      if (static_cast<QKeyEvent *>(event)->matches(QKeySequence::Cancel)) {
         event->accept();
         return true;
      }
   }
   return false;
}

bool QAbstractItemDelegatePrivate::tryFixup(QWidget *editor)
{
#ifndef QT_NO_LINEEDIT
   if (QLineEdit *e = qobject_cast<QLineEdit *>(editor)) {
      if (!e->hasAcceptableInput()) {
         if (const QValidator *validator = e->validator()) {
            QString text = e->text();
            validator->fixup(text);
            e->setText(text);
         }
         return e->hasAcceptableInput();
      }
   }
#endif // QT_NO_LINEEDIT

   return true;
}
QString QAbstractItemDelegatePrivate::textForRole(Qt::ItemDataRole role, const QVariant &value, const QLocale &locale,
   int precision) const
{
   const QLocale::FormatType formatType = (role == Qt::DisplayRole) ? QLocale::ShortFormat : QLocale::LongFormat;
   QString text;

   switch (value.userType()) {
      case QVariant::Float:
         text = locale.toString(value.toFloat());
         break;

      case QVariant::Double:
         text = locale.toString(value.toDouble(), 'g', precision);
         break;

      case QVariant::Int:
      case QVariant::LongLong:
         text = locale.toString(value.toLongLong());
         break;

      case QVariant::UInt:
      case QVariant::ULongLong:
         text = locale.toString(value.toULongLong());
         break;

      case QVariant::Date:
         text = locale.toString(value.toDate(), formatType);
         break;

      case QVariant::Time:
         text = locale.toString(value.toTime(), formatType);
         break;

      case QVariant::DateTime: {
         const QDateTime dateTime = value.toDateTime();
         text = locale.toString(dateTime.date(), formatType) + QChar(' ')
                  + locale.toString(dateTime.time(), formatType);
         break;
      }

      default:
         text = value.toString();
         if (role == Qt::DisplayRole) {
            text.replace('\n', QChar(QChar::LineSeparator));
         }
         break;
   }

   return text;
}

void QAbstractItemDelegatePrivate::_q_commitDataAndCloseEditor(QWidget *editor)
{
   Q_Q(QAbstractItemDelegate);
   emit q->commitData(editor);
   emit q->closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
}

void QAbstractItemDelegate::_q_commitDataAndCloseEditor(QWidget *editor)
{
   Q_D(QAbstractItemDelegate);
   d->_q_commitDataAndCloseEditor(editor);
}

#endif // QT_NO_ITEMVIEWS
