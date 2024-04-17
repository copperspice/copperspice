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

#include <qapplication.h>
#include <qbitmap.h>
#include <qdebug.h>
#include <qdesktopwidget.h>
#include <qdialog.h>
#include <qdialogbuttonbox.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qfontmetrics.h>
#include <qlayoutitem.h>
#include <qmenu.h>
#include <qpixmap.h>
#include <qpointer.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>
#include <qtoolbar.h>

#include <qdialog_p.h>
#include <qmenu_p.h>
#include <qpushbutton_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

QPushButton::QPushButton(QWidget *parent)
   : QAbstractButton(*new QPushButtonPrivate, parent)
{
   Q_D(QPushButton);
   d->init();
}

QPushButton::QPushButton(const QString &text, QWidget *parent)
   : QAbstractButton(*new QPushButtonPrivate, parent)
{
   Q_D(QPushButton);
   setText(text);
   d->init();
}

QPushButton::QPushButton(const QIcon &icon, const QString &text, QWidget *parent)
   : QAbstractButton(*new QPushButtonPrivate, parent)
{
   Q_D(QPushButton);
   setText(text);
   setIcon(icon);
   d->init();
}

QPushButton::QPushButton(QPushButtonPrivate &dd, QWidget *parent)
   : QAbstractButton(dd, parent)
{
   Q_D(QPushButton);
   d->init();
}

QPushButton::~QPushButton()
{
}

QDialog *QPushButtonPrivate::dialogParent() const
{
   Q_Q(const QPushButton);
   const QWidget *p = q;

   while (p && !p->isWindow()) {
      p = p->parentWidget();
      if (const QDialog *dialog = qobject_cast<const QDialog *>(p)) {
         return const_cast<QDialog *>(dialog);
      }
   }

   return nullptr;
}

void QPushButton::initStyleOption(QStyleOptionButton *option) const
{
   if (!option) {
      return;
   }

   Q_D(const QPushButton);
   option->initFrom(this);
   option->features = QStyleOptionButton::None;

   if (d->flat) {
      option->features |= QStyleOptionButton::Flat;
   }

#ifndef QT_NO_MENU
   if (d->menu) {
      option->features |= QStyleOptionButton::HasMenu;
   }
#endif

   if (autoDefault()) {
      option->features |= QStyleOptionButton::AutoDefaultButton;
   }

   if (d->defaultButton) {
      option->features |= QStyleOptionButton::DefaultButton;
   }

   if (d->down || d->menuOpen) {
      option->state |= QStyle::State_Sunken;
   }

   if (d->checked) {
      option->state |= QStyle::State_On;
   }

   if (!d->flat && !d->down) {
      option->state |= QStyle::State_Raised;
   }

   option->text = d->text;
   option->icon = d->icon;
   option->iconSize = iconSize();
}

void QPushButton::setAutoDefault(bool enable)
{
   Q_D(QPushButton);

   uint state = enable ? QPushButtonPrivate::On : QPushButtonPrivate::Off;
   if (d->autoDefault != QPushButtonPrivate::Auto && d->autoDefault == state) {
      return;
   }

   d->autoDefault = state;
   d->sizeHint = QSize();
   update();
   updateGeometry();
}

bool QPushButton::autoDefault() const
{
   Q_D(const QPushButton);

   if (d->autoDefault == QPushButtonPrivate::Auto) {
      return ( d->dialogParent() != nullptr);
   }

   return d->autoDefault;
}

void QPushButton::setDefault(bool enable)
{
   Q_D(QPushButton);

   if (d->defaultButton == enable) {
      return;
   }

   d->defaultButton = enable;
   if (d->defaultButton) {
      if (QDialog *dlg = d->dialogParent()) {
         dlg->d_func()->setMainDefault(this);
      }
   }

   update();

#ifndef QT_NO_ACCESSIBILITY
   QAccessible::State s;
   s.defaultButton = true;
   QAccessibleStateChangeEvent event(this, s);
   QAccessible::updateAccessibility(&event);
#endif
}

