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

#include <qaccessiblewidget_simple_p.h>

#include <qabstractbutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qstatusbar.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>
#include <qmenu.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlineedit_p.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtextdocument.h>
#include <qwindow.h>
#include <qvarlengtharray.h>

#ifdef Q_OS_DARWIN
#include <qfocusframe.h>
#endif

#include <qwindowcontainer_p.h>

#ifndef QT_NO_ACCESSIBILITY

extern QList<QWidget *> childWidgets(const QWidget *widget);

QString qt_accStripAmp(const QString &text);
QString qt_accHotKey(const QString &text);

QAccessibleButton::QAccessibleButton(QWidget *w)
   : QAccessibleWidget(w)
{
   Q_ASSERT(button());

   // FIXME: The checkable state of the button is dynamic,
   // while we only update the controlling signal once :(
   if (button()->isCheckable()) {
      addControllingSignal(QLatin1String("toggled(bool)"));
   } else {
      addControllingSignal(QLatin1String("clicked()"));
   }
}

QAbstractButton *QAccessibleButton::button() const
{
   return qobject_cast<QAbstractButton *>(object());
}

QString QAccessibleButton::text(QAccessible::Text t) const
{
   QString str;

   switch (t) {
      case QAccessible::Accelerator: {
#ifndef QT_NO_SHORTCUT
         QPushButton *pb = qobject_cast<QPushButton *>(object());
         if (pb && pb->isDefault()) {
            str = QKeySequence(Qt::Key_Enter).toString(QKeySequence::NativeText);
         }
#endif

         if (str.isEmpty()) {
            str = qt_accHotKey(button()->text());
         }
      }
      break;

      case QAccessible::Name:
         str = widget()->accessibleName();
         if (str.isEmpty()) {
            str = qt_accStripAmp(button()->text());
         }
         break;

      default:
         break;
   }

   if (str.isEmpty()) {
      str = QAccessibleWidget::text(t);
   }

   return str;
}

QAccessible::State QAccessibleButton::state() const
{
   QAccessible::State state = QAccessibleWidget::state();

   QAbstractButton *b = button();
   QCheckBox *cb = qobject_cast<QCheckBox *>(b);
   if (b->isCheckable()) {
      state.checkable = true;
   }
   if (b->isChecked()) {
      state.checked = true;
   } else if (cb && cb->checkState() == Qt::PartiallyChecked) {
      state.checkStateMixed = true;
   }
   if (b->isDown()) {
      state.pressed = true;
   }
   QPushButton *pb = qobject_cast<QPushButton *>(b);
   if (pb) {
      if (pb->isDefault()) {
         state.defaultButton = true;
      }
#ifndef QT_NO_MENU
      if (pb->menu()) {
         state.hasPopup = true;
      }
#endif
   }

   return state;
}

QRect QAccessibleButton::rect() const
{
   QAbstractButton *ab = button();

   if (!ab->isVisible()) {
      return QRect();
   }

   if (QCheckBox *cb = qobject_cast<QCheckBox *>(ab)) {
      QPoint wpos = cb->mapToGlobal(QPoint(0, 0));
      QStyleOptionButton opt;
      cb->initStyleOption(&opt);
      return cb->style()->subElementRect(QStyle::SE_CheckBoxClickRect, &opt, cb).translated(wpos);

   } else if (QRadioButton *rb = qobject_cast<QRadioButton *>(ab)) {
      QPoint wpos = rb->mapToGlobal(QPoint(0, 0));
      QStyleOptionButton opt;
      rb->initStyleOption(&opt);
      return rb->style()->subElementRect(QStyle::SE_RadioButtonClickRect, &opt, rb).translated(wpos);
   }

   return QAccessibleWidget::rect();
}

QAccessible::Role QAccessibleButton::role() const
{
   QAbstractButton *ab = button();

#ifndef QT_NO_MENU
   if (QPushButton *pb = qobject_cast<QPushButton *>(ab)) {
      if (pb->menu()) {
         return QAccessible::ButtonMenu;
      }
   }
#endif

   if (ab->isCheckable()) {
      return ab->autoExclusive() ? QAccessible::RadioButton : QAccessible::CheckBox;
   }

   return QAccessible::Button;
}

