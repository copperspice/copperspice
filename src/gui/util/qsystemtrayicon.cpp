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

#include <qsystemtrayicon.h>

#ifndef QT_NO_SYSTEMTRAYICON

#include <qapplication.h>
#include <qbitmap.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qstyle.h>

#include <qlabel_p.h>
#include <qsystemtrayicon_p.h>

QSystemTrayIcon::QSystemTrayIcon(QObject *parent)
   : QObject(parent), d_ptr(new QSystemTrayIconPrivate)
{
   d_ptr->q_ptr = this;
}

QSystemTrayIcon::QSystemTrayIcon(const QIcon &icon, QObject *parent)
   : QObject(parent), d_ptr(new QSystemTrayIconPrivate)
{
   d_ptr->q_ptr = this;
   setIcon(icon);
}

QSystemTrayIcon::~QSystemTrayIcon()
{
   Q_D(QSystemTrayIcon);
   d->remove_sys();
}

#ifndef QT_NO_MENU
void QSystemTrayIcon::setContextMenu(QMenu *menu)
{
   Q_D(QSystemTrayIcon);
   d->menu = menu;
   d->updateMenu_sys();
}

QMenu *QSystemTrayIcon::contextMenu() const
{
   Q_D(const QSystemTrayIcon);
   return d->menu;
}
#endif

void QSystemTrayIcon::setIcon(const QIcon &icon)
{
   Q_D(QSystemTrayIcon);
   d->icon = icon;
   d->updateIcon_sys();
}

QIcon QSystemTrayIcon::icon() const
{
   Q_D(const QSystemTrayIcon);
   return d->icon;
}

void QSystemTrayIcon::setToolTip(const QString &tooltip)
{
   Q_D(QSystemTrayIcon);
   d->toolTip = tooltip;
   d->updateToolTip_sys();
}

QString QSystemTrayIcon::toolTip() const
{
   Q_D(const QSystemTrayIcon);
   return d->toolTip;
}

QRect QSystemTrayIcon::geometry() const
{
   Q_D(const QSystemTrayIcon);

   if (! d->visible) {
      return QRect();
   }

   return d->geometry_sys();
}

void QSystemTrayIcon::setVisible(bool visible)
{
   Q_D(QSystemTrayIcon);

   if (visible == d->visible) {
      return;
   }

   if (d->icon.isNull() && visible) {
      qWarning("QSystemTrayIcon::setVisible() No Icon was set");
   }

   d->visible = visible;

   if (d->visible) {
      d->install_sys();
   } else {
      d->remove_sys();
   }
}

bool QSystemTrayIcon::isVisible() const
{
   Q_D(const QSystemTrayIcon);
   return d->visible;
}

bool QSystemTrayIcon::event(QEvent *e)
{
   return QObject::event(e);
}

bool QSystemTrayIcon::isSystemTrayAvailable()
{
   return QSystemTrayIconPrivate::isSystemTrayAvailable_sys();
}

bool QSystemTrayIcon::supportsMessages()
{
   return QSystemTrayIconPrivate::supportsMessages_sys();
}

void QSystemTrayIcon::showMessage(const QString &title, const QString &msg,
      QSystemTrayIcon::MessageIcon icon, int msecs)
{
   Q_D(QSystemTrayIcon);

   if (d->visible) {
      d->showMessage_sys(title, msg, icon, msecs);
   }
}

void QSystemTrayIconPrivate::_q_emitActivated(QPlatformSystemTrayIcon::ActivationReason reason)
{
   Q_Q(QSystemTrayIcon);
   emit q->activated(static_cast<QSystemTrayIcon::ActivationReason>(reason));
}

static QBalloonTip *theSolitaryBalloonTip = nullptr;

void QBalloonTip::showBalloon(QSystemTrayIcon::MessageIcon icon, const QString &title,
      const QString &message, QSystemTrayIcon *trayIcon, const QPoint &pos, int timeout, bool showArrow)
{
   hideBalloon();

   if (message.isEmpty() && title.isEmpty()) {
      return;
   }

   theSolitaryBalloonTip = new QBalloonTip(icon, title, message, trayIcon);

   if (timeout < 0) {
      timeout = 10000;   //10 s default
   }

   theSolitaryBalloonTip->balloon(pos, timeout, showArrow);
}

