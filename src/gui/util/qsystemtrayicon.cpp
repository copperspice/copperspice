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

#include <qsystemtrayicon.h>
#include <qsystemtrayicon_p.h>

#ifndef QT_NO_SYSTEMTRAYICON

#include <qmenu.h>
#include <qevent.h>
#include <qpoint.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpainterpath.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qgridlayout.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qbitmap.h>
#include <qlabel_p.h>
#include <qapplication.h>

QT_BEGIN_NAMESPACE


/*!
    Constructs a QSystemTrayIcon object with the given \a parent.

    The icon is initially invisible.

    \sa visible
*/
QSystemTrayIcon::QSystemTrayIcon(QObject *parent)
   : QObject(parent), d_ptr(new QSystemTrayIconPrivate)
{
   d_ptr->q_ptr = this;
}

/*!
    Constructs a QSystemTrayIcon object with the given \a icon and \a parent.

    The icon is initially invisible.

    \sa visible
*/
QSystemTrayIcon::QSystemTrayIcon(const QIcon &icon, QObject *parent)
   : QObject(parent), d_ptr(new QSystemTrayIconPrivate)
{
   d_ptr->q_ptr = this;
   setIcon(icon);
}

/*!
    Removes the icon from the system tray and frees all allocated resources.
*/
QSystemTrayIcon::~QSystemTrayIcon()
{
   Q_D(QSystemTrayIcon);
   d->remove_sys();
}

#ifndef QT_NO_MENU

/*!
    Sets the specified \a menu to be the context menu for the system tray icon.

    The menu will pop up when the user requests the context menu for the system
    tray icon by clicking the mouse button.

    On Mac OS X, this is currenly converted to a NSMenu, so the
    aboutToHide() signal is not emitted.

    \note The system tray icon does not take ownership of the menu. You must
    ensure that it is deleted at the appropriate time by, for example, creating
    the menu with a suitable parent object.
*/
void QSystemTrayIcon::setContextMenu(QMenu *menu)
{
   Q_D(QSystemTrayIcon);
   d->menu = menu;
   d->updateMenu_sys();
}

/*!
    Returns the current context menu for the system tray entry.
*/
QMenu *QSystemTrayIcon::contextMenu() const
{
   Q_D(const QSystemTrayIcon);
   return d->menu;
}

#endif // QT_NO_MENU

/*!
    \property QSystemTrayIcon::icon
    \brief the system tray icon

    On Windows, the system tray icon size is 16x16; on X11, the preferred size is
    22x22. The icon will be scaled to the appropriate size as necessary.
*/
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

/*!
    \property QSystemTrayIcon::toolTip
    \brief the tooltip for the system tray entry

    On some systems, the tooltip's length is limited. The tooltip will be truncated
    if necessary.
*/
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

/*!
    \fn void QSystemTrayIcon::show()

    Shows the icon in the system tray.

    \sa hide(), visible
*/

/*!
    \fn void QSystemTrayIcon::hide()

    Hides the system tray entry.

    \sa show(), visible
*/

/*!
    \since 4.3
    Returns the geometry of the system tray icon in screen coordinates.

    \sa visible
*/
QRect QSystemTrayIcon::geometry() const
{
   Q_D(const QSystemTrayIcon);
   if (!d->visible) {
      return QRect();
   }
   return d->geometry_sys();
}

