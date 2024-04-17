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
#include <qbackingstore.h>
#include <qbitmap.h>
#include <qdebug.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlist.h>
#include <qmenu.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qplatform_nativeinterface.h>
#include <qplatform_systemtrayicon.h>
#include <qplatform_theme.h>
#include <qscreen.h>
#include <qtimer.h>
#include <qwindow.h>

#include <platformheaders/qxcbwindowfunctions.h>
#include <platformheaders/qxcbintegrationfunctions.h>

#include <qapplication_p.h>
#include <qsystemtrayicon_p.h>

#ifndef QT_NO_SYSTEMTRAYICON

static inline unsigned long locateSystemTray()
{
   return (unsigned long)QGuiApplication::platformNativeInterface()->nativeResourceForScreen(QByteArray("traywindow"),
         QGuiApplication::primaryScreen());
}

// System tray widget. Could be replaced by a QWindow with
// a backing store if it did not need tooltip handling.
class QSystemTrayIconSys : public QWidget
{
   GUI_CS_OBJECT(QSystemTrayIconSys)

 public:
   explicit QSystemTrayIconSys(QSystemTrayIcon *q);

   void updateIcon() {
      update();
   }

   QSystemTrayIcon *systemTrayIcon() const {
      return q;
   }

   QRect globalGeometry() const;

 protected:
   virtual void mousePressEvent(QMouseEvent *ev) override;
   virtual void mouseDoubleClickEvent(QMouseEvent *ev) override;
   virtual bool event(QEvent *) override;
   virtual void paintEvent(QPaintEvent *) override;
   virtual void resizeEvent(QResizeEvent *) override;
   virtual void moveEvent(QMoveEvent *) override;

 private:
   GUI_CS_SLOT_1(Private, void systemTrayWindowChanged(QScreen *screen))
   GUI_CS_SLOT_2(systemTrayWindowChanged)

   bool addToTray();

   QSystemTrayIcon *q;
   QPixmap background;
};

QSystemTrayIconSys::QSystemTrayIconSys(QSystemTrayIcon *qIn)
   : QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint), q(qIn)
{
   setObjectName(QString("QSystemTrayIconSys"));
   setToolTip(q->toolTip());
   setAttribute(Qt::WA_AlwaysShowToolTips, true);
   setAttribute(Qt::WA_QuitOnClose, false);

   const QSize size(22, 22); // Gnome, standard size
   setGeometry(QRect(QPoint(0, 0), size));
   setMinimumSize(size);

   // We need two different behaviors depending on whether the X11 visual for the system tray
   // (a) exists and (b) supports an alpha channel, i.e. is 32 bits.
   // If we have a visual that has an alpha channel, we can paint this widget with a transparent
   // background and it will work.
   // However, if there's no alpha channel visual, in order for transparent tray icons to work,
   // we do not have a transparent background on the widget, but set the BackPixmap property of our
   // window to ParentRelative (so that it inherits the background of its X11 parent window), call
   // xcb_clear_region before painting (so that the inherited background is visible) and then grab
   // the just-drawn background from the X11 server.

   bool hasAlphaChannel = QXcbIntegrationFunctions::xEmbedSystemTrayVisualHasAlphaChannel();
   setAttribute(Qt::WA_TranslucentBackground, hasAlphaChannel);

   if (!hasAlphaChannel) {
      createWinId();
      QXcbWindowFunctions::setParentRelativeBackPixmap(windowHandle());

      // XXX: This is actually required, but breaks things ("QWidget::paintEngine: Should no
      // longer be called"). Why is this needed? When the widget is drawn, we use tricks to grab
      // the tray icon's background from the server. If the tray icon isn't visible (because
      // another window is on top of it), the trick fails and instead uses the content of that
      // other window as the background.
      // setAttribute(Qt::WA_PaintOnScreen);
   }

   addToTray();
}

bool QSystemTrayIconSys::addToTray()
{
   if (! locateSystemTray()) {
      return false;
   }

   createWinId();
   setMouseTracking(true);

   if (! QXcbWindowFunctions::requestSystemTrayWindowDock(windowHandle())) {
      return false;
   }

   if (! background.isNull()) {
      background = QPixmap();
   }

   show();

   return true;
}

void QSystemTrayIconSys::systemTrayWindowChanged(QScreen *)
{
   if (locateSystemTray()) {
      addToTray();
   } else {
      QBalloonTip::hideBalloon();
      hide();                       // still no luck
      destroy();
   }
}

QRect QSystemTrayIconSys::globalGeometry() const
{
   return QXcbWindowFunctions::systemTrayWindowGlobalGeometry(windowHandle());
}

void QSystemTrayIconSys::mousePressEvent(QMouseEvent *ev)
{
   QPoint globalPos = ev->globalPos();

#ifndef QT_NO_CONTEXTMENU

   if (ev->button() == Qt::RightButton && q->contextMenu()) {
      q->contextMenu()->popup(globalPos);
   }

#endif

   if (QBalloonTip::isBalloonVisible()) {
      emit q->messageClicked();
      QBalloonTip::hideBalloon();
   }

   if (ev->button() == Qt::LeftButton) {
      emit q->activated(QSystemTrayIcon::Trigger);

   } else if (ev->button() == Qt::RightButton) {
      emit q->activated(QSystemTrayIcon::Context);

   } else if (ev->button() == Qt::MiddleButton) {
      emit q->activated(QSystemTrayIcon::MiddleClick);
   }
}