QStringList QAccessibleButton::actionNames() const
{
   QStringList names;
   if (widget()->isEnabled()) {
      switch (role()) {
         case QAccessible::ButtonMenu:
            names << showMenuAction();
            break;
         case QAccessible::RadioButton:
            names << toggleAction();
            break;
         default:
            if (button()->isCheckable()) {
               names <<  toggleAction();
            } else {
               names << pressAction();
            }
            break;
      }
   }
   names << QAccessibleWidget::actionNames();
   return names;
}

void QAccessibleButton::doAction(const QString &actionName)
{
   if (!widget()->isEnabled()) {
      return;
   }
   if (actionName == pressAction() ||
      actionName == showMenuAction()) {
#ifndef QT_NO_MENU
      QPushButton *pb = qobject_cast<QPushButton *>(object());
      if (pb && pb->menu()) {
         pb->showMenu();
      } else
#endif
         button()->animateClick();
   } else if (actionName == toggleAction()) {
      button()->toggle();
   } else {
      QAccessibleWidget::doAction(actionName);
   }
}

QStringList QAccessibleButton::keyBindingsForAction(const QString &actionName) const
{
   if (actionName == pressAction()) {
#ifndef QT_NO_SHORTCUT
      return QStringList() << button()->shortcut().toString();
#endif
   }

   return QStringList();
}

#ifndef QT_NO_TOOLBUTTON

QAccessibleToolButton::QAccessibleToolButton(QWidget *w)
   : QAccessibleButton(w)
{
   Q_ASSERT(toolButton());
}

QToolButton *QAccessibleToolButton::toolButton() const
{
   return qobject_cast<QToolButton *>(object());
}

bool QAccessibleToolButton::isSplitButton() const
{
#ifndef QT_NO_MENU
   return toolButton()->menu() && toolButton()->popupMode() == QToolButton::MenuButtonPopup;
#else
   return false;
#endif
}

QAccessible::State QAccessibleToolButton::state() const
{
   QAccessible::State st = QAccessibleButton::state();

  if (toolButton()->autoRaise()) {
      st.hotTracked = true;
   }

#ifndef QT_NO_MENU
   if (toolButton()->menu()) {
      st.hasPopup = true;
   }
#endif

   return st;
}

int QAccessibleToolButton::childCount() const
{
   return isSplitButton() ? 1 : 0;
}

QAccessible::Role QAccessibleToolButton::role() const
{
   QAbstractButton *ab = button();

#ifndef QT_NO_MENU
   QToolButton *tb = qobject_cast<QToolButton *>(ab);

   if (! tb->menu()) {
      return tb->isCheckable() ? QAccessible::CheckBox : QAccessible::Button;

   } else if (tb->popupMode() == QToolButton::DelayedPopup) {
      return QAccessible::ButtonDropDown;
   }
#endif

   return QAccessible::ButtonMenu;
}

QAccessibleInterface *QAccessibleToolButton::child(int index) const
{
#ifndef QT_NO_MENU
   if (index == 0 && toolButton()->menu()) {
      return QAccessible::queryAccessibleInterface(toolButton()->menu());
   }
#endif
   return nullptr;
}

QStringList QAccessibleToolButton::actionNames() const
{
   QStringList names;

   if (widget()->isEnabled()) {
      if (toolButton()->menu()) {
         names << showMenuAction();
      }

      if (toolButton()->popupMode() != QToolButton::InstantPopup) {
         names << QAccessibleButton::actionNames();
      }
   }

   return names;
}

void QAccessibleToolButton::doAction(const QString &actionName)
{
   if (! widget()->isEnabled()) {
      return;
   }

   if (actionName == pressAction()) {
      button()->click();
   } else if (actionName == showMenuAction()) {
      if (toolButton()->popupMode() != QToolButton::InstantPopup) {
         toolButton()->setDown(true);
#ifndef QT_NO_MENU
         toolButton()->showMenu();
#endif
      }
   } else {
      QAccessibleButton::doAction(actionName);
   }

}

