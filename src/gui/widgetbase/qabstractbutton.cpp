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

#include <qabstractbutton.h>

#include <qabstractitemview.h>
#include <qapplication.h>
#include <qaction.h>
#include <qbuttongroup.h>
#include <qevent.h>
#include <qpainter.h>
#include <qstyle.h>

#include <qabstractbutton_p.h>
#include <qbuttongroup_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <algorithm>

#define AUTO_REPEAT_DELAY  300
#define AUTO_REPEAT_INTERVAL 100

Q_GUI_EXPORT extern bool qt_tab_all_widgets();

QAbstractButtonPrivate::QAbstractButtonPrivate(QSizePolicy::ControlType type)
   :
#ifndef QT_NO_SHORTCUT
   shortcutId(0),
#endif

   checkable(false), checked(false), autoRepeat(false), autoExclusive(false),
   down(false), blockRefresh(false), pressed(false),

#ifndef QT_NO_BUTTONGROUP
   group(nullptr),
#endif

   autoRepeatDelay(AUTO_REPEAT_DELAY), autoRepeatInterval(AUTO_REPEAT_INTERVAL),
   controlType(type)
{
}

QList<QAbstractButton *>QAbstractButtonPrivate::queryButtonList() const
{
   Q_Q(const QAbstractButton);

#ifndef QT_NO_BUTTONGROUP

   if (group) {
      return group->d_func()->buttonList;
   }

#endif

   QList<QAbstractButton *> retval;
   auto parent = q->parent();

   if (parent == nullptr) {
      return retval;
   }

   retval = parent->findChildren<QAbstractButton *>();

   if (autoExclusive) {

      for (int i = retval.count() - 1; i >= 0; --i) {
         QAbstractButton *candidate = retval.at(i);

#ifndef QT_NO_BUTTONGROUP

         if (! candidate->autoExclusive() || candidate->group()) {

#else

         if (! candidate->autoExclusive()) {

#endif
            retval.removeAt(i);
         }
      }
   }

   return retval;
}

QAbstractButton *QAbstractButtonPrivate::queryCheckedButton() const
{
#ifndef QT_NO_BUTTONGROUP

   if (group) {
      return group->d_func()->checkedButton;
   }

#endif

   Q_Q(const QAbstractButton);
   QList<QAbstractButton *> buttonList = queryButtonList();

   if (! autoExclusive || buttonList.count() == 1) {
      // no group
      return nullptr;
   }

   for (int i = 0; i < buttonList.count(); ++i) {
      QAbstractButton *b = buttonList.at(i);

      if (b->d_func()->checked && b != q) {
         return b;
      }
   }

   return checked  ? const_cast<QAbstractButton *>(q) : nullptr;
}

void QAbstractButtonPrivate::notifyChecked()
{
#ifndef QT_NO_BUTTONGROUP
   Q_Q(QAbstractButton);

   if (group) {
      QAbstractButton *previous = group->d_func()->checkedButton;
      group->d_func()->checkedButton = q;

      if (group->d_func()->exclusive && previous && previous != q) {
         previous->nextCheckState();
      }
   } else
#endif

      if (autoExclusive) {
         if (QAbstractButton *b = queryCheckedButton()) {
            b->setChecked(false);
         }
      }
}

void QAbstractButtonPrivate::moveFocus(int key)
{
   QList<QAbstractButton *> buttonList = queryButtonList();

#ifndef QT_NO_BUTTONGROUP
   bool exclusive = group ? group->d_func()->exclusive : autoExclusive;
#else
   bool exclusive = autoExclusive;
#endif

   QWidget *f = QApplication::focusWidget();
   QAbstractButton *fb = qobject_cast<QAbstractButton *>(f);

   if (! fb || ! buttonList.contains(fb)) {
      return;
   }

   QAbstractButton *candidate = nullptr;
   int bestScore = -1;

   QRect target = f->rect().translated(f->mapToGlobal(QPoint(0, 0)));
   QPoint goal  = target.center();

   uint focus_flag = qt_tab_all_widgets() ? Qt::TabFocus : Qt::StrongFocus;

   for (int i = 0; i < buttonList.count(); ++i) {
      QAbstractButton *button = buttonList.at(i);

      if (button != f && button->window() == f->window() && button->isEnabled() && !button->isHidden() &&
            (autoExclusive || (button->focusPolicy() & focus_flag) == focus_flag)) {
         QRect buttonRect = button->rect().translated(button->mapToGlobal(QPoint(0, 0)));
         QPoint p = buttonRect.center();

         //Priority to widgets that overlap on the same coordinate.
         //In that case, the distance in the direction will be used as significant score,
         //take also in account orthogonal distance in case two widget are in the same distance.
         int score;

         if ((buttonRect.x() < target.right() && target.x() < buttonRect.right())
               && (key == Qt::Key_Up || key == Qt::Key_Down)) {
            //one item's is at the vertical of the other
            score = (qAbs(p.y() - goal.y()) << 16) + qAbs(p.x() - goal.x());

         } else if ((buttonRect.y() < target.bottom() && target.y() < buttonRect.bottom())
               && (key == Qt::Key_Left || key == Qt::Key_Right) ) {
            //one item's is at the horizontal of the other
            score = (qAbs(p.x() - goal.x()) << 16) + qAbs(p.y() - goal.y());

         } else {
            score = (1 << 30) + (p.y() - goal.y()) * (p.y() - goal.y()) + (p.x() - goal.x()) * (p.x() - goal.x());
         }

         if (score > bestScore && candidate) {
            continue;
         }

         switch (key) {
            case Qt::Key_Up:
               if (p.y() < goal.y()) {
                  candidate = button;
                  bestScore = score;
               }

               break;

            case Qt::Key_Down:
               if (p.y() > goal.y()) {
                  candidate = button;
                  bestScore = score;
               }

               break;

            case Qt::Key_Left:
               if (p.x() < goal.x()) {
                  candidate = button;
                  bestScore = score;
               }

               break;

            case Qt::Key_Right:
               if (p.x() > goal.x()) {
                  candidate = button;
                  bestScore = score;
               }

               break;
         }
      }
   }

   if (exclusive

#ifdef QT_KEYPAD_NAVIGATION
         && ! QApplication::keypadNavigationEnabled()
#endif
         && candidate && fb->d_func()->checked && candidate->d_func()->checkable) {
      candidate->click();
   }

   if (candidate) {
      if (key == Qt::Key_Up || key == Qt::Key_Left) {
         candidate->setFocus(Qt::BacktabFocusReason);
      } else {
         candidate->setFocus(Qt::TabFocusReason);
      }
   }
}

void QAbstractButtonPrivate::fixFocusPolicy()
{
   Q_Q(QAbstractButton);

#ifndef QT_NO_BUTTONGROUP
   if (! group && ! autoExclusive) {
#else
   if (! autoExclusive) {
#endif
      return;
   }

   QList<QAbstractButton *> buttonList = queryButtonList();

   for (int i = 0; i < buttonList.count(); ++i) {
      QAbstractButton *b = buttonList.at(i);

      if (!b->isCheckable()) {
         continue;
      }

      b->setFocusPolicy((Qt::FocusPolicy) ((b == q || !q->isCheckable())
         ? (b->focusPolicy() | Qt::TabFocus) : (b->focusPolicy() & ~Qt::TabFocus)));
   }
}

void QAbstractButtonPrivate::init()
{
   Q_Q(QAbstractButton);

   q->setFocusPolicy(Qt::FocusPolicy(q->style()->styleHint(QStyle::SH_Button_FocusPolicy)));
   q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed, controlType));
   q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
   q->setForegroundRole(QPalette::ButtonText);
   q->setBackgroundRole(QPalette::Button);
}