void QBalloonTip::hideBalloon()
{
   if (! theSolitaryBalloonTip) {
      return;
   }

   theSolitaryBalloonTip->hide();
   delete theSolitaryBalloonTip;
   theSolitaryBalloonTip = nullptr;
}

void QBalloonTip::updateBalloonPosition(const QPoint &pos)
{
   if (! theSolitaryBalloonTip) {
      return;
   }

   theSolitaryBalloonTip->hide();
   theSolitaryBalloonTip->balloon(pos, 0, theSolitaryBalloonTip->showArrow);
}

bool QBalloonTip::isBalloonVisible()
{
   return theSolitaryBalloonTip;
}

QBalloonTip::QBalloonTip(QSystemTrayIcon::MessageIcon icon, const QString &title,
      const QString &message, QSystemTrayIcon *ti)
   : QWidget(nullptr, Qt::ToolTip), trayIcon(ti), timerId(-1), showArrow(true)
{
   setAttribute(Qt::WA_DeleteOnClose);
   QObject::connect(ti, &QSystemTrayIcon::destroyed, this, &QBalloonTip::close);

   QLabel *titleLabel = new QLabel;
   titleLabel->installEventFilter(this);
   titleLabel->setText(title);
   QFont f = titleLabel->font();
   f.setBold(true);

   titleLabel->setFont(f);
   titleLabel->setTextFormat(Qt::PlainText); // to maintain compat with windows

   const int iconSize = 18;
   const int closeButtonSize = 15;

   QPushButton *closeButton = new QPushButton;
   closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
   closeButton->setIconSize(QSize(closeButtonSize, closeButtonSize));
   closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   closeButton->setFixedSize(closeButtonSize, closeButtonSize);

   QObject::connect(closeButton, &QPushButton::clicked, this, &QBalloonTip::close);

   QLabel *msgLabel = new QLabel;

   msgLabel->installEventFilter(this);
   msgLabel->setText(message);
   msgLabel->setTextFormat(Qt::PlainText);
   msgLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

   // smart size for the message label
   int limit = QApplication::desktop()->availableGeometry(msgLabel).size().width() / 3;

   if (msgLabel->sizeHint().width() > limit) {
      msgLabel->setWordWrap(true);

      if (msgLabel->sizeHint().width() > limit) {
         msgLabel->d_func()->ensureTextControl();

         if (QTextControl *control = msgLabel->d_func()->control) {
            QTextOption opt = control->document()->defaultTextOption();
            opt.setWrapMode(QTextOption::WrapAnywhere);
            control->document()->setDefaultTextOption(opt);
         }
      }

      // Here we allow the text being much smaller than the balloon widget
      // to emulate the weird standard windows behavior.
      msgLabel->setFixedSize(limit, msgLabel->heightForWidth(limit));
   }

   QIcon si;

   switch (icon) {
      case QSystemTrayIcon::Warning:
         si = style()->standardIcon(QStyle::SP_MessageBoxWarning);
         break;

      case QSystemTrayIcon::Critical:
         si = style()->standardIcon(QStyle::SP_MessageBoxCritical);
         break;

      case QSystemTrayIcon::Information:
         si = style()->standardIcon(QStyle::SP_MessageBoxInformation);
         break;

      case QSystemTrayIcon::NoIcon:
      default:
         break;
   }

   QGridLayout *layout = new QGridLayout;

   if (!si.isNull()) {
      QLabel *iconLabel = new QLabel;
      iconLabel->setPixmap(si.pixmap(iconSize, iconSize));
      iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      iconLabel->setMargin(2);
      layout->addWidget(iconLabel, 0, 0);
      layout->addWidget(titleLabel, 0, 1);
   } else {
      layout->addWidget(titleLabel, 0, 0, 1, 2);
   }

   layout->addWidget(closeButton, 0, 2);
   layout->addWidget(msgLabel, 1, 0, 1, 3);
   layout->setSizeConstraint(QLayout::SetFixedSize);
   layout->setMargin(3);
   setLayout(layout);

   QPalette pal = palette();
   pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xe1));
   pal.setColor(QPalette::WindowText, Qt::black);
   setPalette(pal);
}