#endif // QT_NO_TOOLBUTTON

QAccessibleDisplay::QAccessibleDisplay(QWidget *w, QAccessible::Role role)
   : QAccessibleWidget(w, role)
{
}

QAccessible::Role QAccessibleDisplay::role() const
{
   QLabel *l = qobject_cast<QLabel *>(object());

   if (l) {
      if (l->pixmap()) {
         return QAccessible::Graphic;
      }

#ifndef QT_NO_PICTURE
      if (l->picture()) {
         return QAccessible::Graphic;
      }

#endif

#ifndef QT_NO_MOVIE
      if (l->movie()) {
         return QAccessible::Animation;
      }
#endif

#ifndef QT_NO_PROGRESSBAR
   } else if (qobject_cast<QProgressBar *>(object())) {
      return QAccessible::ProgressBar;
#endif

   } else if (qobject_cast<QStatusBar *>(object())) {
      return QAccessible::StatusBar;
   }

   return QAccessibleWidget::role();
}

QString QAccessibleDisplay::text(QAccessible::Text t) const
{
   QString str;

   switch (t) {
      case QAccessible::Name:
         str = widget()->accessibleName();

         if (str.isEmpty()) {
            if (qobject_cast<QLabel *>(object())) {
               QLabel *label = qobject_cast<QLabel *>(object());
               str = label->text();

#ifndef QT_NO_TEXTHTMLPARSER
               if (label->textFormat() == Qt::RichText
                  || (label->textFormat() == Qt::AutoText && Qt::mightBeRichText(str))) {
                  QTextDocument doc;
                  doc.setHtml(str);
                  str = doc.toPlainText();
               }
#endif

               if (label->buddy()) {
                  str = qt_accStripAmp(str);
               }

#ifndef QT_NO_LCDNUMBER
            } else if (qobject_cast<QLCDNumber *>(object())) {
               QLCDNumber *l = qobject_cast<QLCDNumber *>(object());
               if (l->digitCount()) {
                  str = QString::number(l->value());
               } else {
                  str = QString::number(l->intValue());
               }
#endif

            } else if (qobject_cast<QStatusBar *>(object())) {
               return qobject_cast<QStatusBar *>(object())->currentMessage();
            }
         }
         break;

      case QAccessible::Value:
#ifndef QT_NO_PROGRESSBAR
         if (qobject_cast<QProgressBar *>(object())) {
            str = QString::number(qobject_cast<QProgressBar *>(object())->value());
         }
#endif
         break;
      default:
         break;
   }

   if (str.isEmpty()) {
      str = QAccessibleWidget::text(t);
   }
   return str;
}

QVector<QPair<QAccessibleInterface *, QAccessible::Relation>> QAccessibleDisplay::relations(QAccessible::Relation match ) const
{
   QVector<QPair<QAccessibleInterface *, QAccessible::Relation>> rels = QAccessibleWidget::relations(match);

   if (match & QAccessible::Labelled) {
      QVarLengthArray<QObject *, 4> relatedObjects;

#ifndef QT_NO_SHORTCUT
      if (QLabel *label = qobject_cast<QLabel *>(object())) {
         relatedObjects.append(label->buddy());
      }
#endif

      for (int i = 0; i < relatedObjects.count(); ++i) {
         const QAccessible::Relation rel = QAccessible::Labelled;
         QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(relatedObjects.at(i));

         if (iface) {
            rels.append(qMakePair(iface, rel));
         }
      }
   }

   return rels;
}

void *QAccessibleDisplay::interface_cast(QAccessible::InterfaceType t)
{
   if (t == QAccessible::ImageInterface) {
      return static_cast<QAccessibleImageInterface *>(this);
   }

   return QAccessibleWidget::interface_cast(t);
}

QString QAccessibleDisplay::imageDescription() const
{
#ifndef QT_NO_TOOLTIP
   return widget()->toolTip();
#else
   return QString::null;
#endif
}
QSize QAccessibleDisplay::imageSize() const
{
   QLabel *label = qobject_cast<QLabel *>(widget());

   if (! label) {
      return QSize();
   }

   const QPixmap *pixmap = label->pixmap();
   if (! pixmap) {
      return QSize();
   }

   return pixmap->size();
}