bool QPushButton::isDefault() const
{
   Q_D(const QPushButton);
   return d->defaultButton;
}

QSize QPushButton::sizeHint() const
{
   Q_D(const QPushButton);

   if (d->sizeHint.isValid() && d->lastAutoDefault == autoDefault()) {
      return d->sizeHint;
   }
   d->lastAutoDefault = autoDefault();
   ensurePolished();

   int w = 0, h = 0;

   QStyleOptionButton opt;
   initStyleOption(&opt);

   // calculate contents size
#ifndef QT_NO_ICON

   bool showButtonBoxIcons = qobject_cast<QDialogButtonBox *>(parentWidget())
      && style()->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons);

   if (!icon().isNull() || showButtonBoxIcons) {
      int ih = opt.iconSize.height();
      int iw = opt.iconSize.width() + 4;
      w += iw;
      h = qMax(h, ih);
   }
#endif

   QString s(text());
   bool empty = s.isEmpty();

   if (empty) {
      s = QString::fromLatin1("XXXX");
   }

   QFontMetrics fm = fontMetrics();
   QSize sz = fm.size(Qt::TextShowMnemonic, s);

   if (!empty || !w) {
      w += sz.width();
   }

   if (!empty || !h) {
      h = qMax(h, sz.height());
   }

   opt.rect.setSize(QSize(w, h)); // PM_MenuButtonIndicator depends on the height

#ifndef QT_NO_MENU
   if (menu()) {
      w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
   }
#endif

   d->sizeHint = (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this).
         expandedTo(QApplication::globalStrut()));
   return d->sizeHint;
}

QSize QPushButton::minimumSizeHint() const
{
   return sizeHint();
}

void QPushButton::paintEvent(QPaintEvent *)
{
   QStylePainter p(this);
   QStyleOptionButton option;
   initStyleOption(&option);
   p.drawControl(QStyle::CE_PushButton, option);
}

void QPushButton::keyPressEvent(QKeyEvent *e)
{
   Q_D(QPushButton);

   switch (e->key()) {
      case Qt::Key_Enter:
      case Qt::Key_Return:
         if (autoDefault() || d->defaultButton) {
            click();
            break;
         }
         [[fallthrough]];

      default:
         QAbstractButton::keyPressEvent(e);
   }
}

void QPushButton::focusInEvent(QFocusEvent *e)
{
   Q_D(QPushButton);

   if (e->reason() != Qt::PopupFocusReason && autoDefault() && ! d->defaultButton) {
      d->defaultButton = true;
      QDialog *dlg = qobject_cast<QDialog *>(window());

      if (dlg) {
         dlg->d_func()->setDefault(this);
      }
   }

   QAbstractButton::focusInEvent(e);
}

void QPushButton::focusOutEvent(QFocusEvent *e)
{
   Q_D(QPushButton);

   if (e->reason() != Qt::PopupFocusReason && autoDefault() && d->defaultButton) {
      QDialog *dlg = qobject_cast<QDialog *>(window());

      if (dlg) {
         dlg->d_func()->setDefault(nullptr);
      } else {
         d->defaultButton = false;
      }
   }

   QAbstractButton::focusOutEvent(e);

#ifndef QT_NO_MENU
   if (d->menu && d->menu->isVisible()) {
      // restore pressed status
      setDown(true);
   }
#endif
}

#ifndef QT_NO_MENU

void QPushButton::setMenu(QMenu *menu)
{
   Q_D(QPushButton);
   if (menu == d->menu) {
      return;
   }

   if (menu && ! d->menu) {
      connect(this, &QPushButton::pressed, this, &QPushButton::_q_popupPressed, Qt::UniqueConnection);
   }

   if (d->menu) {
      removeAction(d->menu->menuAction());
   }

   d->menu = menu;
   if (d->menu) {
      addAction(d->menu->menuAction());
   }

   d->resetLayoutItemMargins();
   d->sizeHint = QSize();
   update();
   updateGeometry();
}