QBalloonTip::~QBalloonTip()
{
   theSolitaryBalloonTip = nullptr;
}

void QBalloonTip::paintEvent(QPaintEvent *)
{
   QPainter painter(this);
   painter.drawPixmap(rect(), pixmap);
}

void QBalloonTip::resizeEvent(QResizeEvent *ev)
{
   QWidget::resizeEvent(ev);
}

void QBalloonTip::balloon(const QPoint &pos, int msecs, bool showArrow)
{
   this->showArrow = showArrow;
   QRect scr = QApplication::desktop()->screenGeometry(pos);
   QSize sh = sizeHint();
   const int border = 1;
   const int ah = 18, ao = 18, aw = 18, rc = 7;
   bool arrowAtTop = (pos.y() + sh.height() + ah < scr.height());
   bool arrowAtLeft = (pos.x() + sh.width() - ao < scr.width());
   setContentsMargins(border + 3,  border + (arrowAtTop ? ah : 0) + 2, border + 3, border + (arrowAtTop ? 0 : ah) + 2);
   updateGeometry();
   sh  = sizeHint();

   int ml, mr, mt, mb;
   QSize sz = sizeHint();

   if (! arrowAtTop) {
      ml = mt = 0;
      mr = sz.width() - 1;
      mb = sz.height() - ah - 1;
   } else {
      ml = 0;
      mt = ah;
      mr = sz.width() - 1;
      mb = sz.height() - 1;
   }

   QPainterPath path;
   path.moveTo(ml + rc, mt);

   if (arrowAtTop && arrowAtLeft) {
      if (showArrow) {
         path.lineTo(ml + ao, mt);
         path.lineTo(ml + ao, mt - ah);
         path.lineTo(ml + ao + aw, mt);
      }

      move(qMax(pos.x() - ao, scr.left() + 2), pos.y());

   } else if (arrowAtTop && !arrowAtLeft) {
      if (showArrow) {
         path.lineTo(mr - ao - aw, mt);
         path.lineTo(mr - ao, mt - ah);
         path.lineTo(mr - ao, mt);
      }

      move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2), pos.y());
   }

   path.lineTo(mr - rc, mt);
   path.arcTo(QRect(mr - rc * 2, mt, rc * 2, rc * 2), 90, -90);
   path.lineTo(mr, mb - rc);
   path.arcTo(QRect(mr - rc * 2, mb - rc * 2, rc * 2, rc * 2), 0, -90);

   if (! arrowAtTop && !arrowAtLeft) {
      if (showArrow) {
         path.lineTo(mr - ao, mb);
         path.lineTo(mr - ao, mb + ah);
         path.lineTo(mr - ao - aw, mb);
      }

      move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2), pos.y() - sh.height());

   } else if (!arrowAtTop && arrowAtLeft) {
      if (showArrow) {
         path.lineTo(ao + aw, mb);
         path.lineTo(ao, mb + ah);
         path.lineTo(ao, mb);
      }

      move(qMax(pos.x() - ao, scr.x() + 2), pos.y() - sh.height());
   }

   path.lineTo(ml + rc, mb);
   path.arcTo(QRect(ml, mb - rc * 2, rc * 2, rc * 2), -90, -90);
   path.lineTo(ml, mt + rc);
   path.arcTo(QRect(ml, mt, rc * 2, rc * 2), 180, -90);

   // Set the mask
   QBitmap bitmap = QBitmap(sizeHint());
   bitmap.fill(Qt::color0);
   QPainter painter1(&bitmap);
   painter1.setPen(QPen(Qt::color1, border));
   painter1.setBrush(QBrush(Qt::color1));
   painter1.drawPath(path);
   setMask(bitmap);

   // Draw the border
   pixmap = QPixmap(sz);
   QPainter painter2(&pixmap);
   painter2.setPen(QPen(palette().color(QPalette::Window).darker(160), border));
   painter2.setBrush(palette().color(QPalette::Window));
   painter2.drawPath(path);

   if (msecs > 0) {
      timerId = startTimer(msecs);
   }

   show();
}