QPoint QAccessibleDisplay::imagePosition() const
{
   QLabel *label = qobject_cast<QLabel *>(widget());

   if (! label) {
      return QPoint();
   }
   const QPixmap *pixmap = label->pixmap();
   if (! pixmap) {
      return QPoint();
   }

   return QPoint(label->mapToGlobal(label->pos()));
}

#ifndef QT_NO_GROUPBOX
QAccessibleGroupBox::QAccessibleGroupBox(QWidget *w)
   : QAccessibleWidget(w)
{
}

QGroupBox *QAccessibleGroupBox::groupBox() const
{
   return static_cast<QGroupBox *>(widget());
}

QString QAccessibleGroupBox::text(QAccessible::Text t) const
{
   QString txt = QAccessibleWidget::text(t);

   if (txt.isEmpty()) {
      switch (t) {
         case QAccessible::Name:
            txt = qt_accStripAmp(groupBox()->title());
            break;

         case QAccessible::Description:
            txt = groupBox()->toolTip();
            break;

         case QAccessible::Accelerator:
            txt = qt_accHotKey(groupBox()->title());
            break;

         default:
            break;
      }
   }

   return txt;
}

QAccessible::State QAccessibleGroupBox::state() const
{
   QAccessible::State st = QAccessibleWidget::state();
   st.checkable = groupBox()->isCheckable();
   st.checked = groupBox()->isChecked();
   return st;
}

QAccessible::Role QAccessibleGroupBox::role() const
{
   return groupBox()->isCheckable() ? QAccessible::CheckBox : QAccessible::Grouping;
}

QVector<QPair<QAccessibleInterface *, QAccessible::Relation>> QAccessibleGroupBox::relations(QAccessible::Relation match) const
{
   QVector<QPair<QAccessibleInterface *, QAccessible::Relation>> rels = QAccessibleWidget::relations(match);

   if ((match & QAccessible::Labelled) && (!groupBox()->title().isEmpty())) {
      const QList<QWidget *> kids = childWidgets(widget());

      for (int i = 0; i < kids.count(); ++i) {
         QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(kids.at(i));

         if (iface) {
            rels.append(qMakePair(iface, QAccessible::Relation(QAccessible::Labelled)));
         }
      }
   }

   return rels;
}

QStringList QAccessibleGroupBox::actionNames() const
{
   QStringList actions = QAccessibleWidget::actionNames();

   if (groupBox()->isCheckable()) {
      actions.prepend(QAccessibleActionInterface::toggleAction());
   }

   return actions;
}

void QAccessibleGroupBox::doAction(const QString &actionName)
{
   if (actionName == QAccessibleActionInterface::toggleAction()) {
      groupBox()->setChecked(! groupBox()->isChecked());
   }
}

QStringList QAccessibleGroupBox::keyBindingsForAction(const QString &) const
{
   return QStringList();
}

#endif

#ifndef QT_NO_LINEEDIT

QAccessibleLineEdit::QAccessibleLineEdit(QWidget *w, const QString &name)
   : QAccessibleWidget(w, QAccessible::EditableText, name)
{
   addControllingSignal("textChanged(const QString&)");
   addControllingSignal("returnPressed()");
}

QLineEdit *QAccessibleLineEdit::lineEdit() const
{
   return qobject_cast<QLineEdit *>(object());
}

QString QAccessibleLineEdit::text(QAccessible::Text t) const
{
   QString str;

   switch (t) {
      case QAccessible::Value:
         if (lineEdit()->echoMode() == QLineEdit::Normal) {
            str = lineEdit()->text();
         } else if (lineEdit()->echoMode() != QLineEdit::NoEcho) {
            str = QString(lineEdit()->text().length(), QChar::fromLatin1('*'));
         }
         break;

      default:
         break;
   }

   if (str.isEmpty()) {
      str = QAccessibleWidget::text(t);
   }

   return str;
}