void QAbstractButtonPrivate::refresh()
{
   Q_Q(QAbstractButton);

   if (blockRefresh) {
      return;
   }

   q->update();
}

void QAbstractButtonPrivate::click()
{
   Q_Q(QAbstractButton);

   down         = false;
   blockRefresh = true;

   bool changeState = true;

   if (checked && queryCheckedButton() == q) {
      // the checked button of an exclusive or autoexclusive group cannot be unchecked

#ifndef QT_NO_BUTTONGROUP
      if (group ? group->d_func()->exclusive : autoExclusive) {
#else
      if (autoExclusive) {
#endif
         changeState = false;
      }
   }

   QPointer<QAbstractButton> guard(q);

   if (changeState) {
      q->nextCheckState();

      if (! guard) {
         return;
      }
   }

   blockRefresh = false;
   refresh();

   q->repaint(); //flush paint event before invoking potentially expensive operation
   QApplication::flush();

   if (guard) {
      emitReleased();
   }

   if (guard) {
      emitClicked();
   }
}

void QAbstractButtonPrivate::emitClicked()
{
   Q_Q(QAbstractButton);
   QPointer<QAbstractButton> guard(q);
   emit q->clicked(checked);

#ifndef QT_NO_BUTTONGROUP

   if (guard && group) {
      emit group->buttonClicked(group->id(q));

      if (guard && group) {
         emit group->buttonClicked(q);
      }
   }

#endif
}

void QAbstractButtonPrivate::emitPressed()
{
   Q_Q(QAbstractButton);
   QPointer<QAbstractButton> guard(q);
   emit q->pressed();

#ifndef QT_NO_BUTTONGROUP

   if (guard && group) {
      emit group->buttonPressed(group->id(q));

      if (guard && group) {
         emit group->buttonPressed(q);
      }
   }

#endif
}

void QAbstractButtonPrivate::emitReleased()
{
   Q_Q(QAbstractButton);
   QPointer<QAbstractButton> guard(q);
   emit q->released();

#ifndef QT_NO_BUTTONGROUP

   if (guard && group) {
      emit group->buttonReleased(group->id(q));

      if (guard && group) {
         emit group->buttonReleased(q);
      }
   }

#endif
}
void QAbstractButtonPrivate::emitToggled(bool checked)
{
   Q_Q(QAbstractButton);
   QPointer<QAbstractButton> guard(q);

   emit q->toggled(checked);

#ifndef QT_NO_BUTTONGROUP

   if (guard && group) {
      emit group->buttonToggled(group->id(q), checked);

      if (guard && group) {
         emit group->buttonToggled(q, checked);
      }
   }

#endif
}

QAbstractButton::QAbstractButton(QWidget *parent)
   : QWidget(*new QAbstractButtonPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QAbstractButton);
   d->init();
}