void QBalloonTip::mousePressEvent(QMouseEvent *e)
{
   close();

   if (e->button() == Qt::LeftButton) {
      emit trayIcon->messageClicked();
   }
}

void QBalloonTip::timerEvent(QTimerEvent *e)
{
   if (e->timerId() == timerId) {
      killTimer(timerId);

      if (!underMouse()) {
         close();
      }

      return;
   }

   QWidget::timerEvent(e);
}

void QSystemTrayIconPrivate::install_sys_qpa()
{
   qpa_sys->init();

   QObject::connect(qpa_sys, &QPlatformSystemTrayIcon::activated,      q_func(), &QSystemTrayIcon::_q_emitActivated);
   QObject::connect(qpa_sys, &QPlatformSystemTrayIcon::messageClicked, q_func(), &QSystemTrayIcon::messageClicked);

   updateMenu_sys();
   updateIcon_sys();
   updateToolTip_sys();
}

void QSystemTrayIconPrivate::remove_sys_qpa()
{
   QObject::disconnect(qpa_sys, &QPlatformSystemTrayIcon::activated,      q_func(), &QSystemTrayIcon::_q_emitActivated);
   QObject::disconnect(qpa_sys, &QPlatformSystemTrayIcon::messageClicked, q_func(), &QSystemTrayIcon::messageClicked);

   qpa_sys->cleanup();
}

QRect QSystemTrayIconPrivate::geometry_sys_qpa() const
{
   return qpa_sys->geometry();
}

void QSystemTrayIconPrivate::updateIcon_sys_qpa()
{
   qpa_sys->updateIcon(icon);
}

void QSystemTrayIconPrivate::updateMenu_sys_qpa()
{
   if (menu) {
      addPlatformMenu(menu);
      qpa_sys->updateMenu(menu->platformMenu());
   }
}

void QSystemTrayIconPrivate::updateToolTip_sys_qpa()
{
   qpa_sys->updateToolTip(toolTip);
}

void QSystemTrayIconPrivate::showMessage_sys_qpa(const QString &title,
      const QString &message, QSystemTrayIcon::MessageIcon icon, int msecs)
{
   QIcon notificationIcon;

   switch (icon) {
      case QSystemTrayIcon::Information:
         notificationIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
         break;

      case QSystemTrayIcon::Warning:
         notificationIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
         break;

      case QSystemTrayIcon::Critical:
         notificationIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
         break;

      default:
         break;
   }

   qpa_sys->showMessage(title, message, notificationIcon,
         static_cast<QPlatformSystemTrayIcon::MessageIcon>(icon), msecs);
}

void QSystemTrayIconPrivate::addPlatformMenu(QMenu *menu) const
{
   if (menu->platformMenu()) {
      return;   // The platform menu already exists.
   }

   // The recursion depth is the same as menu depth, so should not
   // be higher than 3 levels.
   QListIterator<QAction *> it(menu->actions());

   while (it.hasNext()) {
      QAction *action = it.next();

      if (action->menu()) {
         addPlatformMenu(action->menu());
      }
   }

   // This menu should be processed *after* its children, otherwise
   // setMenu() is not called on respective QPlatformMenuItems.
   QPlatformMenu *platformMenu = qpa_sys->createMenu();

   if (platformMenu) {
      menu->setPlatformMenu(platformMenu);
   }
}

void QSystemTrayIcon::_q_emitActivated(QPlatformSystemTrayIcon::ActivationReason data)
{
   Q_D(QSystemTrayIcon);
   d->_q_emitActivated(data);
}

#endif // QT_NO_SYSTEMTRAYICON