void QAccessibleLineEdit::setText(QAccessible::Text t, const QString &text)
{
   if (t != QAccessible::Value) {
      QAccessibleWidget::setText(t, text);
      return;
   }

   QString newText = text;
   if (lineEdit()->validator()) {
      int pos = 0;
      if (lineEdit()->validator()->validate(newText, pos) != QValidator::Acceptable) {
         return;
      }
   }

   lineEdit()->setText(newText);
}

QAccessible::State QAccessibleLineEdit::state() const
{
   QAccessible::State state = QAccessibleWidget::state();

   QLineEdit *l = lineEdit();

   if (l->isReadOnly()) {
      state.readOnly = true;
   } else {
      state.editable = true;
   }

   if (l->echoMode() != QLineEdit::Normal) {
      state.passwordEdit = true;
   }

   state.selectableText = true;
   return state;
}

void *QAccessibleLineEdit::interface_cast(QAccessible::InterfaceType t)
{
   if (t == QAccessible::TextInterface) {
      return static_cast<QAccessibleTextInterface *>(this);
   }

   if (t == QAccessible::EditableTextInterface) {
      return static_cast<QAccessibleEditableTextInterface *>(this);
   }

   return QAccessibleWidget::interface_cast(t);
}

void QAccessibleLineEdit::addSelection(int startOffset, int endOffset)
{
   setSelection(0, startOffset, endOffset);
}

QString QAccessibleLineEdit::attributes(int offset, int *startOffset, int *endOffset) const
{
   // QLineEdit doesn't have text attributes
   *startOffset = *endOffset = offset;
   return QString();
}

int QAccessibleLineEdit::cursorPosition() const
{
   return lineEdit()->cursorPosition();
}

QRect QAccessibleLineEdit::characterRect(int offset) const
{
   int x = lineEdit()->d_func()->control->cursorToX(offset);
   int y;
   lineEdit()->getTextMargins(nullptr, &y, nullptr, nullptr);
   QFontMetrics fm(lineEdit()->font());
   const QString ch = text(offset, offset + 1);

   if (ch.isEmpty()) {
      return QRect();
   }

   int w = fm.width(ch);
   int h = fm.height();

   QRect r(x, y, w, h);
   r.moveTo(lineEdit()->mapToGlobal(r.topLeft()));
   return r;
}

int QAccessibleLineEdit::selectionCount() const
{
   return lineEdit()->hasSelectedText() ? 1 : 0;
}

int QAccessibleLineEdit::offsetAtPoint(const QPoint &point) const
{
   QPoint p = lineEdit()->mapFromGlobal(point);

   return lineEdit()->cursorPositionAt(p);
}

void QAccessibleLineEdit::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
   *startOffset = 0;
   *endOffset   = 0;

   if (selectionIndex != 0) {
      return;
   }

   *startOffset = lineEdit()->selectionStart();
   *endOffset = *startOffset + lineEdit()->selectedText().count();
}

QString QAccessibleLineEdit::text(int startOffset, int endOffset) const
{
   if (startOffset > endOffset) {
      return QString();
   }

   if (lineEdit()->echoMode() != QLineEdit::Normal) {
      return QString();
   }

   return lineEdit()->text().mid(startOffset, endOffset - startOffset);
}

QString QAccessibleLineEdit::textBeforeOffset(int offset, QAccessible::TextBoundaryType boundaryType,
   int *startOffset, int *endOffset) const
{
   if (lineEdit()->echoMode() != QLineEdit::Normal) {
      *startOffset = *endOffset = -1;
      return QString();
   }

   if (offset == -2) {
      offset = cursorPosition();
   }

   return QAccessibleTextInterface::textBeforeOffset(offset, boundaryType, startOffset, endOffset);
}

QString QAccessibleLineEdit::textAfterOffset(int offset, QAccessible::TextBoundaryType boundaryType,
      int *startOffset, int *endOffset) const
{
   if (lineEdit()->echoMode() != QLineEdit::Normal) {
      *startOffset = *endOffset = -1;
      return QString();
   }

   if (offset == -2) {
      offset = cursorPosition();
   }

   return QAccessibleTextInterface::textAfterOffset(offset, boundaryType, startOffset, endOffset);
}