QAbstractButton::~QAbstractButton()
{
#ifndef QT_NO_BUTTONGROUP
   Q_D(QAbstractButton);

   if (d->group) {
      d->group->removeButton(this);
   }

#endif
}

QAbstractButton::QAbstractButton(QAbstractButtonPrivate &dd, QWidget *parent)
   : QWidget(dd, parent, Qt::EmptyFlag)
{
   Q_D(QAbstractButton);
   d->init();
}

void QAbstractButton::setText(const QString &text)
{
   Q_D(QAbstractButton);

   if (d->text == text) {
      return;
   }

   d->text = text;

#ifndef QT_NO_SHORTCUT
   QKeySequence newMnemonic = QKeySequence::mnemonic(text);
   setShortcut(newMnemonic);
#endif

   d->sizeHint = QSize();
   update();
   updateGeometry();

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleEvent event(this, QAccessible::NameChanged);
   QAccessible::updateAccessibility(&event);
#endif
}

QString QAbstractButton::text() const
{
   Q_D(const QAbstractButton);
   return d->text;
}

void QAbstractButton::setIcon(const QIcon &icon)
{
   Q_D(QAbstractButton);
   d->icon = icon;
   d->sizeHint = QSize();

   update();
   updateGeometry();
}

QIcon QAbstractButton::icon() const
{
   Q_D(const QAbstractButton);
   return d->icon;
}

#ifndef QT_NO_SHORTCUT

void QAbstractButton::setShortcut(const QKeySequence &key)
{
   Q_D(QAbstractButton);

   if (d->shortcutId != 0) {
      releaseShortcut(d->shortcutId);
   }

   d->shortcut = key;
   d->shortcutId = grabShortcut(key);
}

QKeySequence QAbstractButton::shortcut() const
{
   Q_D(const QAbstractButton);
   return d->shortcut;
}
#endif // QT_NO_SHORTCUT