/*!
    \property QSystemTrayIcon::visible
    \brief whether the system tray entry is visible

    Setting this property to true or calling show() makes the system tray icon
    visible; setting this property to false or calling hide() hides it.
*/
void QSystemTrayIcon::setVisible(bool visible)
{
   Q_D(QSystemTrayIcon);
   if (visible == d->visible) {
      return;
   }
   if (d->icon.isNull() && visible) {
      qWarning("QSystemTrayIcon::setVisible: No Icon set");
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

/*!
  \reimp
*/
bool QSystemTrayIcon::event(QEvent *e)
{
#if defined(Q_WS_X11)
   if (e->type() == QEvent::ToolTip) {
      Q_D(QSystemTrayIcon);
      return d->sys->deliverToolTipEvent(e);
   }
#endif
   return QObject::event(e);
}

/*!
    \enum QSystemTrayIcon::ActivationReason

     This enum describes the reason the system tray was activated.

     \value Unknown     Unknown reason
     \value Context     The context menu for the system tray entry was requested
     \value DoubleClick The system tray entry was double clicked
     \value Trigger     The system tray entry was clicked
     \value MiddleClick The system tray entry was clicked with the middle mouse button

     \sa activated()
*/

/*!
    \fn void QSystemTrayIcon::activated(QSystemTrayIcon::ActivationReason reason)

    This signal is emitted when the user activates the system tray icon. \a reason
    specifies the reason for activation. QSystemTrayIcon::ActivationReason enumerates
    the various reasons.

    \sa QSystemTrayIcon::ActivationReason
*/

/*!
    \fn void QSystemTrayIcon::messageClicked()

    This signal is emitted when the message displayed using showMessage()
    was clicked by the user.

    Currently this signal is not sent on Mac OS X.

    \note We follow Microsoft Windows XP/Vista behavior, so the
    signal is also emitted when the user clicks on a tray icon with
    a balloon message displayed.

    \sa activated()
*/


/*!
    Returns true if the system tray is available; otherwise returns false.

    If the system tray is currently unavailable but becomes available later,
    QSystemTrayIcon will automatically add an entry in the system tray if it
    is \l visible.
*/

bool QSystemTrayIcon::isSystemTrayAvailable()
{
   return QSystemTrayIconPrivate::isSystemTrayAvailable_sys();
}

/*!
    Returns true if the system tray supports balloon messages; otherwise returns false.

    \sa showMessage()
*/
bool QSystemTrayIcon::supportsMessages()
{
   return QSystemTrayIconPrivate::supportsMessages_sys();
}

/*!
    \fn void QSystemTrayIcon::showMessage(const QString &title, const QString &message, MessageIcon icon, int millisecondsTimeoutHint)
    \since 4.3

    Shows a balloon message for the entry with the given \a title, \a message and
    \a icon for the time specified in \a millisecondsTimeoutHint. \a title and \a message
    must be plain text strings.

    Message can be clicked by the user; the messageClicked() signal will emitted when
    this occurs.

    Note that display of messages are dependent on the system configuration and user
    preferences, and that messages may not appear at all. Hence, it should not be
    relied upon as the sole means for providing critical information.

    On Windows, the \a millisecondsTimeoutHint is usually ignored by the system
    when the application has focus.

    On Mac OS X, the Growl notification system must be installed for this function to
    display messages.

    \sa show() supportsMessages()
  */
void QSystemTrayIcon::showMessage(const QString &title, const QString &msg,
                                  QSystemTrayIcon::MessageIcon icon, int msecs)
{
   Q_D(QSystemTrayIcon);
   if (d->visible) {
      d->showMessage_sys(title, msg, icon, msecs);
   }
}

//////////////////////////////////////////////////////////////////////
static QBalloonTip *theSolitaryBalloonTip = 0;

void QBalloonTip::showBalloon(QSystemTrayIcon::MessageIcon icon, const QString &title,
                              const QString &message, QSystemTrayIcon *trayIcon,
                              const QPoint &pos, int timeout, bool showArrow)
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
   if (!theSolitaryBalloonTip) {
      return;
   }
   theSolitaryBalloonTip->hide();
   delete theSolitaryBalloonTip;
   theSolitaryBalloonTip = 0;
}

bool QBalloonTip::isBalloonVisible()
{
   return theSolitaryBalloonTip;
}

QBalloonTip::QBalloonTip(QSystemTrayIcon::MessageIcon icon, const QString &title,
                         const QString &message, QSystemTrayIcon *ti)
   : QWidget(0, Qt::ToolTip), trayIcon(ti), timerId(-1)
{
   setAttribute(Qt::WA_DeleteOnClose);
   QObject::connect(ti, SIGNAL(destroyed()), this, SLOT(close()));

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
   QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

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
   theSolitaryBalloonTip = 0;
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
   if (!arrowAtTop) {
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
#if defined(QT_NO_XSHAPE) && defined(Q_WS_X11)
   // XShape is required for setting the mask, so we just
   // draw an ugly square when its not available
   path.moveTo(0, 0);
   path.lineTo(sz.width() - 1, 0);
   path.lineTo(sz.width() - 1, sz.height() - 1);
   path.lineTo(0, sz.height() - 1);
   path.lineTo(0, 0);
   move(qMax(pos.x() - sz.width(), scr.left()), pos.y());
#else
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
   if (!arrowAtTop && !arrowAtLeft) {
      if (showArrow) {
         path.lineTo(mr - ao, mb);
         path.lineTo(mr - ao, mb + ah);
         path.lineTo(mr - ao - aw, mb);
      }
      move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2),
           pos.y() - sh.height());
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
#endif

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

void qtsystray_sendActivated(QSystemTrayIcon *i, int r)
{
   emit i->activated((QSystemTrayIcon::ActivationReason)r);
}

void QSystemTrayIcon::show()
{
   setVisible(true);
}

void QSystemTrayIcon::hide()
{
   setVisible(false);
}

QT_END_NAMESPACE

#endif // QT_NO_SYSTEMTRAYICON