QString QAccessibleLineEdit::textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType,
      int *startOffset, int *endOffset) const
{
   if (lineEdit()->echoMode() != QLineEdit::Normal) {
      *startOffset = *endOffset = -1;
      return QString();
   }

   if (offset == -2) {
      offset = cursorPosition();
   }

   return QAccessibleTextInterface::textAtOffset(offset, boundaryType, startOffset, endOffset);
}

void QAccessibleLineEdit::removeSelection(int selectionIndex)
{
   if (selectionIndex != 0) {
      return;
   }

   lineEdit()->deselect();
}

void QAccessibleLineEdit::setCursorPosition(int position)
{
   lineEdit()->setCursorPosition(position);
}

void QAccessibleLineEdit::setSelection(int selectionIndex, int startOffset, int endOffset)
{
   if (selectionIndex != 0) {
      return;
   }

   lineEdit()->setSelection(startOffset, endOffset - startOffset);
}

int QAccessibleLineEdit::characterCount() const
{
   return lineEdit()->text().count();
}

void QAccessibleLineEdit::scrollToSubstring(int startIndex, int endIndex)
{
   lineEdit()->setCursorPosition(endIndex);
   lineEdit()->setCursorPosition(startIndex);
}

void QAccessibleLineEdit::deleteText(int startOffset, int endOffset)
{
   lineEdit()->setText(lineEdit()->text().remove(startOffset, endOffset - startOffset));
}

void QAccessibleLineEdit::insertText(int offset, const QString &text)
{
   lineEdit()->setText(lineEdit()->text().insert(offset, text));
}

void QAccessibleLineEdit::replaceText(int startOffset, int endOffset, const QString &text)
{
   lineEdit()->setText(lineEdit()->text().replace(startOffset, endOffset - startOffset, text));
}

#endif // QT_NO_LINEEDIT

#ifndef QT_NO_PROGRESSBAR
QAccessibleProgressBar::QAccessibleProgressBar(QWidget *o)
   : QAccessibleDisplay(o)
{
   Q_ASSERT(progressBar());
}

void *QAccessibleProgressBar::interface_cast(QAccessible::InterfaceType t)
{
   if (t == QAccessible::ValueInterface) {
      return static_cast<QAccessibleValueInterface *>(this);
   }
   return QAccessibleDisplay::interface_cast(t);
}

QVariant QAccessibleProgressBar::currentValue() const
{
   return progressBar()->value();
}

QVariant QAccessibleProgressBar::maximumValue() const
{
   return progressBar()->maximum();
}

QVariant QAccessibleProgressBar::minimumValue() const
{
   return progressBar()->minimum();
}

QVariant QAccessibleProgressBar::minimumStepSize() const
{
   // This is arbitrary since any value between min and max is valid.
   // Some screen readers (orca use it to calculate how many digits to display though,
   // so it makes sense to return a "sensible" value. Providing 100 increments seems ok.
   return (progressBar()->maximum() - progressBar()->minimum()) / 100.0;
}

QProgressBar *QAccessibleProgressBar::progressBar() const
{
   return qobject_cast<QProgressBar *>(object());
}
#endif

QAccessibleWindowContainer::QAccessibleWindowContainer(QWidget *w)
   : QAccessibleWidget(w)
{
}

int QAccessibleWindowContainer::childCount() const
{
   if (container()->containedWindow()) {
      return 1;
   }

   return 0;
}

int QAccessibleWindowContainer::indexOfChild(const QAccessibleInterface *child) const
{
   if (child->object() == container()->containedWindow()) {
      return 0;
   }

   return -1;
}

QAccessibleInterface *QAccessibleWindowContainer::child(int i) const
{
   if (i == 0) {
      return QAccessible::queryAccessibleInterface(container()->containedWindow());
   }

   return nullptr;
}

QWindowContainer *QAccessibleWindowContainer::container() const
{
   return static_cast<QWindowContainer *>(widget());
}

#endif // QT_NO_ACCESSIBILITY