void QAbstractButton::setCheckable(bool checkable)
{
   Q_D(QAbstractButton);

   if (d->checkable == checkable) {
      return;
   }

   d->checkable = checkable;
   d->checked = false;
}

bool QAbstractButton::isCheckable() const
{
   Q_D(const QAbstractButton);
   return d->checkable;
}

void QAbstractButton::setChecked(bool checked)
{
   Q_D(QAbstractButton);

   if (! d->checkable || d->checked == checked) {
      if (! d->blockRefresh) {
         checkStateSet();
      }

      return;
   }

   if (! checked && d->queryCheckedButton() == this) {
      // the checked button of an exclusive or autoexclusive group can not be  unchecked

#ifndef QT_NO_BUTTONGROUP
      if (d->group ? d->group->d_func()->exclusive : d->autoExclusive) {
         return;
      }

      if (d->group) {
         d->group->d_func()->detectCheckedButton();
      }

#else

      if (d->autoExclusive) {
         return;
      }

#endif
   }

   QPointer<QAbstractButton> guard(this);

   d->checked = checked;

   if (!d->blockRefresh) {
      checkStateSet();
   }

   d->refresh();

   if (guard && checked) {
      d->notifyChecked();
   }

   if (guard) {
      d->emitToggled(checked);
   }

#ifndef QT_NO_ACCESSIBILITY
   QAccessible::State s;
   s.checked = true;
   QAccessibleStateChangeEvent event(this, s);
   QAccessible::updateAccessibility(&event);
#endif
}

bool QAbstractButton::isChecked() const
{
   Q_D(const QAbstractButton);
   return d->checked;
}

void QAbstractButton::setDown(bool down)
{
   Q_D(QAbstractButton);

   if (d->down == down) {
      return;
   }

   d->down = down;
   d->refresh();

   if (d->autoRepeat && d->down) {
      d->repeatTimer.start(d->autoRepeatDelay, this);
   } else {
      d->repeatTimer.stop();
   }
}

bool QAbstractButton::isDown() const
{
   Q_D(const QAbstractButton);
   return d->down;
}

void QAbstractButton::setAutoRepeat(bool autoRepeat)
{
   Q_D(QAbstractButton);

   if (d->autoRepeat == autoRepeat) {
      return;
   }

   d->autoRepeat = autoRepeat;

   if (d->autoRepeat && d->down) {
      d->repeatTimer.start(d->autoRepeatDelay, this);
   } else {
      d->repeatTimer.stop();
   }
}

bool QAbstractButton::autoRepeat() const
{
   Q_D(const QAbstractButton);
   return d->autoRepeat;
}

void QAbstractButton::setAutoRepeatDelay(int autoRepeatDelay)
{
   Q_D(QAbstractButton);
   d->autoRepeatDelay = autoRepeatDelay;
}

int QAbstractButton::autoRepeatDelay() const
{
   Q_D(const QAbstractButton);
   return d->autoRepeatDelay;
}

void QAbstractButton::setAutoRepeatInterval(int autoRepeatInterval)
{
   Q_D(QAbstractButton);
   d->autoRepeatInterval = autoRepeatInterval;
}

int QAbstractButton::autoRepeatInterval() const
{
   Q_D(const QAbstractButton);
   return d->autoRepeatInterval;
}

void QAbstractButton::setAutoExclusive(bool autoExclusive)
{
   Q_D(QAbstractButton);
   d->autoExclusive = autoExclusive;
}

bool QAbstractButton::autoExclusive() const
{
   Q_D(const QAbstractButton);
   return d->autoExclusive;
}

#ifndef QT_NO_BUTTONGROUP

QButtonGroup *QAbstractButton::group() const
{
   Q_D(const QAbstractButton);
   return d->group;
}
#endif

void QAbstractButton::animateClick(int msec)
{
   if (!isEnabled()) {
      return;
   }

   Q_D(QAbstractButton);

   if (d->checkable && focusPolicy() & Qt::ClickFocus) {
      setFocus();
   }

   setDown(true);
   repaint(); //flush paint event before invoking potentially expensive operation
   QApplication::flush();

   if (!d->animateTimer.isActive()) {
      d->emitPressed();
   }

   d->animateTimer.start(msec, this);
}