QMenu *QPushButton::menu() const
{
   Q_D(const QPushButton);
   return d->menu;
}

void QPushButton::showMenu()
{
   Q_D(QPushButton);
   if (!d || !d->menu) {
      return;
   }
   setDown(true);
   d->_q_popupPressed();
}

void QPushButtonPrivate::_q_popupPressed()
{
   Q_Q(QPushButton);
   if (!down || !menu) {
      return;
   }

   menu->setNoReplayFor(q);

   QPoint menuPos = adjustedMenuPosition();

   QPointer<QPushButton> guard(q);
   QMenuPrivate::get(menu)->causedPopup.widget = guard;

   // Because of a delay in menu effects, we must keep track of the
   // menu visibility to avoid flicker on button release
   menuOpen = true;
   menu->exec(menuPos);
   if (guard) {
      menuOpen = false;
      q->setDown(false);
   }
}

QPoint QPushButtonPrivate::adjustedMenuPosition()
{
   Q_Q(QPushButton);

   bool horizontal = true;

#if !defined(QT_NO_TOOLBAR)
   QToolBar *tb = qobject_cast<QToolBar *>(q->parent());
   if (tb && tb->orientation() == Qt::Vertical) {
      horizontal = false;
   }
#endif

   QWidgetItem item(q);
   QRect rect = item.geometry();
   rect.setRect(rect.x() - q->x(), rect.y() - q->y(), rect.width(), rect.height());

   QSize menuSize   = menu->sizeHint();
   QPoint globalPos = q->mapToGlobal(rect.topLeft());
   int x = globalPos.x();
   int y = globalPos.y();

   if (horizontal) {
      if (globalPos.y() + rect.height() + menuSize.height() <= QApplication::desktop()->availableGeometry(q).height()) {
         y += rect.height();
      } else {
         y -= menuSize.height();
      }
      if (q->layoutDirection() == Qt::RightToLeft) {
         x += rect.width() - menuSize.width();
      }
   } else {
      if (globalPos.x() + rect.width() + menu->sizeHint().width() <= QApplication::desktop()->availableGeometry(q).width()) {
         x += rect.width();
      } else {
         x -= menuSize.width();
      }
   }

   return QPoint(x, y);
}

#endif // QT_NO_MENU

void QPushButtonPrivate::resetLayoutItemMargins()
{
   Q_Q(QPushButton);
   QStyleOptionButton opt;
   q->initStyleOption(&opt);
   setLayoutItemMargins(QStyle::SE_PushButtonLayoutItem, &opt);
}

void QPushButton::setFlat(bool flat)
{
   Q_D(QPushButton);

   if (d->flat == flat) {
      return;
   }

   d->flat = flat;
   d->resetLayoutItemMargins();
   d->sizeHint = QSize();

   update();
   updateGeometry();
}

bool QPushButton::isFlat() const
{
   Q_D(const QPushButton);
   return d->flat;
}

bool QPushButton::event(QEvent *e)
{
   Q_D(QPushButton);

   if (e->type() == QEvent::ParentChange) {
      if (QDialog *dialog = d->dialogParent()) {
         if (d->defaultButton) {
            dialog->d_func()->setMainDefault(this);
         }
      }

   } else if (e->type() == QEvent::StyleChange

#ifdef Q_OS_DARWIN
      || e->type() == QEvent::MacSizeChange
#endif

   ) {
      d->resetLayoutItemMargins();
      updateGeometry();
   } else if (e->type() == QEvent::PolishRequest) {
      updateGeometry();
   }
   return QAbstractButton::event(e);
}

void QPushButton::_q_popupPressed()
{
   Q_D(QPushButton);
   d->_q_popupPressed();
}