void QSystemTrayIconSys::mouseDoubleClickEvent(QMouseEvent *ev)
{
   if (ev->button() == Qt::LeftButton) {
      emit q->activated(QSystemTrayIcon::DoubleClick);
   }
}

bool QSystemTrayIconSys::event(QEvent *e)
{
   switch (e->type()) {
      case QEvent::ToolTip:
         QApplication::sendEvent(q, e);
         break;

#ifndef QT_NO_WHEELEVENT
      case QEvent::Wheel:
         return QApplication::sendEvent(q, e);
#endif

      default:
         break;
   }

   return QWidget::event(e);
}

void QSystemTrayIconSys::paintEvent(QPaintEvent *)
{
   const QRect rect(QPoint(0, 0), geometry().size());
   QPainter painter(this);

   // If we have Qt::WA_TranslucentBackground set, during widget creation
   // we detected the systray visual supported an alpha channel
   if (testAttribute(Qt::WA_TranslucentBackground)) {
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.fillRect(rect, Qt::transparent);

   } else {
      // clearRegion() was called on XEMBED_EMBEDDED_NOTIFY, so we hope that got done by now.
      // Grab the tray background pixmap, before rendering the icon for the first time.

      if (background.isNull()) {
         background = QGuiApplication::primaryScreen()->grabWindow(winId(),
               0, 0, rect.size().width(), rect.size().height());
      }

      // Then paint over the icon area with the background before compositing the icon on top.
      painter.drawPixmap(QPoint(0, 0), background);
   }

   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   q->icon().paint(&painter, rect);
}

void QSystemTrayIconSys::moveEvent(QMoveEvent *event)
{
   QWidget::moveEvent(event);

   if (QBalloonTip::isBalloonVisible()) {
      QBalloonTip::updateBalloonPosition(globalGeometry().center());
   }
}

void QSystemTrayIconSys::resizeEvent(QResizeEvent *event)
{
   update();
   QWidget::resizeEvent(event);

   if (QBalloonTip::isBalloonVisible()) {
      QBalloonTip::updateBalloonPosition(globalGeometry().center());
   }
}

QSystemTrayIconPrivate::QSystemTrayIconPrivate()
   : sys(nullptr), qpa_sys(QGuiApplicationPrivate::platformTheme()->createPlatformSystemTrayIcon()),
     visible(false)
{
}

QSystemTrayIconPrivate::~QSystemTrayIconPrivate()
{
   delete qpa_sys;
}

void QSystemTrayIconPrivate::install_sys()
{
   if (qpa_sys) {
      install_sys_qpa();
      return;
   }

   Q_Q(QSystemTrayIcon);

   if (! sys && locateSystemTray()) {
      sys = new QSystemTrayIconSys(q);

      QObject::connect(QApplication::platformNativeInterface(), SIGNAL(systemTrayWindowChanged(QScreen *)),
            sys, SLOT(systemTrayWindowChanged(QScreen *)));
   }
}

QRect QSystemTrayIconPrivate::geometry_sys() const
{
   if (qpa_sys) {
      return geometry_sys_qpa();
   }

   if (! sys) {
      return QRect();
   }

   return sys->globalGeometry();
}

void QSystemTrayIconPrivate::remove_sys()
{
   if (qpa_sys) {
      remove_sys_qpa();
      return;
   }

   if (!sys) {
      return;
   }

   QBalloonTip::hideBalloon();
   sys->hide();      // this should do the trick, but...
   delete sys;       // wm may resize system tray only for DestroyEvents
   sys = nullptr;
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
   if (qpa_sys) {
      updateIcon_sys_qpa();
      return;
   }

   if (sys) {
      sys->updateIcon();
   }
}

void QSystemTrayIconPrivate::updateMenu_sys()
{
   if (qpa_sys) {
      updateMenu_sys_qpa();
   }
}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
   if (qpa_sys) {
      updateToolTip_sys_qpa();
      return;
   }

   if (! sys) {
      return;
   }

#ifndef QT_NO_TOOLTIP
   sys->setToolTip(toolTip);
#endif
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
   QScopedPointer<QPlatformSystemTrayIcon> sys(QGuiApplicationPrivate::platformTheme()->createPlatformSystemTrayIcon());

   if (sys && sys->isSystemTrayAvailable()) {
      return true;
   }

   // no QPlatformSystemTrayIcon so fall back to default xcb platform behavior
   const QString platform = QGuiApplication::platformName();

   if (platform.compare("xcb", Qt::CaseInsensitive) == 0) {
      return locateSystemTray();
   }

   return false;
}

bool QSystemTrayIconPrivate::supportsMessages_sys()
{
   QScopedPointer<QPlatformSystemTrayIcon> sys(QGuiApplicationPrivate::platformTheme()->createPlatformSystemTrayIcon());

   if (sys) {
      return sys->supportsMessages();
   }

   return true;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &title, const QString &message,
      QSystemTrayIcon::MessageIcon icon, int msecs)
{
   if (qpa_sys) {
      showMessage_sys_qpa(title, message, icon, msecs);
      return;
   }

   if (! sys) {
      return;
   }

   QBalloonTip::showBalloon(icon, title, message, sys->systemTrayIcon(),
         sys->globalGeometry().center(), msecs);
}

#endif //QT_NO_SYSTEMTRAYICON