void QAbstractButton::click()
{
   if (! isEnabled()) {
      return;
   }

   Q_D(QAbstractButton);
   QPointer<QAbstractButton> guard(this);
   d->down = true;
   d->emitPressed();

   if (guard) {
      d->down = false;
      nextCheckState();

      if (guard) {
         d->emitReleased();
      }

      if (guard) {
         d->emitClicked();
      }
   }
}

void QAbstractButton::toggle()
{
   Q_D(QAbstractButton);
   setChecked(!d->checked);
}

void QAbstractButton::checkStateSet()
{
}

void QAbstractButton::nextCheckState()
{
   if (isCheckable()) {
      setChecked(!isChecked());
   }
}

bool QAbstractButton::hitButton(const QPoint &pos) const
{
   return rect().contains(pos);
}

bool QAbstractButton::event(QEvent *e)
{
   // as opposed to other widgets, disabled buttons accept mouse
   // events. This avoids surprising click-through scenarios

   if (! isEnabled()) {
      switch (e->type()) {
         case QEvent::TabletPress:
         case QEvent::TabletRelease:
         case QEvent::TabletMove:
         case QEvent::MouseButtonPress:
         case QEvent::MouseButtonRelease:
         case QEvent::MouseButtonDblClick:
         case QEvent::MouseMove:
         case QEvent::HoverMove:
         case QEvent::HoverEnter:
         case QEvent::HoverLeave:
         case QEvent::ContextMenu:
#ifndef QT_NO_WHEELEVENT
         case QEvent::Wheel:
#endif
            return true;

         default:
            break;
      }
   }

#ifndef QT_NO_SHORTCUT

   if (e->type() == QEvent::Shortcut) {
      Q_D(QAbstractButton);
      QShortcutEvent *se = static_cast<QShortcutEvent *>(e);

      if (d->shortcutId != se->shortcutId()) {
         return false;
      }

      if (! se->isAmbiguous()) {
         if (!d->animateTimer.isActive()) {
            animateClick();
         }

      } else {
         if (focusPolicy() != Qt::NoFocus) {
            setFocus(Qt::ShortcutFocusReason);
         }

         window()->setAttribute(Qt::WA_KeyboardFocusChange);
      }

      return true;
   }

#endif
   return QWidget::event(e);
}

void QAbstractButton::mousePressEvent(QMouseEvent *e)
{
   Q_D(QAbstractButton);

   if (e->button() != Qt::LeftButton) {
      e->ignore();
      return;
   }

   if (hitButton(e->pos())) {
      setDown(true);
      d->pressed = true;
      repaint(); //flush paint event before invoking potentially expensive operation
      QApplication::flush();
      d->emitPressed();
      e->accept();
   } else {
      e->ignore();
   }
}

void QAbstractButton::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QAbstractButton);
   d->pressed = false;

   if (e->button() != Qt::LeftButton) {
      e->ignore();
      return;
   }

   if (! d->down) {
      e->ignore();
      return;
   }

   if (hitButton(e->pos())) {
      d->repeatTimer.stop();
      d->click();
      e->accept();
   } else {
      setDown(false);
      e->ignore();
   }
}

void QAbstractButton::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QAbstractButton);

   if (! (e->buttons() & Qt::LeftButton) || !d->pressed) {
      e->ignore();
      return;
   }

   if (hitButton(e->pos()) != d->down) {
      setDown(! d->down);
      repaint(); //flush paint event before invoking potentially expensive operation
      QApplication::flush();

      if (d->down) {
         d->emitPressed();
      } else {
         d->emitReleased();
      }

      e->accept();

   } else if (! hitButton(e->pos())) {
      e->ignore();
   }
}

void QAbstractButton::keyPressEvent(QKeyEvent *e)
{
   Q_D(QAbstractButton);
   bool next = true;

   switch (e->key()) {
      case Qt::Key_Enter:
      case Qt::Key_Return:
         e->ignore();
         break;

      case Qt::Key_Select:
      case Qt::Key_Space:
         if (!e->isAutoRepeat()) {
            setDown(true);
            repaint(); //flush paint event before invoking potentially expensive operation
            QApplication::flush();
            d->emitPressed();
         }

         break;

      case Qt::Key_Up:
         next = false;
         [[fallthrough]];

      case Qt::Key_Left:
      case Qt::Key_Right:
      case Qt::Key_Down: {

#ifdef QT_KEYPAD_NAVIGATION

         if ((QApplication::keypadNavigationEnabled() && (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right))
               || (! QApplication::navigationMode() == Qt::NavigationModeKeypadDirectional
               || (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down))) {
            e->ignore();
            return;
         }

#endif
         QWidget *pw = parentWidget();

         if (d->autoExclusive

#ifndef QT_NO_BUTTONGROUP
               || d->group
#endif

#ifndef QT_NO_ITEMVIEWS
               || (pw && qobject_cast<QAbstractItemView *>(pw->parentWidget()))
#endif
         ) {

            d->moveFocus(e->key());

            if (hasFocus()) {
               // nothing happend, propagate
               e->ignore();
            }

         } else {
            QWidget *w = pw ? pw : this;
            bool reverse = (w->layoutDirection() == Qt::RightToLeft);

            if ((e->key() == Qt::Key_Left && !reverse) || (e->key() == Qt::Key_Right && reverse)) {
               next = false;
            }

            focusNextPrevChild(next);
         }

         break;
      }

      default:
         if (e->matches(QKeySequence::Cancel) && d->down) {
            setDown(false);
            repaint(); //flush paint event before invoking potentially expensive operation
            QApplication::flush();
            d->emitReleased();
            return;
         }

         e->ignore();
   }
}

void QAbstractButton::keyReleaseEvent(QKeyEvent *e)
{
   Q_D(QAbstractButton);

   if (! e->isAutoRepeat()) {
      d->repeatTimer.stop();
   }

   switch (e->key()) {
      case Qt::Key_Select:
      case Qt::Key_Space:
         if (!e->isAutoRepeat() && d->down) {
            d->click();
         }

         break;

      default:
         e->ignore();
   }
}

void QAbstractButton::timerEvent(QTimerEvent *e)
{
   Q_D(QAbstractButton);

   if (e->timerId() == d->repeatTimer.timerId()) {
      d->repeatTimer.start(d->autoRepeatInterval, this);

      if (d->down) {
         QPointer<QAbstractButton> guard(this);
         nextCheckState();

         if (guard) {
            d->emitReleased();
         }

         if (guard) {
            d->emitClicked();
         }

         if (guard) {
            d->emitPressed();
         }
      }

   } else if (e->timerId() == d->animateTimer.timerId()) {
      d->animateTimer.stop();
      d->click();
   }
}

void QAbstractButton::focusInEvent(QFocusEvent *e)
{
   Q_D(QAbstractButton);

#ifdef QT_KEYPAD_NAVIGATION

   if (! QApplication::keypadNavigationEnabled()) {
      d->fixFocusPolicy();
   }

#else
   d->fixFocusPolicy();
#endif

   QWidget::focusInEvent(e);
}

void QAbstractButton::focusOutEvent(QFocusEvent *e)
{
   Q_D(QAbstractButton);

   if (e->reason() != Qt::PopupFocusReason && d->down) {
      d->down = false;
      d->emitReleased();
   }

   QWidget::focusOutEvent(e);
}

void QAbstractButton::changeEvent(QEvent *e)
{
   Q_D(QAbstractButton);

   switch (e->type()) {
      case QEvent::EnabledChange:
         if (! isEnabled() && d->down) {
            d->down = false;
            d->emitReleased();
         }

         break;

      default:
         d->sizeHint = QSize();
         break;
   }

   QWidget::changeEvent(e);
}

QSize QAbstractButton::iconSize() const
{
   Q_D(const QAbstractButton);

   if (d->iconSize.isValid()) {
      return d->iconSize;
   }

   int e = style()->pixelMetric(QStyle::PM_ButtonIconSize, nullptr, this);

   return QSize(e, e);
}

void QAbstractButton::setIconSize(const QSize &size)
{
   Q_D(QAbstractButton);

   if (d->iconSize == size) {
      return;
   }

   d->iconSize = size;
   d->sizeHint = QSize();
   updateGeometry();

   if (isVisible()) {
      update();
   }
}
