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

#include <qabstracteventdispatcher.h>
#include <qcolormap.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdesktopwidget.h>
#include <qdir.h>
#include <qevent.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qgenericpluginfactory.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicsscene.h>
#include <qgesture.h>
#include <qhash.h>
#include <qinputmethod.h>
#include <qlayout.h>
#include <qlibrary.h>
#include <qlibraryinfo.h>
#include <qmessagebox.h>
#include <qmutex.h>
#include <qnumeric.h>
#include <qplatformdefs.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qpalette.h>
#include <qscreen.h>
#include <qset.h>
#include <qstylehints.h>
#include <qplatform_fontdatabase.h>
#include <qplatform_drag.h>
#include <qplatform_inputcontext.h>
#include <qplatform_integration.h>
#include <qplatform_nativeinterface.h>
#include <qplatform_theme.h>
#include <qplatform_window.h>
#include <qsessionmanager.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylefactory.h>
#include <qthread.h>
#include <qtextcodec.h>
#include <qtooltip.h>
#include <qtranslator.h>
#include <qvariant.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qwindowsysteminterface.h>

#include <qabstracteventdispatcher_p.h>
#include <qaccessiblewidget_factory_p.h>
#include <qapplication_p.h>
#include <qcoreapplication_p.h>
#include <qcursor_p.h>
#include <qdrawhelper_p.h>
#include <qdnd_p.h>
#include <qevent_p.h>
#include <qfont_p.h>
#include <qgesturemanager_p.h>
#include <qinputdevicemanager_p.h>
#include <qkeymapper_p.h>
#include <qopenglcontext_p.h>
#include <qplatform_inputcontext_p.h>
#include <qscreen_p.h>
#include <qstylesheetstyle_p.h>
#include <qstyle_p.h>
#include <qplatform_integrationfactory_p.h>
#include <qplatform_themefactory_p.h>
#include <qthread_p.h>
#include <qvariantgui_p.h>
#include <qwidget_p.h>
#include <qwindowsysteminterface_p.h>
#include <qwindow_p.h>
#include <qwidgetwindow_p.h>

#ifndef QT_NO_CURSOR
#include <qplatform_cursor.h>
#endif

#if defined(Q_OS_DARWIN)
#include <qcore_mac_p.h>

#elif defined(Q_OS_WIN)
#include <qt_windows.h>

#endif

#include <ctype.h>
#include <stdlib.h>

// helper macro for static functions to check on the existence of the application class.
#define CHECK_QAPP_INSTANCE(...) \
    if (QCoreApplication::instance()) { \
    } else { \
        qWarning("QApplication must be started before calling this method"); \
        return __VA_ARGS__; \
    }

Qt::ApplicationState  QApplicationPrivate::applicationState  = Qt::ApplicationInactive;
Qt::KeyboardModifiers QApplicationPrivate::modifier_buttons  = Qt::NoModifier;

Qt::MouseButtons QApplicationPrivate::tabletState      = Qt::NoButton;
Qt::MouseButtons QApplicationPrivate::buttons          = Qt::NoButton;
Qt::MouseButtons QApplicationPrivate::mouse_buttons    = Qt::NoButton;
Qt::MouseButton  QApplicationPrivate::mousePressButton = Qt::NoButton;

bool  QApplicationPrivate::scrollNoPhaseAllowed        = false;
bool  QApplicationPrivate::obey_desktop_settings       = true;
bool  QApplicationPrivate::highDpiScalingUpdated       = false;

int   QApplicationPrivate::mousePressX                 = 0;
int   QApplicationPrivate::mousePressY                 = 0;
int   QApplicationPrivate::mouse_double_click_distance = -1;
int   QApplicationPrivate::m_fakeMouseSourcePointId    = 0;
ulong QApplicationPrivate::mousePressTime              = 0;

QPointF     QApplicationPrivate::lastCursorPosition    = QPointF(qInf(), qInf());

QString     QApplicationPrivate::styleOverride;
QWindowList QApplicationPrivate::window_list;

QList<QObject *> QApplicationPrivate::generic_plugin_list;
QList<QScreen *> QApplicationPrivate::screen_list;

QFont    *QApplicationPrivate::app_font                = nullptr;
QString  *QApplicationPrivate::platform_name           = nullptr;
QString  *QApplicationPrivate::displayName             = nullptr;
QIcon    *QApplicationPrivate::app_icon                = nullptr;
QPalette *QApplicationPrivate::app_palette             = nullptr;
QWindow  *QApplicationPrivate::tabletPressTarget       = nullptr;
QWindow  *QApplicationPrivate::currentMouseWindow      = nullptr;
QWindow  *QApplicationPrivate::currentMousePressWindow = nullptr;
QWindow  *QApplicationPrivate::focus_window            = nullptr;

QGuiApplicationPrivate *QApplicationPrivate::self                 = nullptr;
QInputDeviceManager    *QApplicationPrivate::m_inputDeviceManager = nullptr;
QPlatformIntegration   *QApplicationPrivate::platform_integration = nullptr;
QPlatformTheme         *QApplicationPrivate::platform_theme       = nullptr;
QStyleHints            *QApplicationPrivate::styleHints           = nullptr;
QTouchDevice           *QApplicationPrivate::m_fakeTouchDevice    = nullptr;

#ifndef QT_NO_CLIPBOARD
   QClipboard *QApplicationPrivate::qt_clipboard = nullptr;
#endif

#ifndef QT_NO_SESSIONMANAGER
   bool QApplicationPrivate::is_fallback_session_management_enabled = true;
#endif

QDesktopWidget *qt_desktopWidget  = nullptr;       // root window widgets

enum ApplicationResourceFlags {
   ApplicationPaletteExplicitlySet = 0x1,
   ApplicationFontExplicitlySet    = 0x2
};

enum ApplicationMouseFlags {
   MouseCapsMask       = 0xFF,
   MouseSourceMaskDst  = 0xFF00,
   MouseSourceMaskSrc  = MouseCapsMask,
   MouseSourceShift    = 8,
   MouseFlagsCapsMask  = 0xFF0000,
   MouseFlagsShift     = 16
};

static bool force_reverse                = false;
static unsigned applicationResourceFlags = 0;
static qreal fontSmoothingGamma          = 1.7;

static QMutex applicationFontMutex;
static Qt::LayoutDirection layout_direction = Qt::LayoutDirectionAuto;

void qt_init(QApplicationPrivate *priv, int type);
void qt_init_tooltip_palette();
void qt_cleanupFontDatabase();
void qt_cleanup();

Q_CORE_EXPORT void qt_call_post_routines();

#if ! defined(QT_NO_STATEMACHINE)
   int qRegisterGuiStateMachine();
   int qUnregisterGuiStateMachine();
#endif

PaletteHash *cs_app_palettes_hash();
FontHash *cs_app_fonts_hash();

// set up for variant system, animations
void cs_addGuiFormulas();

static bool qt_detectRTLLanguage()
{
   return force_reverse ^
      (QApplication::tr("QT_LAYOUT_DIRECTION",
            "Translate this string to the string 'LTR' in left-to-right"
            " languages or to 'RTL' in right-to-left languages (such as Hebrew"
            " and Arabic) to get proper widget layout.") == "RTL");
}

static void initPalette()
{
   if (! QGuiApplicationPrivate::app_palette) {
      if (const QPalette *themePalette = QGuiApplicationPrivate::platformTheme()->palette()) {
         QGuiApplicationPrivate::app_palette = new QPalette(*themePalette);
      }
   }

   if (! QGuiApplicationPrivate::app_palette) {
      QGuiApplicationPrivate::app_palette = new QPalette(Qt::gray);
   }
}

static void initResources()
{
   Q_INIT_RESOURCE(qstyle);
   Q_INIT_RESOURCE(qmessagebox);
}

static void initSystemPalette()
{
   if (! QApplicationPrivate::sys_palette) {
      QPalette defaultPlatte;
      if (QApplicationPrivate::app_style) {
         defaultPlatte = QApplicationPrivate::app_style->standardPalette();
      }

      if (const QPalette *themePalette = QGuiApplicationPrivate::platformTheme()->palette()) {
         QApplicationPrivate::setSystemPalette(themePalette->resolve(defaultPlatte));
         QApplicationPrivate::initializeWidgetPaletteHash();
      } else {
         QApplicationPrivate::setSystemPalette(defaultPlatte);
      }
   }
}

static void clearPalette()
{
   delete QGuiApplicationPrivate::app_palette;
   QGuiApplicationPrivate::app_palette = nullptr;
}

static void clearSystemPalette()
{
   delete QApplicationPrivate::sys_palette;
   QApplicationPrivate::sys_palette = nullptr;
}

static void initFontUnlocked()
{
   if (! QGuiApplicationPrivate::app_font) {
      if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme())
         if (const QFont *font = theme->font(QPlatformTheme::SystemFont)) {
            QGuiApplicationPrivate::app_font = new QFont(*font);
         }
   }

   if (! QGuiApplicationPrivate::app_font) {
      QGuiApplicationPrivate::app_font =
         new QFont(QGuiApplicationPrivate::platformIntegration()->fontDatabase()->defaultFont());
   }
}

static inline void clearFontUnlocked()
{
   delete QGuiApplicationPrivate::app_font;
   QGuiApplicationPrivate::app_font = nullptr;
}

static QString get_style_class_name()
{
   QScopedPointer<QStyle> s(QStyleFactory::create(QApplicationPrivate::desktopStyleKey()));

   if (! s.isNull()) {
      return s->metaObject()->className();
   }

   return QString();
}

static QString nativeStyleClassName()
{
   static QString name = get_style_class_name();
   return name;
}

static void updateBlockedStatusRecursion(QWindow *window, bool shouldBeBlocked)
{
   QWindowPrivate *p = qt_window_private(window);

   if (p->blockedByModalWindow != shouldBeBlocked) {
      p->blockedByModalWindow = shouldBeBlocked;
      QEvent e(shouldBeBlocked ? QEvent::WindowBlocked : QEvent::WindowUnblocked);
      QGuiApplication::sendEvent(window, &e);

      for (QObject *c : window->children())
         if (c->isWindowType()) {
            updateBlockedStatusRecursion(static_cast<QWindow *>(c), shouldBeBlocked);
         }
   }
}

// Using aggregate initialization instead of ctor so we can have a POD global static
#define Q_WINDOW_GEOMETRY_SPECIFICATION_INITIALIZER { Qt::TopLeftCorner, -1, -1, -1, -1 }

// Geometry specification for top level windows following the convention of the
// -geometry command line arguments in X11 (see XParseGeometry).
struct QWindowGeometrySpecification {
   static QWindowGeometrySpecification fromArgument(const QByteArray &a);
   void applyTo(QWindow *window) const;

   Qt::Corner corner;
   int xOffset;
   int yOffset;
   int width;
   int height;
};

// Parse a token of a X11 geometry specification "200x100+10-20".
static inline int nextGeometryToken(const QByteArray &a, int &pos, char *op)
{
   *op = 0;
   const int size = a.size();
   if (pos >= size) {
      return -1;
   }

   *op = a.at(pos);
   if (*op == '+' || *op == '-' || *op == 'x') {
      pos++;
   } else if (isdigit(*op)) {
      *op = 'x';   // If it starts with a digit, it is supposed to be a width specification.
   } else {
      return -1;
   }

   const int numberPos = pos;
   for ( ; pos < size && isdigit(a.at(pos)); ++pos) ;

   bool ok;
   const int result = a.mid(numberPos, pos - numberPos).toInt(&ok);
   return ok ? result : -1;
}

QWindowGeometrySpecification QWindowGeometrySpecification::fromArgument(const QByteArray &a)
{
   QWindowGeometrySpecification result = Q_WINDOW_GEOMETRY_SPECIFICATION_INITIALIZER;
   int pos = 0;
   for (int i = 0; i < 4; ++i) {
      char op;
      const int value = nextGeometryToken(a, pos, &op);
      if (value < 0) {
         break;
      }
      switch (op) {
         case 'x':
            (result.width >= 0 ? result.height : result.width) = value;
            break;
         case '+':
         case '-':
            if (result.xOffset >= 0) {
               result.yOffset = value;
               if (op == '-') {
                  result.corner = result.corner == Qt::TopRightCorner ? Qt::BottomRightCorner : Qt::BottomLeftCorner;
               }
            } else {
               result.xOffset = value;
               if (op == '-') {
                  result.corner = Qt::TopRightCorner;
               }
            }
      }
   }
   return result;
}

void QWindowGeometrySpecification::applyTo(QWindow *window) const
{
   QRect windowGeometry = window->frameGeometry();
   QSize size = windowGeometry.size();

   if (width >= 0 || height >= 0) {
      const QSize windowMinimumSize = window->minimumSize();
      const QSize windowMaximumSize = window->maximumSize();
      if (width >= 0) {
         size.setWidth(qBound(windowMinimumSize.width(), width, windowMaximumSize.width()));
      }
      if (height >= 0) {
         size.setHeight(qBound(windowMinimumSize.height(), height, windowMaximumSize.height()));
      }
      window->resize(size);
   }

   if (xOffset >= 0 || yOffset >= 0) {
      const QRect availableGeometry = window->screen()->virtualGeometry();
      QPoint topLeft = windowGeometry.topLeft();
      if (xOffset >= 0) {
         topLeft.setX(corner == Qt::TopLeftCorner || corner == Qt::BottomLeftCorner ?
            xOffset :
            qMax(availableGeometry.right() - size.width() - xOffset, availableGeometry.left()));
      }
      if (yOffset >= 0) {
         topLeft.setY(corner == Qt::TopLeftCorner || corner == Qt::TopRightCorner ?
            yOffset :
            qMax(availableGeometry.bottom() - size.height() - yOffset, availableGeometry.top()));
      }
      window->setFramePosition(topLeft);
   }
}

static QWindowGeometrySpecification windowGeometrySpecification = Q_WINDOW_GEOMETRY_SPECIFICATION_INITIALIZER;


QApplication::QApplication(int &argc, char **argv, int flags)
   : QCoreApplication(*new QGuiApplicationPrivate(argc, argv, flags))
{
   Q_D(QApplication);

   d->init();
   QCoreApplicationPrivate::eventDispatcher->startingUp();
}

QApplication::QApplication(QGuiApplicationPrivate &p)
   : QCoreApplication(p)
{
}

QApplication::~QApplication()
{
   Q_D(QGuiApplication);

   //### this should probable be done even later
   qt_call_post_routines();

   // kill timers before closing down the dispatcher
   d->toolTipWakeUp.stop();
   d->toolTipFallAsleep.stop();

   QApplicationPrivate::is_app_closing = true;
   QApplicationPrivate::is_app_running = false;

   delete QWidgetPrivate::mapper;
   QWidgetPrivate::mapper = nullptr;

   // delete all widgets
   if (QWidgetPrivate::allWidgets) {
      QWidgetSet *mySet = QWidgetPrivate::allWidgets;
      QWidgetPrivate::allWidgets = nullptr;

      for (QWidgetSet::const_iterator it = mySet->constBegin(), cend = mySet->constEnd(); it != cend; ++it) {

         QWidget *w = *it;

         if (! w->parent()) {
            w->destroy(true, true);
         }
      }

      delete mySet;
   }

   delete qt_desktopWidget;
   qt_desktopWidget = nullptr;

   delete QApplicationPrivate::app_palette;
   QApplicationPrivate::app_palette = nullptr;
   clearSystemPalette();

   delete QApplicationPrivate::set_palette;
   QApplicationPrivate::set_palette = nullptr;
   cs_app_palettes_hash()->clear();

   delete QApplicationPrivate::sys_font;
   QApplicationPrivate::sys_font = nullptr;

   delete QApplicationPrivate::set_font;
   QApplicationPrivate::set_font = nullptr;
   cs_app_fonts_hash()->clear();

   delete QApplicationPrivate::app_style;
   QApplicationPrivate::app_style = nullptr;

#ifndef QT_NO_DRAGANDDROP
   if (cs_isRealGuiApp()) {
      delete QDragManager::self();
   }
#endif

   d->cleanupMultitouch();
   qt_cleanup();

   QApplicationPrivate::obey_desktop_settings = true;

   QApplicationPrivate::app_strut = QSize(0, 0);
   QApplicationPrivate::enabledAnimations = QPlatformTheme::GeneralUiEffect;

#if ! defined(QT_NO_STATEMACHINE)
   // trigger unregistering of QStateMachine's GUI types
   qUnregisterGuiStateMachine();
#endif

   d->eventDispatcher->closingDown();
   d->eventDispatcher = nullptr;

#ifndef QT_NO_CLIPBOARD
   delete QGuiApplicationPrivate::qt_clipboard;
   QGuiApplicationPrivate::qt_clipboard = nullptr;
#endif

#ifndef QT_NO_SESSIONMANAGER
   delete d->session_manager;
   d->session_manager = nullptr;
#endif

   clearPalette();
   QFontDatabase::removeAllApplicationFonts();

#ifndef QT_NO_CURSOR
   d->cursor_list.clear();
#endif

   delete QGuiApplicationPrivate::app_icon;
   QGuiApplicationPrivate::app_icon = nullptr;

   delete QGuiApplicationPrivate::platform_name;
   QGuiApplicationPrivate::platform_name = nullptr;

   delete QGuiApplicationPrivate::displayName;
   QGuiApplicationPrivate::displayName = nullptr;

   delete QGuiApplicationPrivate::m_inputDeviceManager;
   QGuiApplicationPrivate::m_inputDeviceManager = nullptr;
}

void QApplication::setApplicationDisplayName(const QString &name)
{
   if (!QGuiApplicationPrivate::displayName) {
      QGuiApplicationPrivate::displayName = new QString;
   }

   *QGuiApplicationPrivate::displayName = name;
}

QString QApplication::applicationDisplayName()
{
   return QGuiApplicationPrivate::displayName ? *QGuiApplicationPrivate::displayName : applicationName();
}

QDesktopWidget *QApplication::desktop()
{
   CHECK_QAPP_INSTANCE(nullptr)

   // may not be created yet
   if (! qt_desktopWidget || !( qt_desktopWidget->windowType() == Qt::Desktop)) {
      // reparented away
      qt_desktopWidget = new QDesktopWidget();
   }

   return qt_desktopWidget;
}

QWindow *QApplication::modalWindow()
{
   CHECK_QAPP_INSTANCE(nullptr)

   if (QGuiApplicationPrivate::self->modalWindowList.isEmpty()) {
      return nullptr;
   }

   return QGuiApplicationPrivate::self->modalWindowList.first();
}

QApplicationPrivate::~QApplicationPrivate()
{
   if (self == this) {
      self = nullptr;
   }

   is_app_closing = true;
   is_app_running = false;

   for (int i = 0; i < generic_plugin_list.count(); ++i) {
      delete generic_plugin_list.at(i);
   }

   generic_plugin_list.clear();

   clearFontUnlocked();

   QFont::cleanup();

#ifndef QT_NO_CURSOR
   QCursorData::cleanup();
#endif

   layout_direction = Qt::LeftToRight;

   delete QGuiApplicationPrivate::styleHints;
   QApplicationPrivate::styleHints = nullptr;

   delete inputMethod;

   qt_cleanupFontDatabase();

   QPixmapCache::clear();

#ifndef QT_NO_OPENGL
   if (ownGlobalShareContext) {
      delete qt_gl_global_share_context();
      qt_gl_set_global_share_context(nullptr);
   }
#endif

   platform_integration->destroy();

   delete platform_theme;
   platform_theme = nullptr;

   delete platform_integration;
   platform_integration = nullptr;

   delete m_gammaTables.load();
   window_list.clear();
}

void QGuiApplicationPrivate::updateBlockedStatus(QWindow *window)
{
   bool shouldBeBlocked = false;
   if (!QWindowPrivate::get(window)->isPopup() && !self->modalWindowList.isEmpty()) {
      shouldBeBlocked = self->isWindowBlocked(window);
   }

   updateBlockedStatusRecursion(window, shouldBeBlocked);
}

void QGuiApplicationPrivate::showModalWindow(QWindow *modal)
{
   self->modalWindowList.prepend(modal);

   // Send leave for currently entered window if it should be blocked
   if (currentMouseWindow && !QWindowPrivate::get(currentMouseWindow)->isPopup()) {
      bool shouldBeBlocked = self->isWindowBlocked(currentMouseWindow);
      if (shouldBeBlocked) {
         // Remove the new window from modalWindowList temporarily so leave can go through
         self->modalWindowList.removeFirst();
         QEvent e(QEvent::Leave);
         QGuiApplication::sendEvent(currentMouseWindow, &e);
         currentMouseWindow = nullptr;
         self->modalWindowList.prepend(modal);
      }
   }

   QWindowList windows = QGuiApplication::topLevelWindows();
   for (int i = 0; i < windows.count(); ++i) {
      QWindow *window = windows.at(i);
      if (!window->d_func()->blockedByModalWindow) {
         updateBlockedStatus(window);
      }
   }

   updateBlockedStatus(modal);
}

void QGuiApplicationPrivate::hideModalWindow(QWindow *window)
{
   self->modalWindowList.removeAll(window);

   QWindowList windows = QGuiApplication::topLevelWindows();
   for (int i = 0; i < windows.count(); ++i) {
      QWindow *window = windows.at(i);
      if (window->d_func()->blockedByModalWindow) {
         updateBlockedStatus(window);
      }
   }
}

QWindow *QApplication::focusWindow()
{
   return QGuiApplicationPrivate::focus_window;
}

QObject *QApplication::focusObject()
{
   if (focusWindow()) {
      return focusWindow()->focusObject();
   }

   return nullptr;
}

QWindowList QApplication::allWindows()
{
   return QGuiApplicationPrivate::window_list;
}

QWindowList QApplication::topLevelWindows()
{
   const QWindowList &list = QGuiApplicationPrivate::window_list;
   QWindowList topLevelWindows;

   for (int i = 0; i < list.size(); i++) {
      if (!list.at(i)->parent() && list.at(i)->type() != Qt::Desktop) {
         // Top windows of embedded QAxServers do not have QWindow parents,
         // but they are not true top level windows, so do not include them.
         const bool embedded = list.at(i)->handle() && list.at(i)->handle()->isEmbedded();
         if (!embedded) {
            topLevelWindows.prepend(list.at(i));
         }
      }
   }
   return topLevelWindows;
}

QScreen *QApplication::primaryScreen()
{
   if (QGuiApplicationPrivate::screen_list.isEmpty()) {
      return nullptr;
   }

   return QGuiApplicationPrivate::screen_list.at(0);
}

QList<QScreen *> QApplication::screens()
{
   return QGuiApplicationPrivate::screen_list;
}

qreal QApplication::devicePixelRatio() const
{
   // Cache topDevicePixelRatio, iterate through the screen list once only.
   static qreal topDevicePixelRatio = 0.0;
   if (!qFuzzyIsNull(topDevicePixelRatio)) {
      return topDevicePixelRatio;
   }

   topDevicePixelRatio = 1.0; // make sure we never return 0.
   for (QScreen *screen : QGuiApplicationPrivate::screen_list) {
      topDevicePixelRatio = qMax(topDevicePixelRatio, screen->devicePixelRatio());
   }

   return topDevicePixelRatio;
}

bool QApplication::event(QEvent *e)
{
   Q_D(QApplication);
   if (e->type() == QEvent::Close) {
      QCloseEvent *ce = static_cast<QCloseEvent *>(e);
      ce->accept();
      closeAllWindows();

      QWidgetList list = topLevelWidgets();
      for (int i = 0; i < list.size(); ++i) {
         QWidget *w = list.at(i);
         if (w->isVisible() && !(w->windowType() == Qt::Desktop) && !(w->windowType() == Qt::Popup) &&
            (!(w->windowType() == Qt::Dialog) || !w->parentWidget())) {
            ce->ignore();
            break;
         }
      }

      if (ce->isAccepted()) {
         return true;
      }

#ifndef Q_OS_WIN
   } else if (e->type() == QEvent::LocaleChange) {
      // on Windows the event propagation is taken care by the
      // WM_SETTINGCHANGE event handler.
      QWidgetList list = topLevelWidgets();
      for (int i = 0; i < list.size(); ++i) {
         QWidget *w = list.at(i);
         if (!(w->windowType() == Qt::Desktop)) {
            if (!w->testAttribute(Qt::WA_SetLocale)) {
               w->d_func()->setLocale_helper(QLocale(), true);
            }
         }
      }
#endif

   } else if (e->type() == QEvent::Timer) {
      QTimerEvent *te = static_cast<QTimerEvent *>(e);
      Q_ASSERT(te != nullptr);
      if (te->timerId() == d->toolTipWakeUp.timerId()) {
         d->toolTipWakeUp.stop();

         if (d->toolTipWidget) {
            QWidget *w = d->toolTipWidget->window();
            // show tooltip if WA_AlwaysShowToolTips is set, or if
            // any ancestor of d->toolTipWidget is the active
            // window
            bool showToolTip = w->testAttribute(Qt::WA_AlwaysShowToolTips);
            while (w && !showToolTip) {
               showToolTip = w->isActiveWindow();
               w = w->parentWidget();
               w = w ? w->window() : nullptr;
            }

            if (showToolTip) {
               QHelpEvent e(QEvent::ToolTip, d->toolTipPos, d->toolTipGlobalPos);
               QApplication::sendEvent(d->toolTipWidget, &e);
               if (e.isAccepted()) {
                  QStyle *s = d->toolTipWidget->style();
                  int sleepDelay = s->styleHint(QStyle::SH_ToolTip_FallAsleepDelay, nullptr, d->toolTipWidget, nullptr);
                  d->toolTipFallAsleep.start(sleepDelay, this);
               }
            }
         }

      } else if (te->timerId() == d->toolTipFallAsleep.timerId()) {
         d->toolTipFallAsleep.stop();
      }

#ifndef QT_NO_WHATSTHIS
   } else if (e->type() == QEvent::EnterWhatsThisMode) {
      QWhatsThis::enterWhatsThisMode();
      return true;
#endif
   }

   if (e->type() == QEvent::LanguageChange) {
      QWidgetList list = topLevelWidgets();

      for (int i = 0; i < list.size(); ++i) {
         QWidget *w = list.at(i);
         if (!(w->windowType() == Qt::Desktop)) {
            postEvent(w, new QEvent(QEvent::LanguageChange));
         }
      }
   }

   if (e->type() == QEvent::LanguageChange) {
      setLayoutDirection(qt_detectRTLLanguage() ? Qt::RightToLeft : Qt::LeftToRight);
   }

   return QCoreApplication::event(e);
}

QWindow *QApplication::topLevelWindowAt(const QPoint &pos)
{
   const QList<QScreen *> screens = QGuiApplication::screens();

   if (! screens.isEmpty()) {
      const QList<QScreen *> primaryScreens = screens.first()->virtualSiblings();
      QScreen *windowScreen = nullptr;

      // Find the window on the primary virtual desktop first
      for (QScreen *screen : primaryScreens) {
         if (screen->geometry().contains(pos)) {
            windowScreen = screen;
            break;
         }
      }

      // If the window is not found on primary virtual desktop, find it on all screens
      // except the first which was for sure in the previous loop. Some other screens
      // may repeat. Find only when there is more than one virtual desktop.

      if (! windowScreen && screens.count() != primaryScreens.count()) {
         for (int i = 1; i < screens.size(); ++i) {
            QScreen *screen = screens[i];
            if (screen->geometry().contains(pos)) {
               windowScreen = screen;
               break;
            }
         }
      }

      if (windowScreen) {
         const QPoint devicePosition = QHighDpi::toNativePixels(pos, windowScreen);
         return windowScreen->handle()->topLevelWindowAt(devicePosition);
      }
   }

   return nullptr;
}

QString QApplication::platformName()
{
   return QGuiApplicationPrivate::platform_name ?
      *QGuiApplicationPrivate::platform_name : QString();
}

static void init_platform(const QString &pluginArgument, const QString &platformPluginPath,
                  const QString &platformThemeName, int &argc, char **argv)
{
   // Split into platform arguments and key
   QStringList arguments   = pluginArgument.split(':');
   const QString pluginKey = arguments.takeFirst().toLower();

   // look up the arguments in system settings
   arguments.append(QLibraryInfo::platformPluginArguments(pluginKey));

   // load the platform plugin
   QGuiApplicationPrivate::platform_integration = QPlatformIntegrationFactory::create(pluginKey, arguments,
         argc, argv, platformPluginPath);

   if (QGuiApplicationPrivate::platform_integration != nullptr) {
      QGuiApplicationPrivate::platform_name = new QString(pluginKey);

   } else {
      QStringList keys = QPlatformIntegrationFactory::keys(platformPluginPath);

      QString fatalMessage = QString("The application failed to start because the platform plugin was "
            "not found or did not load.\nRequested Plugin Key: \"%1\"\n\n").formatArg(pluginKey);

      if (! keys.isEmpty()) {
         fatalMessage += QString("Available platform plugins: %1.\n\n").formatArg(keys.join(", "));
      }

      fatalMessage += "Reinstalling the application may resolve this problem.";

#if defined(Q_OS_WIN)
      // display the message box unless it is a console application or debug build showing an assert box

      if (! GetConsoleWindow()) {
         MessageBox(nullptr, &fatalMessage.toStdWString()[0],
            &QCoreApplication::applicationName().toStdWString()[0], MB_OK | MB_ICONERROR);
      }
#endif

      qFatal("%s", csPrintable(fatalMessage));
      return;
   }

   // Many platforms have created QScreens at this point. Finish initializing QHighDpiScaling
   // to be prepared for early calls to qt_defaultDpi().
   if (QGuiApplication::primaryScreen()) {
      QGuiApplicationPrivate::highDpiScalingUpdated = true;
      QHighDpiScaling::updateHighDpiScaling();
   }

   // Create the platform theme:

   // (1) Fetch the platform name from the environment if present
   QStringList themeNames;
   if (! platformThemeName.isEmpty()) {
      themeNames.append(platformThemeName);
   }

   // (2) Ask the platform integration for a list of theme names
   themeNames += QGuiApplicationPrivate::platform_integration->themeNames();

   // (3) Look for a theme plugin
   for (const QString &themeName : themeNames) {
      QGuiApplicationPrivate::platform_theme = QPlatformThemeFactory::create(themeName, platformPluginPath);

      if (QGuiApplicationPrivate::platform_theme) {
         break;
      }
   }

   // (4) If no theme plugin was found ask the platform integration to create a theme
   if (! QGuiApplicationPrivate::platform_theme) {

      for (const QString &themeName : themeNames) {
         QGuiApplicationPrivate::platform_theme =
                  QGuiApplicationPrivate::platform_integration->createPlatformTheme(themeName);

         if (QGuiApplicationPrivate::platform_theme) {
            break;
         }
      }

      // No error message, not having a theme plugin is allowed
   }

   // (5) Fall back on the built-in "null" platform theme.
   if (! QGuiApplicationPrivate::platform_theme) {
      QGuiApplicationPrivate::platform_theme = new QPlatformTheme;
   }

#ifndef QT_NO_PROPERTIES
   // Set arguments as dynamic properties on the native interface as
   // boolean 'foo' or strings: 'foo=bar'

   if (! arguments.isEmpty()) {
      if (QObject *nativeInterface = QGuiApplicationPrivate::platform_integration->nativeInterface()) {

         for (const QString &argument : arguments) {
            const int equalsPos = argument.indexOf(QChar('='));

            const QString name   = equalsPos != -1 ? argument.left(equalsPos) : argument;
            const QVariant value = equalsPos != -1 ? QVariant(argument.mid(equalsPos + 1)) : QVariant(true);

            nativeInterface->setProperty(name, value);
         }
      }
   }
#endif

   fontSmoothingGamma = QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::FontSmoothingGamma).toReal();
}

static void init_plugins(const QList<QString> &pluginList)
{
   for (int i = 0; i < pluginList.count(); ++i) {
      QString pluginSpec = pluginList.at(i);

      int colonPos = pluginSpec.indexOf(':');
      QObject *plugin;

      if (colonPos < 0) {
         plugin = QGenericPluginFactory::create(pluginSpec, QString());

      } else {
         plugin = QGenericPluginFactory::create(pluginSpec.mid(0, colonPos), pluginSpec.mid(colonPos + 1));
      }

      if (plugin) {
         QGuiApplicationPrivate::generic_plugin_list.append(plugin);

      } else {
         qWarning() << "QApplication::init_plugins() " << "Plugin failed to load, " << pluginSpec;
      }
   }
}

void QGuiApplicationPrivate::createPlatformIntegration()
{
   // Use the CS menus by default. Platform plugins that want to enable a native
   // menu implementation can clear this flag.
   QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, true);

   QHighDpiScaling::initHighDpiScaling();

   //
   QString platformName;

#if defined(Q_OS_WIN)
   platformName = "windows";

#elif defined(Q_OS_DARWIN)
   platformName = "cocoa";

#else
   platformName = "xcb";

#endif

   // allow the plugin name to be changed
   QString platformNameEnv = QString::fromUtf8(qgetenv("QT_QPA_PLATFORM"));
   if (! platformNameEnv.isEmpty()) {
      platformName = platformNameEnv;
   }

   QString platformPluginPath = QString::fromUtf8(qgetenv("QT_QPA_PLATFORM_PLUGIN_PATH"));
   QString platformThemeName  = QString::fromUtf8(qgetenv("QT_QPA_PLATFORMTHEME"));

   // Get command line parameters
   QString icon;

   int j = argc ? 1 : 0;
   for (int i = 1; i < argc; i++) {
      if (! argv[i]) {
         continue;
      }

      if (*argv[i] != '-') {
         argv[j++] = argv[i];
         continue;
      }

      const bool isXcb = (platformName.startsWith("CsGuiXcb"));

      //
      QString arg = QString::fromUtf8(argv[i]);

      if (arg.startsWith("--")) {
         arg = arg.mid(1);
      }

      if (arg == "-platformpluginpath") {
         if (++i < argc) {
            platformPluginPath = QString::fromUtf8(argv[i]);
         }

      } else if (arg == "-platform") {
         if (++i < argc) {
            platformName = QString::fromUtf8(argv[i]);
         }

      } else if (arg == "-platformtheme") {
         if (++i < argc) {
            platformThemeName = QString::fromUtf8(argv[i]);
         }

      } else if (arg == "-qwindowgeometry" || (isXcb && arg == "-geometry")) {
         if (++i < argc) {
            windowGeometrySpecification = QWindowGeometrySpecification::fromArgument(argv[i]);
         }

      } else if (arg == "-qwindowtitle" || (isXcb && arg == "-title")) {
         if (++i < argc) {
            firstWindowTitle = QString::fromUtf8(argv[i]);
         }

      } else if (arg == "-qwindowicon" || (isXcb && arg == "-icon")) {
         if (++i < argc) {
            icon = QString::fromUtf8(argv[i]);
         }

      } else {
         argv[j++] = argv[i];
      }
   }

   if (j < argc) {
      argv[j] = nullptr;
      argc = j;
   }

   init_platform(platformName, platformPluginPath, platformThemeName, argc, argv);

   if (! icon.isEmpty()) {
      forcedWindowIcon = QDir::isAbsolutePath(icon) ? QIcon(icon) : QIcon::fromTheme(icon);
   }
}

void QGuiApplicationPrivate::createEventDispatcher()
{
   Q_ASSERT(! eventDispatcher);

   if (platform_integration == nullptr) {
      createPlatformIntegration();
   }

   //  platform integration should not mess with the event dispatcher
   Q_ASSERT(! eventDispatcher);

   eventDispatcher = platform_integration->createEventDispatcher();
}

void QGuiApplicationPrivate::eventDispatcherReady()
{
   if (platform_integration == nullptr) {
      createPlatformIntegration();
   }

   platform_integration->initialize();

   // All platforms should have added screens at this point
   // Finish QHighDpiScaling initialization if it has not been done so already
   if (! QGuiApplicationPrivate::highDpiScalingUpdated) {
      QHighDpiScaling::updateHighDpiScaling();
   }
}

void QGuiApplicationPrivate::init()
{
   QCoreApplicationPrivate::init();

   QCoreApplicationPrivate::is_app_running = false;
   QList<QString> pluginList;

   // Get command line params
#ifndef QT_NO_SESSIONMANAGER
   QString session_id;
   QString session_key;

# if defined(Q_OS_WIN)
   std::wstring guidstr(40, L'\0');
   GUID guid;
   CoCreateGuid(&guid);

   StringFromGUID2(guid, &guidstr[0], 40);
   session_id = QString::fromStdWString(guidstr);

   CoCreateGuid(&guid);
   StringFromGUID2(guid, &guidstr[0], 40);
   session_key = QString::fromStdWString(guidstr);
# endif

#endif

   QString s;
   int j = argc ? 1 : 0;

   for (int i = 1; i < argc; i++) {

      if (! argv[i]) {
         continue;
      }

      if (*argv[i] != '-') {
         argv[j++] = argv[i];
         continue;
      }

      QString arg = QString::fromUtf8(argv[i]);

      if (arg.startsWith("--")) {
         arg = arg.mid(1);
      }

      if (arg == "-plugin") {
         if (++i < argc) {
            pluginList << QString::fromUtf8(argv[i]);
         }

      } else if (arg == "-reverse") {
         force_reverse = true;

#ifdef Q_OS_DARWIN
      } else if (arg.startsWith("-psn_")) {
         // consume "-psn_xxxx" on Mac, which is passed when starting an app from Finder
         // used to change the working directory (for an app bundle) when running from finder

         if (QDir::currentPath() == "/") {
            QCFType<CFURLRef> bundleURL(CFBundleCopyBundleURL(CFBundleGetMainBundle()));
            QString qbundlePath = QCFString(CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle)).toQString();

            if (qbundlePath.endsWith(".app")) {
               QDir::setCurrent(qbundlePath.section('/', 0, -2));
            }
         }
#endif

#ifndef QT_NO_SESSIONMANAGER
      } else if (arg == "-session" && i < argc - 1) {
         ++i;

         if (argv[i] && *argv[i]) {
            session_id = QString::fromUtf8(argv[i]);

            int p = session_id.indexOf('_');

            if (p >= 0) {
               session_key = session_id.mid(p + 1);
               session_id  = session_id.left(p);
            }

            is_session_restored = true;
         }
#endif

      } else if (arg.startsWith("-style=")) {
         s = arg.mid(7).toLower();

      } else if (arg.startsWith("-style") && i < argc - 1) {
         s = QString::fromUtf8(argv[++i]).toLower();

      } else {
         argv[j++] = argv[i];
      }

      if (! s.isEmpty()) {
         styleOverride = s;
      }
   }

   if (j < argc) {
      argv[j] = nullptr;
      argc = j;
   }

   // Load environment exported generic plugins
   QByteArray envPlugins = qgetenv("QT_QPA_GENERIC_PLUGINS");

   if (! envPlugins.isEmpty()) {
      for (const QByteArray &plugin : envPlugins.split(',')) {
         pluginList << plugin;
      }
   }

   if (platform_integration == nullptr) {
      createPlatformIntegration();
   }

   initPalette();
   QFont::initialize();

   mouse_double_click_distance = platformTheme()->themeHint(QPlatformTheme::MouseDoubleClickDistance).toInt();

#ifndef QT_NO_CURSOR
   QCursorData::initialize();
#endif

   // add gui to the variant system
   static QVariantGui objVariant;
   QVariant::registerClient(&objVariant);

   // set up for variant system, animations
   cs_addGuiFormulas();

// set a global share context when enabled unless there is already one
#ifndef QT_NO_OPENGL
   if (qApp->testAttribute(Qt::AA_ShareOpenGLContexts) && ! qt_gl_global_share_context()) {
      QOpenGLContext *ctx = new QOpenGLContext;
      ctx->setFormat(QSurfaceFormat::defaultFormat());
      ctx->create();

      qt_gl_set_global_share_context(ctx);
      ownGlobalShareContext = true;
   }
#endif

   QWindowSystemInterfacePrivate::eventTime.start();

   is_app_running = true;
   init_plugins(pluginList);
   QWindowSystemInterface::flushWindowSystemEvents();

#ifndef QT_NO_SESSIONMANAGER
   Q_Q(QGuiApplication);

   // connect to the session manager
   session_manager = new QSessionManager(q, session_id, session_key);
#endif

   if (layout_direction == Qt::LayoutDirectionAuto || force_reverse) {
      QGuiApplication::setLayoutDirection(qt_detectRTLLanguage() ? Qt::RightToLeft : Qt::LeftToRight);
   }

   scrollNoPhaseAllowed = ! qgetenv("QT_ENABLE_MOUSE_WHEEL_TRACKING").isEmpty();

   initResources();
   process_cmdline();

   // must be called before initialize()
   qt_init(this, application_type);
   initialize();
   eventDispatcher->startingUp();

#ifndef QT_NO_ACCESSIBILITY
   // factory for accessible interfaces for widgets shipped with Qt
   QAccessible::installFactory(&qAccessibleFactory);
#endif

}

Qt::KeyboardModifiers QApplication::keyboardModifiers()
{
   return QGuiApplicationPrivate::modifier_buttons;
}

Qt::KeyboardModifiers QApplication::queryKeyboardModifiers()
{
   CHECK_QAPP_INSTANCE(Qt::KeyboardModifiers(Qt::EmptyFlag))
   QPlatformIntegration *pi = QGuiApplicationPrivate::platformIntegration();

   return pi->queryKeyboardModifiers();
}

Qt::MouseButtons QApplication::mouseButtons()
{
   return QGuiApplicationPrivate::mouse_buttons;
}

QPlatformNativeInterface *QApplication::platformNativeInterface()
{
   QPlatformIntegration *platform_interface = QGuiApplicationPrivate::platformIntegration();
   return platform_interface ? platform_interface->nativeInterface() : nullptr;
}

QApplication::FP_Void QApplication::platformFunction(const QByteArray &function)
{
   QPlatformIntegration *platform_interface = QGuiApplicationPrivate::platformIntegration();

   if (! platform_interface) {
      qWarning("QApplication::platformFunction() QApplication must be started before accessing platform functions");
      return nullptr;
   }

   return platform_interface->nativeInterface() ? platform_interface->nativeInterface()->platformFunction(function) : nullptr;
}

int QApplication::exec()
{
#ifndef QT_NO_ACCESSIBILITY
   QAccessible::setRootObject(qApp);
#endif
   return QCoreApplication::exec();
}

void QGuiApplicationPrivate::sendQWindowEventToQPlatformWindow(QWindow *window, QEvent *event)
{
   if (!window) {
      return;
   }
   QPlatformWindow *platformWindow = window->handle();
   if (!platformWindow) {
      return;
   }
   // spontaneous events come from the platform integration already, we don't need to send the events back
   if (event->spontaneous()) {
      return;
   }
   // let the platform window do any handling it needs to as well
   platformWindow->windowEvent(event);
}

bool QGuiApplicationPrivate::processNativeEvent(QWindow *window, const QByteArray &eventType, void *message, long *result)
{
   return window->nativeEvent(eventType, message, result);
}

void QGuiApplicationPrivate::processWindowSystemEvent(QWindowSystemInterfacePrivate::WindowSystemEvent *e)
{
   switch (e->type) {
      case QWindowSystemInterfacePrivate::FrameStrutMouse:
      case QWindowSystemInterfacePrivate::Mouse:
         QGuiApplicationPrivate::processMouseEvent(static_cast<QWindowSystemInterfacePrivate::MouseEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::Wheel:
         QGuiApplicationPrivate::processWheelEvent(static_cast<QWindowSystemInterfacePrivate::WheelEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::Key:
         QGuiApplicationPrivate::processKeyEvent(static_cast<QWindowSystemInterfacePrivate::KeyEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::Touch:
         QGuiApplicationPrivate::processTouchEvent(static_cast<QWindowSystemInterfacePrivate::TouchEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::GeometryChange:
         QGuiApplicationPrivate::processGeometryChangeEvent(static_cast<QWindowSystemInterfacePrivate::GeometryChangeEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::Enter:
         QGuiApplicationPrivate::processEnterEvent(static_cast<QWindowSystemInterfacePrivate::EnterEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::Leave:
         QGuiApplicationPrivate::processLeaveEvent(static_cast<QWindowSystemInterfacePrivate::LeaveEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::ActivatedWindow:
         QGuiApplicationPrivate::processActivatedEvent(static_cast<QWindowSystemInterfacePrivate::ActivatedWindowEvent *>(e));
         break;
      case QWindowSystemInterfacePrivate::WindowStateChanged:
         QGuiApplicationPrivate::processWindowStateChangedEvent(static_cast<QWindowSystemInterfacePrivate::WindowStateChangedEvent *>(e));
         break;
      case QWindowSystemInterfacePrivate::WindowScreenChanged:
         QGuiApplicationPrivate::processWindowScreenChangedEvent(static_cast<QWindowSystemInterfacePrivate::WindowScreenChangedEvent *>(e));
         break;
      case QWindowSystemInterfacePrivate::ApplicationStateChanged: {
         QWindowSystemInterfacePrivate::ApplicationStateChangedEvent *changeEvent =
            static_cast<QWindowSystemInterfacePrivate::ApplicationStateChangedEvent *>(e);
         QGuiApplicationPrivate::setApplicationState(changeEvent->newState, changeEvent->forcePropagate);
      }
      break;

      case QWindowSystemInterfacePrivate::FlushEvents: {
         QWindowSystemInterfacePrivate::FlushEventsEvent *flushEventsEvent = static_cast<QWindowSystemInterfacePrivate::FlushEventsEvent *>(e);
         QWindowSystemInterface::deferredFlushWindowSystemEvents(flushEventsEvent->flags);
      }
      break;

      case QWindowSystemInterfacePrivate::Close:
         QGuiApplicationPrivate::processCloseEvent(
            static_cast<QWindowSystemInterfacePrivate::CloseEvent *>(e));
         break;
      case QWindowSystemInterfacePrivate::ScreenOrientation:
         QGuiApplicationPrivate::reportScreenOrientationChange(
            static_cast<QWindowSystemInterfacePrivate::ScreenOrientationEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::ScreenGeometry:
         QGuiApplicationPrivate::reportGeometryChange(
            static_cast<QWindowSystemInterfacePrivate::ScreenGeometryEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::ScreenLogicalDotsPerInch:
         QGuiApplicationPrivate::reportLogicalDotsPerInchChange(
            static_cast<QWindowSystemInterfacePrivate::ScreenLogicalDotsPerInchEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::ScreenRefreshRate:
         QGuiApplicationPrivate::reportRefreshRateChange(
            static_cast<QWindowSystemInterfacePrivate::ScreenRefreshRateEvent *>(e));
         break;
      case QWindowSystemInterfacePrivate::ThemeChange:
         QGuiApplicationPrivate::processThemeChanged(
            static_cast<QWindowSystemInterfacePrivate::ThemeChangeEvent *>(e));
         break;
      case QWindowSystemInterfacePrivate::Expose:
         QGuiApplicationPrivate::processExposeEvent(static_cast<QWindowSystemInterfacePrivate::ExposeEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::Tablet:
         QGuiApplicationPrivate::processTabletEvent(
            static_cast<QWindowSystemInterfacePrivate::TabletEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::TabletEnterProximity:
         QGuiApplicationPrivate::processTabletEnterProximityEvent(
            static_cast<QWindowSystemInterfacePrivate::TabletEnterProximityEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::TabletLeaveProximity:
         QGuiApplicationPrivate::processTabletLeaveProximityEvent(
            static_cast<QWindowSystemInterfacePrivate::TabletLeaveProximityEvent *>(e));
         break;

#ifndef QT_NO_GESTURES
      case QWindowSystemInterfacePrivate::Gesture:
         QGuiApplicationPrivate::processGestureEvent(
            static_cast<QWindowSystemInterfacePrivate::GestureEvent *>(e));
         break;
#endif

      case QWindowSystemInterfacePrivate::PlatformPanel:
         QGuiApplicationPrivate::processPlatformPanelEvent(
            static_cast<QWindowSystemInterfacePrivate::PlatformPanelEvent *>(e));
         break;

      case QWindowSystemInterfacePrivate::FileOpen:
         QGuiApplicationPrivate::processFileOpenEvent(
            static_cast<QWindowSystemInterfacePrivate::FileOpenEvent *>(e));
         break;

#ifndef QT_NO_CONTEXTMENU
      case QWindowSystemInterfacePrivate::ContextMenu:
         QGuiApplicationPrivate::processContextMenuEvent(
            static_cast<QWindowSystemInterfacePrivate::ContextMenuEvent *>(e));
         break;
#endif

      case QWindowSystemInterfacePrivate::EnterWhatsThisMode:
         QGuiApplication::postEvent(QGuiApplication::instance(), new QEvent(QEvent::EnterWhatsThisMode));
         break;

      default:
         qWarning() << "QApplication::processWindowSystemEvent() Unknown user input event type," << e->type;
         break;
   }
}

void QGuiApplicationPrivate::processMouseEvent(QWindowSystemInterfacePrivate::MouseEvent *e)
{
   QEvent::Type type;
   Qt::MouseButtons stateChange = e->buttons ^ buttons;
   if (e->globalPos != QGuiApplicationPrivate::lastCursorPosition && (stateChange != Qt::NoButton)) {
      // A mouse event should not change both position and buttons at the same time. Instead we
      // should first send a move event followed by a button changed event. Since this is not the case
      // with the current event, we split it in two.
      QWindowSystemInterfacePrivate::MouseEvent mouseButtonEvent(
         e->window.data(), e->timestamp, e->type, e->localPos, e->globalPos, e->buttons, e->modifiers, e->source);
      if (e->flags & QWindowSystemInterfacePrivate::WindowSystemEvent::Synthetic) {
         mouseButtonEvent.flags |= QWindowSystemInterfacePrivate::WindowSystemEvent::Synthetic;
      }
      e->buttons = buttons;
      processMouseEvent(e);
      processMouseEvent(&mouseButtonEvent);
      return;
   }

   QWindow *window = e->window.data();
   modifier_buttons = e->modifiers;

   QPointF localPoint = e->localPos;
   QPointF globalPoint = e->globalPos;

   if (e->nullWindow()) {
      window = QGuiApplication::topLevelWindowAt(globalPoint.toPoint());

      if (window) {
         // Moves and the release following a press must go to the same
         // window, even if the cursor has moved on over another window.
         if (e->buttons != Qt::NoButton) {
            if (!currentMousePressWindow) {
               currentMousePressWindow = window;
            } else {
               window = currentMousePressWindow;
            }
         } else if (currentMousePressWindow) {
            window = currentMousePressWindow;
            currentMousePressWindow = nullptr;
         }
         QPointF delta = globalPoint - globalPoint.toPoint();
         localPoint = window->mapFromGlobal(globalPoint.toPoint()) + delta;
      }
   }

   Qt::MouseButton button = Qt::NoButton;
   bool doubleClick = false;
   const bool frameStrut = e->type == QWindowSystemInterfacePrivate::FrameStrutMouse;

   if (QGuiApplicationPrivate::lastCursorPosition != globalPoint) {
      type = frameStrut ? QEvent::NonClientAreaMouseMove : QEvent::MouseMove;
      QGuiApplicationPrivate::lastCursorPosition = globalPoint;
      if (qAbs(globalPoint.x() - mousePressX) > mouse_double_click_distance ||
         qAbs(globalPoint.y() - mousePressY) > mouse_double_click_distance) {
         mousePressButton = Qt::NoButton;
      }
   } else { // Check to see if a new button has been pressed/released.
      for (int check = Qt::LeftButton;
         check <= int(Qt::MaxMouseButton);
         check = check << 1) {
         if (check & stateChange) {
            button = Qt::MouseButton(check);
            break;
         }
      }
      if (button == Qt::NoButton) {
         // Ignore mouse events that don't change the current state.
         return;
      }
      mouse_buttons = buttons = e->buttons;
      if (button & e->buttons) {
         ulong doubleClickInterval = static_cast<ulong>(QGuiApplication::styleHints()->mouseDoubleClickInterval());
         doubleClick = e->timestamp - mousePressTime < doubleClickInterval && button == mousePressButton;
         type = frameStrut ? QEvent::NonClientAreaMouseButtonPress : QEvent::MouseButtonPress;
         mousePressTime = e->timestamp;
         mousePressButton = button;
         const QPoint point = QGuiApplicationPrivate::lastCursorPosition.toPoint();
         mousePressX = point.x();
         mousePressY = point.y();
      } else {
         type = frameStrut ? QEvent::NonClientAreaMouseButtonRelease : QEvent::MouseButtonRelease;
      }
   }

   if (!window) {
      return;
   }

#ifndef QT_NO_CURSOR
   if (!e->synthetic()) {
      if (const QScreen *screen = window->screen())
         if (QPlatformCursor *cursor = screen->handle()->cursor()) {
            const QPointF nativeLocalPoint = QHighDpi::toNativePixels(localPoint, screen);
            const QPointF nativeGlobalPoint = QHighDpi::toNativePixels(globalPoint, screen);
            QMouseEvent ev(type, nativeLocalPoint, nativeLocalPoint, nativeGlobalPoint,
               button, buttons, e->modifiers, e->source);
            ev.setTimestamp(e->timestamp);
            cursor->pointerEvent(ev);
         }
   }
#endif

   QMouseEvent ev(type, localPoint, localPoint, globalPoint, button, buttons, e->modifiers, e->source);
   ev.setTimestamp(e->timestamp);

   if (window->d_func()->blockedByModalWindow && !qApp->d_func()->popupActive()) {
      // a modal window is blocking this window, don't allow mouse events through
      return;
   }

   if (doubleClick && (ev.type() == QEvent::MouseButtonPress)) {
      // QtBUG-25831, used to suppress delivery in qwidgetwindow.cpp
      setMouseEventFlags(&ev, ev.flags() | Qt::MouseEventCreatedDoubleClick);
   }

   QGuiApplication::sendSpontaneousEvent(window, &ev);
   e->eventAccepted = ev.isAccepted();
   if (!e->synthetic() && !ev.isAccepted()
      && !frameStrut
      && qApp->testAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents)) {
      if (!m_fakeTouchDevice) {
         m_fakeTouchDevice = new QTouchDevice;
         QWindowSystemInterface::registerTouchDevice(m_fakeTouchDevice);
      }
      QList<QWindowSystemInterface::TouchPoint> points;
      QWindowSystemInterface::TouchPoint point;
      point.id = 1;
      point.area = QRectF(globalPoint.x() - 2, globalPoint.y() - 2, 4, 4);

      // only translate left button related events to
      // avoid strange touch event sequences when several
      // buttons are pressed
      if (type == QEvent::MouseButtonPress && button == Qt::LeftButton) {
         point.state = Qt::TouchPointPressed;
      } else if (type == QEvent::MouseButtonRelease && button == Qt::LeftButton) {
         point.state = Qt::TouchPointReleased;
      } else if (type == QEvent::MouseMove && (buttons & Qt::LeftButton)) {
         point.state = Qt::TouchPointMoved;
      } else {
         return;
      }

      points << point;

      QEvent::Type type;
      QList<QTouchEvent::TouchPoint> touchPoints = QWindowSystemInterfacePrivate::fromNativeTouchPoints(points, window, &type);

      QWindowSystemInterfacePrivate::TouchEvent fake(window, e->timestamp, type, m_fakeTouchDevice, touchPoints, e->modifiers);
      fake.flags |= QWindowSystemInterfacePrivate::WindowSystemEvent::Synthetic;
      processTouchEvent(&fake);
   }

   if (doubleClick) {
      mousePressButton = Qt::NoButton;

      if (!e->window.isNull() || e->nullWindow()) { // QTBUG-36364, check if window closed in response to press
         const QEvent::Type doubleClickType = frameStrut ? QEvent::NonClientAreaMouseButtonDblClick : QEvent::MouseButtonDblClick;

         QMouseEvent dblClickEvent(doubleClickType, localPoint, localPoint, globalPoint,
            button, buttons, e->modifiers, e->source);

         dblClickEvent.setTimestamp(e->timestamp);
         QGuiApplication::sendSpontaneousEvent(window, &dblClickEvent);
      }
   }
}

void QGuiApplicationPrivate::processWheelEvent(QWindowSystemInterfacePrivate::WheelEvent *e)
{
#ifndef QT_NO_WHEELEVENT
   QWindow *window = e->window.data();
   QPointF globalPoint = e->globalPos;
   QPointF localPoint = e->localPos;

   if (e->nullWindow()) {
      window = QGuiApplication::topLevelWindowAt(globalPoint.toPoint());

      if (window) {
         QPointF delta = globalPoint - globalPoint.toPoint();
         localPoint = window->mapFromGlobal(globalPoint.toPoint()) + delta;
      }
   }

   if (! window) {
      return;
   }

   QGuiApplicationPrivate::lastCursorPosition = globalPoint;
   modifier_buttons = e->modifiers;

   if (window->d_func()->blockedByModalWindow) {
      // a modal window is blocking this window, don't allow wheel events through
      return;
   }

   QWheelEvent ev(localPoint, globalPoint, e->pixelDelta, e->angleDelta, buttons, e->modifiers, e->phase, e->source);

   ev.setTimestamp(e->timestamp);
   QGuiApplication::sendSpontaneousEvent(window, &ev);

#endif
}

void QGuiApplicationPrivate::processKeyEvent(QWindowSystemInterfacePrivate::KeyEvent *e)
{
   QWindow *window = e->window.data();
   modifier_buttons = e->modifiers;

   if (e->nullWindow()

#if defined(Q_OS_ANDROID)
      || e->key == Qt::Key_Back || e->key == Qt::Key_Menu
#endif

   ) {
      window = QGuiApplication::focusWindow();
   }

#if ! defined(Q_OS_DARWIN)
   // FIXME: Include OS X in this code path by passing the key event through
   // QPlatformInputContext::filterEvent().
   if (e->keyType == QEvent::KeyPress && window) {
      if (QWindowSystemInterface::handleShortcutEvent(window, e->timestamp, e->key, e->modifiers,
            e->nativeScanCode, e->nativeVirtualKey, e->nativeModifiers, e->unicode, e->repeat, e->repeatCount)) {
         return;
      }
   }
#endif

   QKeyEvent ev(e->keyType, e->key, e->modifiers,
      e->nativeScanCode, e->nativeVirtualKey, e->nativeModifiers,
      e->unicode, e->repeat, e->repeatCount);
   ev.setTimestamp(e->timestamp);

   // only deliver key events when we have a window, and no modal window is blocking this window

   if (window && !window->d_func()->blockedByModalWindow) {
      QGuiApplication::sendSpontaneousEvent(window, &ev);
   }

#if defined(Q_OS_ANDROID)
   else {
      ev.setAccepted(false);
   }

   static bool backKeyPressAccepted = false;
   static bool menuKeyPressAccepted = false;
   if (e->keyType == QEvent::KeyPress) {
      backKeyPressAccepted = e->key == Qt::Key_Back && ev.isAccepted();
      menuKeyPressAccepted = e->key == Qt::Key_Menu && ev.isAccepted();
   } else if (e->keyType == QEvent::KeyRelease) {
      if (e->key == Qt::Key_Back && !backKeyPressAccepted && !ev.isAccepted()) {
         if (window) {
            QWindowSystemInterface::handleCloseEvent(window);
         }
      } else if (e->key == Qt::Key_Menu && !menuKeyPressAccepted && !ev.isAccepted()) {
         platform_theme->showPlatformMenuBar();
      }
   }
#endif
   e->eventAccepted = ev.isAccepted();
}

void QGuiApplicationPrivate::processEnterEvent(QWindowSystemInterfacePrivate::EnterEvent *e)
{
   if (!e->enter) {
      return;
   }
   if (e->enter.data()->d_func()->blockedByModalWindow) {
      // a modal window is blocking this window, don't allow enter events through
      return;
   }

   currentMouseWindow = e->enter;

   QEnterEvent event(e->localPos, e->localPos, e->globalPos);
   QCoreApplication::sendSpontaneousEvent(e->enter.data(), &event);
}

void QGuiApplicationPrivate::processLeaveEvent(QWindowSystemInterfacePrivate::LeaveEvent *e)
{
   if (!e->leave) {
      return;
   }
   if (e->leave.data()->d_func()->blockedByModalWindow) {
      // a modal window is blocking this window, don't allow leave events through
      return;
   }

   currentMouseWindow = nullptr;

   QEvent event(QEvent::Leave);
   QCoreApplication::sendSpontaneousEvent(e->leave.data(), &event);
}

void QGuiApplicationPrivate::processActivatedEvent(QWindowSystemInterfacePrivate::ActivatedWindowEvent *e)
{
   QWindow *previous = QGuiApplicationPrivate::focus_window;
   QWindow *newFocus = e->activated.data();

   if (previous == newFocus) {
      return;
   }

   if (newFocus)
      if (QPlatformWindow *platformWindow = newFocus->handle())
         if (platformWindow->isAlertState()) {
            platformWindow->setAlertState(false);
         }

   QObject *previousFocusObject = previous ? previous->focusObject() : nullptr;

   if (previous) {
      QFocusEvent focusAboutToChange(QEvent::FocusAboutToChange);
      QCoreApplication::sendSpontaneousEvent(previous, &focusAboutToChange);
   }

   QGuiApplicationPrivate::focus_window = newFocus;
   if (! qApp) {
      return;
   }

   if (previous) {
      Qt::FocusReason r = e->reason;
      if ((r == Qt::OtherFocusReason || r == Qt::ActiveWindowFocusReason) &&
         newFocus && (newFocus->flags() & Qt::Popup) == Qt::Popup) {
         r = Qt::PopupFocusReason;
      }

      QFocusEvent focusOut(QEvent::FocusOut, r);
      QCoreApplication::sendSpontaneousEvent(previous, &focusOut);

      QObject::disconnect(previous, &QWindow::focusObjectChanged, qApp, &QApplication::_q_updateFocusObject);

   } else if (!platformIntegration()->hasCapability(QPlatformIntegration::ApplicationState)) {
      setApplicationState(Qt::ApplicationActive);
   }

   if (QGuiApplicationPrivate::focus_window) {
      Qt::FocusReason r = e->reason;

      if ((r == Qt::OtherFocusReason || r == Qt::ActiveWindowFocusReason) &&
         previous && (previous->flags() & Qt::Popup) == Qt::Popup) {
         r = Qt::PopupFocusReason;
      }

      QFocusEvent focusIn(QEvent::FocusIn, r);
      QCoreApplication::sendSpontaneousEvent(QGuiApplicationPrivate::focus_window, &focusIn);

      QObject::connect(QGuiApplicationPrivate::focus_window, &QWindow::focusObjectChanged,
            qApp, &QApplication::_q_updateFocusObject);

   } else if (! platformIntegration()->hasCapability(QPlatformIntegration::ApplicationState)) {
      setApplicationState(Qt::ApplicationInactive);
   }

   if (self) {
      self->notifyActiveWindowChange(previous);

      if (previousFocusObject != qApp->focusObject()) {
         self->_q_updateFocusObject(qApp->focusObject());
      }
   }

   emit qApp->focusWindowChanged(newFocus);
   if (previous) {
      emit previous->activeChanged();
   }

   if (newFocus) {
      emit newFocus->activeChanged();
   }
}

void QGuiApplicationPrivate::processWindowStateChangedEvent(QWindowSystemInterfacePrivate::WindowStateChangedEvent *wse)
{
   if (QWindow *window  = wse->window.data()) {
      QWindowStateChangeEvent e(window->windowState());
      window->d_func()->windowState = wse->newState;
      QGuiApplication::sendSpontaneousEvent(window, &e);
   }
}

void QGuiApplicationPrivate::processWindowScreenChangedEvent(QWindowSystemInterfacePrivate::WindowScreenChangedEvent *wse)
{
   if (QWindow *window  = wse->window.data()) {
      if (window->isTopLevel()) {
         if (QScreen *screen = wse->screen.data()) {
            window->d_func()->setTopLevelScreen(screen, false);

         } else {
            // Fall back to default behavior, and try to find some appropriate screen
            window->setScreen(nullptr);
         }
      }

      // we may have changed scaling, so trigger resize event if needed
      if (window->handle()) {
         QWindowSystemInterfacePrivate::GeometryChangeEvent gce(window,
            QHighDpi::fromNativePixels(window->handle()->geometry(), window), QRect());

         processGeometryChangeEvent(&gce);
      }
   }
}

void QGuiApplicationPrivate::processThemeChanged(QWindowSystemInterfacePrivate::ThemeChangeEvent *tce)
{
   if (self) {
      self->notifyThemeChanged();
   }

   if (QWindow *window  = tce->window.data()) {
      QEvent e(QEvent::ThemeChange);
      QGuiApplication::sendSpontaneousEvent(window, &e);
   }
}

void QGuiApplicationPrivate::processGeometryChangeEvent(QWindowSystemInterfacePrivate::GeometryChangeEvent *e)
{
   if (e->tlw.isNull()) {
      return;
   }

   QWindow *window = e->tlw.data();
   if (!window) {
      return;
   }

   QRect newRect = e->newGeometry;
   QRect oldRect = e->oldGeometry.isNull() ? window->d_func()->geometry : e->oldGeometry;

   bool isResize = oldRect.size() != newRect.size();
   bool isMove = oldRect.topLeft() != newRect.topLeft();

   window->d_func()->geometry = newRect;

   if (isResize || window->d_func()->resizeEventPending) {
      QResizeEvent e(newRect.size(), oldRect.size());
      QGuiApplication::sendSpontaneousEvent(window, &e);

      window->d_func()->resizeEventPending = false;

      if (oldRect.width() != newRect.width()) {
         window->widthChanged(newRect.width());
      }
      if (oldRect.height() != newRect.height()) {
         window->heightChanged(newRect.height());
      }
   }

   if (isMove) {
      //### frame geometry
      QMoveEvent e(newRect.topLeft(), oldRect.topLeft());
      QGuiApplication::sendSpontaneousEvent(window, &e);

      if (oldRect.x() != newRect.x()) {
         window->xChanged(newRect.x());
      }
      if (oldRect.y() != newRect.y()) {
         window->yChanged(newRect.y());
      }
   }
}

void QGuiApplicationPrivate::processCloseEvent(QWindowSystemInterfacePrivate::CloseEvent *e)
{
   if (e->window.isNull()) {
      return;
   }
   if (e->window.data()->d_func()->blockedByModalWindow) {
      // a modal window is blocking this window, don't allow close events through
      return;
   }

   QCloseEvent event;
   QGuiApplication::sendSpontaneousEvent(e->window.data(), &event);
   if (e->accepted) {
      *(e->accepted) = event.isAccepted();
   }
}

void QGuiApplicationPrivate::processFileOpenEvent(QWindowSystemInterfacePrivate::FileOpenEvent *e)
{
   if (e->m_url.isEmpty()) {
      return;
   }

   QFileOpenEvent event(e->m_url);
   QGuiApplication::sendSpontaneousEvent(qApp, &event);
}

void QGuiApplicationPrivate::processTabletEvent(QWindowSystemInterfacePrivate::TabletEvent *e)
{
#ifndef QT_NO_TABLETEVENT
   QEvent::Type type = QEvent::TabletMove;

   if (e->buttons != tabletState) {
      type = (e->buttons > tabletState) ? QEvent::TabletPress : QEvent::TabletRelease;
   }

   QWindow *window = e->window.data();
   modifier_buttons = e->modifiers;

   bool localValid = true;
   // If window is null, pick one based on the global position and make sure all
   // subsequent events up to the release are delivered to that same window.
   // If window is given, just send to that.
   if (type == QEvent::TabletPress) {
      if (e->nullWindow()) {
         window = QGuiApplication::topLevelWindowAt(e->global.toPoint());
         localValid = false;
      }

      if (!window) {
         return;
      }

      tabletPressTarget = window;

   } else {
      if (e->nullWindow()) {
         window = tabletPressTarget;
         localValid = false;
      }
      if (type == QEvent::TabletRelease) {
         tabletPressTarget = nullptr;
      }
      if (!window) {
         return;
      }
   }
   QPointF local = e->local;
   if (!localValid) {
      QPointF delta = e->global - e->global.toPoint();
      local = window->mapFromGlobal(e->global.toPoint()) + delta;
   }

   Qt::MouseButtons stateChange = e->buttons ^ tabletState;
   Qt::MouseButton button = Qt::NoButton;
   for (int check = Qt::LeftButton; check <= int(Qt::MaxMouseButton); check = check << 1) {
      if (check & stateChange) {
         button = Qt::MouseButton(check);
         break;
      }
   }

   QTabletEvent ev(type, local, e->global,
      e->device, e->pointerType, e->pressure, e->xTilt, e->yTilt,
      e->tangentialPressure, e->rotation, e->z,
      e->modifiers, e->uid, button, e->buttons);

   ev.setTimestamp(e->timestamp);
   QGuiApplication::sendSpontaneousEvent(window, &ev);
   tabletState = e->buttons;

#else
   (void) e;
#endif
}

void QGuiApplicationPrivate::processTabletEnterProximityEvent(QWindowSystemInterfacePrivate::TabletEnterProximityEvent *e)
{
#ifndef QT_NO_TABLETEVENT
   QTabletEvent ev(QEvent::TabletEnterProximity, QPointF(), QPointF(),
            e->device, e->pointerType, 0, 0, 0, 0, 0, 0, Qt::NoModifier, e->uid, Qt::NoButton, tabletState);

   ev.setTimestamp(e->timestamp);
   QGuiApplication::sendSpontaneousEvent(qGuiApp, &ev);

#else
   (void) e;
#endif
}

void QGuiApplicationPrivate::processTabletLeaveProximityEvent(QWindowSystemInterfacePrivate::TabletLeaveProximityEvent *e)
{
#ifndef QT_NO_TABLETEVENT
   QTabletEvent ev(QEvent::TabletLeaveProximity, QPointF(), QPointF(),
            e->device, e->pointerType, 0, 0, 0, 0, 0, 0, Qt::NoModifier, e->uid, Qt::NoButton, tabletState);

   ev.setTimestamp(e->timestamp);
   QGuiApplication::sendSpontaneousEvent(qGuiApp, &ev);

#else
   (void) e;
#endif
}

#ifndef QT_NO_GESTURES
void QGuiApplicationPrivate::processGestureEvent(QWindowSystemInterfacePrivate::GestureEvent *e)
{
   if (e->window.isNull()) {
      return;
   }

   QNativeGestureEvent ev(e->type, e->pos, e->pos, e->globalPos, e->realValue, e->sequenceId, e->intValue);
   ev.setTimestamp(e->timestamp);
   QGuiApplication::sendSpontaneousEvent(e->window, &ev);
}
#endif // QT_NO_GESTURES

void QGuiApplicationPrivate::processPlatformPanelEvent(QWindowSystemInterfacePrivate::PlatformPanelEvent *e)
{
   if (! e->window) {
      return;
   }

   if (e->window->d_func()->blockedByModalWindow) {
      // a modal window is blocking this window, don't allow events through
      return;
   }

   QEvent ev(QEvent::PlatformPanel);
   QGuiApplication::sendSpontaneousEvent(e->window.data(), &ev);
}

#ifndef QT_NO_CONTEXTMENU
void QGuiApplicationPrivate::processContextMenuEvent(QWindowSystemInterfacePrivate::ContextMenuEvent *e)
{
   // Widgets do not care about mouse triggered context menu events. Also, do not forward event
   // to a window blocked by a modal window.
   if (!e->window || e->mouseTriggered || e->window->d_func()->blockedByModalWindow) {
      return;
   }

   QContextMenuEvent ev(QContextMenuEvent::Keyboard, e->pos, e->globalPos, e->modifiers);
   QGuiApplication::sendSpontaneousEvent(e->window.data(), &ev);
}
#endif

Q_GUI_EXPORT uint qHash(const QGuiApplicationPrivate::ActiveTouchPointsKey &k)
{
   return qHash(k.device) + k.touchPointId;
}

Q_GUI_EXPORT bool operator==(const QGuiApplicationPrivate::ActiveTouchPointsKey &a,
   const QGuiApplicationPrivate::ActiveTouchPointsKey &b)
{
   return a.device == b.device && a.touchPointId == b.touchPointId;
}

void QGuiApplicationPrivate::processTouchEvent(QWindowSystemInterfacePrivate::TouchEvent *e)
{
   QGuiApplicationPrivate *d = self;
   modifier_buttons = e->modifiers;

   if (e->touchType == QEvent::TouchCancel) {
      // The touch sequence has been canceled (e.g. by the compositor).
      // Send the TouchCancel to all windows with active touches and clean up.
      QTouchEvent touchEvent(QEvent::TouchCancel, e->device, e->modifiers);
      touchEvent.setTimestamp(e->timestamp);

      QHash<ActiveTouchPointsKey, ActiveTouchPointsValue>::const_iterator it
         = self->activeTouchPoints.constBegin(), ite = self->activeTouchPoints.constEnd();
      QSet<QWindow *> windowsNeedingCancel;

      while (it != ite) {
         QWindow *w = it->window.data();

         if (w) {
            windowsNeedingCancel.insert(w);
         }
         ++it;
      }

      for (QSet<QWindow *>::const_iterator winIt = windowsNeedingCancel.constBegin(),
         winItEnd = windowsNeedingCancel.constEnd(); winIt != winItEnd; ++winIt) {
         touchEvent.setWindow(*winIt);
         QGuiApplication::sendSpontaneousEvent(*winIt, &touchEvent);
      }

      if (! self->synthesizedMousePoints.isEmpty() && ! e->synthetic()) {
         for (QHash<QWindow *, SynthesizedMouseData>::const_iterator synthIt = self->synthesizedMousePoints.constBegin(),
            synthItEnd = self->synthesizedMousePoints.constEnd(); synthIt != synthItEnd; ++synthIt) {

            if (! synthIt->window) {
               continue;
            }

            QWindowSystemInterfacePrivate::MouseEvent fake(synthIt->window.data(),
               e->timestamp, synthIt->pos, synthIt->screenPos, buttons & ~Qt::LeftButton,
               e->modifiers, Qt::MouseEventSynthesizedByCS);

            fake.flags |= QWindowSystemInterfacePrivate::WindowSystemEvent::Synthetic;
            processMouseEvent(&fake);
         }
         self->synthesizedMousePoints.clear();
      }

      self->activeTouchPoints.clear();
      self->lastTouchType = e->touchType;
      return;
   }

   // Prevent sending ill-formed event sequences: Cancel can only be followed by a Begin.
   if (self->lastTouchType == QEvent::TouchCancel && e->touchType != QEvent::TouchBegin) {
      return;
   }

   self->lastTouchType = e->touchType;

   QWindow *window = e->window.data();
   typedef QPair<Qt::TouchPointStates, QList<QTouchEvent::TouchPoint>> StatesAndTouchPoints;
   QHash<QWindow *, StatesAndTouchPoints> windowsNeedingEvents;

   for (int i = 0; i < e->points.count(); ++i) {
      QTouchEvent::TouchPoint touchPoint = e->points.at(i);
      // explicitly detach from the original touch point that we got, so even
      // if the touchpoint structs are reused, we will make a copy that we'll
      // deliver to the user (which might want to store the struct for later use).
      touchPoint.d = touchPoint.d->detach();

      // update state
      QPointer<QWindow> w;
      QTouchEvent::TouchPoint previousTouchPoint;
      ActiveTouchPointsKey touchInfoKey(e->device, touchPoint.id());
      ActiveTouchPointsValue &touchInfo = d->activeTouchPoints[touchInfoKey];

      switch (touchPoint.state()) {
         case Qt::TouchPointPressed:
            if (e->device->type() == QTouchDevice::TouchPad) {
               // on touch-pads, send all touch points to the same widget
               w = d->activeTouchPoints.isEmpty()
                  ? QPointer<QWindow>() : d->activeTouchPoints.constBegin().value().window;
            }

            if (! w) {
               // determine which window this event will go to
               if (!window) {
                  window = QGuiApplication::topLevelWindowAt(touchPoint.screenPos().toPoint());
               }
               if (!window) {
                  continue;
               }
               w = window;
            }

            touchInfo.window = w;
            touchPoint.d->startScreenPos = touchPoint.screenPos();
            touchPoint.d->lastScreenPos = touchPoint.screenPos();
            touchPoint.d->startNormalizedPos = touchPoint.normalizedPos();
            touchPoint.d->lastNormalizedPos = touchPoint.normalizedPos();

            if (touchPoint.pressure() < qreal(0.)) {
               touchPoint.d->pressure = qreal(1.);
            }

            touchInfo.touchPoint = touchPoint;
            break;

         case Qt::TouchPointReleased:
            w = touchInfo.window;
            if (!w) {
               continue;
            }

            previousTouchPoint = touchInfo.touchPoint;
            touchPoint.d->startScreenPos = previousTouchPoint.startScreenPos();
            touchPoint.d->lastScreenPos = previousTouchPoint.screenPos();
            touchPoint.d->startPos = previousTouchPoint.startPos();
            touchPoint.d->lastPos = previousTouchPoint.pos();
            touchPoint.d->startNormalizedPos = previousTouchPoint.startNormalizedPos();
            touchPoint.d->lastNormalizedPos = previousTouchPoint.normalizedPos();
            if (touchPoint.pressure() < qreal(0.)) {
               touchPoint.d->pressure = qreal(0.);
            }

            break;

         default:
            w = touchInfo.window;
            if (!w) {
               continue;
            }

            previousTouchPoint = touchInfo.touchPoint;
            touchPoint.d->startScreenPos = previousTouchPoint.startScreenPos();
            touchPoint.d->lastScreenPos = previousTouchPoint.screenPos();
            touchPoint.d->startPos = previousTouchPoint.startPos();
            touchPoint.d->lastPos = previousTouchPoint.pos();
            touchPoint.d->startNormalizedPos = previousTouchPoint.startNormalizedPos();
            touchPoint.d->lastNormalizedPos = previousTouchPoint.normalizedPos();
            if (touchPoint.pressure() < qreal(0.)) {
               touchPoint.d->pressure = qreal(1.);
            }

            // Stationary points might not be delivered down to the receiving item
            // and get their position transformed, keep the old values instead.
            if (touchPoint.state() != Qt::TouchPointStationary) {
               touchInfo.touchPoint = touchPoint;
            }
            break;
      }

      Q_ASSERT(w.data() != nullptr);

      // make the *scene* functions return the same as the *screen* functions
      touchPoint.d->sceneRect = touchPoint.screenRect();
      touchPoint.d->startScenePos = touchPoint.startScreenPos();
      touchPoint.d->lastScenePos  = touchPoint.lastScreenPos();

      StatesAndTouchPoints &maskAndPoints = windowsNeedingEvents[w.data()];
      maskAndPoints.first |= touchPoint.state();
      maskAndPoints.second.append(touchPoint);
   }

   if (windowsNeedingEvents.isEmpty()) {
      return;
   }

   QHash<QWindow *, StatesAndTouchPoints>::const_iterator it        = windowsNeedingEvents.constBegin();
   const QHash<QWindow *, StatesAndTouchPoints>::const_iterator end = windowsNeedingEvents.constEnd();

   for (; it != end; ++it) {
      QWindow *w = it.key();

      QEvent::Type eventType;
      switch (it.value().first) {
         case Qt::TouchPointPressed:
            eventType = QEvent::TouchBegin;
            break;

         case Qt::TouchPointReleased:
            eventType = QEvent::TouchEnd;
            break;

         case Qt::TouchPointStationary:
            // don't send the event if nothing changed
            continue;

         default:
            eventType = QEvent::TouchUpdate;
            break;
      }

      if (w->d_func()->blockedByModalWindow) {
         // a modal window is blocking this window, don't allow touch events through

         // QTBUG-37371 temporary fix; TODO: revisit in 5.4 when we have a forwarding solution
         if (eventType == QEvent::TouchEnd) {
            // but don't leave dangling state: e.g.
            // QQuickWindowPrivate::itemForTouchPointId needs to be cleared.
            QTouchEvent touchEvent(QEvent::TouchCancel, e->device, e->modifiers);
            touchEvent.setTimestamp(e->timestamp);
            touchEvent.setWindow(w);
            QGuiApplication::sendSpontaneousEvent(w, &touchEvent);
         }
         continue;
      }

      QTouchEvent touchEvent(eventType, e->device, e->modifiers, it.value().first, it.value().second);
      touchEvent.setTimestamp(e->timestamp);
      touchEvent.setWindow(w);

      const int pointCount = touchEvent.touchPoints().count();
      for (int i = 0; i < pointCount; ++i) {
         QTouchEvent::TouchPoint &touchPoint = touchEvent._touchPoints[i];

         // preserve the sub-pixel resolution
         QRectF rect = touchPoint.screenRect();
         const QPointF screenPos = rect.center();
         const QPointF delta = screenPos - screenPos.toPoint();

         rect.moveCenter(w->mapFromGlobal(screenPos.toPoint()) + delta);
         touchPoint.d->rect = rect;
         if (touchPoint.state() == Qt::TouchPointPressed) {
            touchPoint.d->startPos = w->mapFromGlobal(touchPoint.startScreenPos().toPoint()) + delta;
            touchPoint.d->lastPos = w->mapFromGlobal(touchPoint.lastScreenPos().toPoint()) + delta;
         }
      }

      QGuiApplication::sendSpontaneousEvent(w, &touchEvent);
      if (!e->synthetic() && !touchEvent.isAccepted() && qApp->testAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents)) {
         // exclude devices which generate their own mouse events

         if (!(touchEvent.device()->capabilities() & QTouchDevice::MouseEmulation)) {
            Qt::MouseButtons b = eventType == QEvent::TouchEnd ? Qt::NoButton : Qt::LeftButton;
            if (b == Qt::NoButton) {
               self->synthesizedMousePoints.clear();
            }

            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent.touchPoints();
            if (eventType == QEvent::TouchBegin) {
               m_fakeMouseSourcePointId = touchPoints.first().id();
            }

            for (int i = 0; i < touchPoints.count(); ++i) {
               const QTouchEvent::TouchPoint &touchPoint = touchPoints.at(i);

               if (touchPoint.id() == m_fakeMouseSourcePointId) {
                  if (b != Qt::NoButton) {
                     self->synthesizedMousePoints.insert(w, SynthesizedMouseData(
                           touchPoint.pos(), touchPoint.screenPos(), w));
                  }

                  QWindowSystemInterfacePrivate::MouseEvent fake(w, e->timestamp,
                     touchPoint.pos(),
                     touchPoint.screenPos(),
                     b | (buttons & ~Qt::LeftButton),
                     e->modifiers,
                     Qt::MouseEventSynthesizedByCS);

                  fake.flags |= QWindowSystemInterfacePrivate::WindowSystemEvent::Synthetic;
                  processMouseEvent(&fake);
                  break;
               }
            }
         }
      }
   }

   // Remove released points from the hash table only after the event is
   // delivered. When the receiver is a widget, QApplication will access
   // activeTouchPoints during delivery and therefore nothing can be removed
   // before sending the event.
   for (int i = 0; i < e->points.count(); ++i) {
      QTouchEvent::TouchPoint touchPoint = e->points.at(i);
      if (touchPoint.state() == Qt::TouchPointReleased) {
         d->activeTouchPoints.remove(ActiveTouchPointsKey(e->device, touchPoint.id()));
      }
   }
}

void QGuiApplicationPrivate::reportScreenOrientationChange(QWindowSystemInterfacePrivate::ScreenOrientationEvent *e)
{
   // This operation only makes sense after the QGuiApplication constructor runs
   if (QCoreApplication::startingUp()) {
      return;
   }

   if (!e->screen) {
      return;
   }

   QScreen *s = e->screen.data();
   s->d_func()->orientation = e->orientation;

   updateFilteredScreenOrientation(s);
}

void QGuiApplicationPrivate::updateFilteredScreenOrientation(QScreen *s)
{
   Qt::ScreenOrientation o = s->d_func()->orientation;
   if (o == Qt::PrimaryOrientation) {
      o = s->primaryOrientation();
   }
   o = Qt::ScreenOrientation(o & s->orientationUpdateMask());
   if (o == Qt::PrimaryOrientation) {
      return;
   }
   if (o == s->d_func()->filteredOrientation) {
      return;
   }
   s->d_func()->filteredOrientation = o;
   reportScreenOrientationChange(s);
}

void QGuiApplicationPrivate::reportScreenOrientationChange(QScreen *s)
{
   emit s->orientationChanged(s->orientation());

   QScreenOrientationChangeEvent event(s, s->orientation());
   QCoreApplication::sendEvent(QCoreApplication::instance(), &event);
}

void QGuiApplicationPrivate::reportGeometryChange(QWindowSystemInterfacePrivate::ScreenGeometryEvent *e)
{
   // This operation only makes sense after the QGuiApplication constructor runs
   if (QCoreApplication::startingUp()) {
      return;
   }

   if (!e->screen) {
      return;
   }

   QScreen *s = e->screen.data();

   bool geometryChanged = e->geometry != s->d_func()->geometry;
   s->d_func()->geometry = e->geometry;

   bool availableGeometryChanged = e->availableGeometry != s->d_func()->availableGeometry;
   s->d_func()->availableGeometry = e->availableGeometry;

   if (geometryChanged) {
      Qt::ScreenOrientation primaryOrientation = s->primaryOrientation();
      s->d_func()->updatePrimaryOrientation();

      emit s->geometryChanged(s->geometry());
      emit s->physicalSizeChanged(s->physicalSize());
      emit s->physicalDotsPerInchChanged(s->physicalDotsPerInch());
      emit s->logicalDotsPerInchChanged(s->logicalDotsPerInch());

      if (s->primaryOrientation() != primaryOrientation) {
         emit s->primaryOrientationChanged(s->primaryOrientation());
      }

      if (s->d_func()->orientation == Qt::PrimaryOrientation) {
         updateFilteredScreenOrientation(s);
      }
   }

   if (availableGeometryChanged) {
      emit s->availableGeometryChanged(s->availableGeometry());
   }

   if (geometryChanged || availableGeometryChanged) {
      for (QScreen *sibling : s->virtualSiblings()) {
         emit sibling->virtualGeometryChanged(sibling->virtualGeometry());
      }
   }
}

void QGuiApplicationPrivate::reportLogicalDotsPerInchChange(QWindowSystemInterfacePrivate::ScreenLogicalDotsPerInchEvent *e)
{
   // This operation only makes sense after the QGuiApplication constructor runs
   if (QCoreApplication::startingUp()) {
      return;
   }

   if (!e->screen) {
      return;
   }

   QScreen *s = e->screen.data();
   s->d_func()->logicalDpi = QDpi(e->dpiX, e->dpiY);

   emit s->logicalDotsPerInchChanged(s->logicalDotsPerInch());
}

void QGuiApplicationPrivate::reportRefreshRateChange(QWindowSystemInterfacePrivate::ScreenRefreshRateEvent *e)
{
   // This operation only makes sense after the QGuiApplication constructor runs
   if (QCoreApplication::startingUp()) {
      return;
   }

   if (!e->screen) {
      return;
   }

   QScreen *s = e->screen.data();
   qreal rate = e->rate;
   // safeguard ourselves against buggy platform behavior...
   if (rate < 1.0) {
      rate = 60.0;
   }
   if (!qFuzzyCompare(s->d_func()->refreshRate, rate)) {
      s->d_func()->refreshRate = rate;
      emit s->refreshRateChanged(s->refreshRate());
   }
}

void QGuiApplicationPrivate::processExposeEvent(QWindowSystemInterfacePrivate::ExposeEvent *e)
{
   if (!e->exposed) {
      return;
   }

   QWindow *window = e->exposed.data();
   if (!window) {
      return;
   }
   QWindowPrivate *p = qt_window_private(window);

   if (!p->receivedExpose) {
      if (p->resizeEventPending) {
         // as a convenience for plugins, send a resize event before the first expose event if they haven't done so
         // window->geometry() should have a valid size as soon as a handle exists.
         QResizeEvent e(window->geometry().size(), p->geometry.size());
         QGuiApplication::sendSpontaneousEvent(window, &e);

         p->resizeEventPending = false;
      }

      p->receivedExpose = true;
   }

   p->exposed = e->isExposed && window->screen();

   QExposeEvent exposeEvent(e->region);
   QCoreApplication::sendSpontaneousEvent(window, &exposeEvent);
}

#ifndef QT_NO_DRAGANDDROP

QPlatformDragQtResponse QGuiApplicationPrivate::processDrag(QWindow *w, const QMimeData *dropData, const QPoint &p,
   Qt::DropActions supportedActions)
{
   static QPointer<QWindow> currentDragWindow;
   static Qt::DropAction lastAcceptedDropAction = Qt::IgnoreAction;

   QPlatformDrag *platformDrag = platformIntegration()->drag();

   if (! platformDrag) {
      lastAcceptedDropAction = Qt::IgnoreAction;
      return QPlatformDragQtResponse(false, lastAcceptedDropAction, QRect());
   }

   if (! dropData) {
      if (currentDragWindow.data() == w) {
         currentDragWindow = nullptr;
      }

      QDragLeaveEvent e;
      QGuiApplication::sendEvent(w, &e);
      lastAcceptedDropAction = Qt::IgnoreAction;
      return QPlatformDragQtResponse(false, lastAcceptedDropAction, QRect());
   }

   QDragMoveEvent me(p, supportedActions, dropData, QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());

   if (w != currentDragWindow) {
      lastAcceptedDropAction = Qt::IgnoreAction;

      if (currentDragWindow) {
         QDragLeaveEvent e;
         QGuiApplication::sendEvent(currentDragWindow, &e);
      }

      currentDragWindow = w;
      QDragEnterEvent e(p, supportedActions, dropData, QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());
      QGuiApplication::sendEvent(w, &e);

      if (e.isAccepted() && e.dropAction() != Qt::IgnoreAction) {
         lastAcceptedDropAction = e.dropAction();
      }
   }

   // Handling 'DragEnter' should suffice for the application.
   if (lastAcceptedDropAction != Qt::IgnoreAction
      && (supportedActions & lastAcceptedDropAction)) {
      me.setDropAction(lastAcceptedDropAction);
      me.accept();
   }

   QGuiApplication::sendEvent(w, &me);
   lastAcceptedDropAction = me.isAccepted() ? me.dropAction() : Qt::IgnoreAction;
   return QPlatformDragQtResponse(me.isAccepted(), lastAcceptedDropAction, me.answerRect());
}

QPlatformDropQtResponse QGuiApplicationPrivate::processDrop(QWindow *w, const QMimeData *dropData, const QPoint &p,
   Qt::DropActions supportedActions)
{
   QDropEvent de(p, supportedActions, dropData,
      QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());
   QGuiApplication::sendEvent(w, &de);

   Qt::DropAction acceptedAction = de.isAccepted() ? de.dropAction() : Qt::IgnoreAction;
   QPlatformDropQtResponse response(de.isAccepted(), acceptedAction);
   return response;
}

#endif // QT_NO_DRAGANDDROP

#ifndef QT_NO_CLIPBOARD

QClipboard *QApplication::clipboard()
{
   if (QGuiApplicationPrivate::qt_clipboard == nullptr) {
      if (! qApp) {
         qWarning("QApplication::clipboard() QApplication must be started before accessing the clipboard");
         return nullptr;
      }
      QGuiApplicationPrivate::qt_clipboard = new QClipboard(nullptr);
   }

   return QGuiApplicationPrivate::qt_clipboard;
}
#endif

QPalette QApplication::palette()
{
   initPalette();
   return *QGuiApplicationPrivate::app_palette;
}

void QGuiApplicationPrivate::applyWindowGeometrySpecificationTo(QWindow *window)
{
   windowGeometrySpecification.applyTo(window);
}

QFont QApplication::font()
{
   Q_ASSERT_X(QGuiApplicationPrivate::self, "QGuiApplication::font()", "no QGuiApplication instance");
   QMutexLocker locker(&applicationFontMutex);
   initFontUnlocked();
   return *QGuiApplicationPrivate::app_font;
}

QIcon QGuiApplication::windowIcon()
{
   return QGuiApplicationPrivate::app_icon ? *QGuiApplicationPrivate::app_icon : QIcon();
}

void QApplication::setWindowIcon(const QIcon &icon)
{
   if (!QGuiApplicationPrivate::app_icon) {
      QGuiApplicationPrivate::app_icon = new QIcon();
   }
   *QGuiApplicationPrivate::app_icon = icon;
   if (QGuiApplicationPrivate::platform_integration
      && QGuiApplicationPrivate::platform_integration->hasCapability(QPlatformIntegration::ApplicationIcon)) {
      QGuiApplicationPrivate::platform_integration->setApplicationIcon(icon);
   }
   if (QGuiApplicationPrivate::is_app_running && !QGuiApplicationPrivate::is_app_closing) {
      QGuiApplicationPrivate::self->notifyWindowIconChanged();
   }
}

void QApplication::setQuitOnLastWindowClosed(bool quit)
{
   QCoreApplication::setQuitLockEnabled(quit);
}

bool QApplication::quitOnLastWindowClosed()
{
   return QCoreApplication::isQuitLockEnabled();
}

void QGuiApplicationPrivate::emitLastWindowClosed()
{
   if (qGuiApp && qGuiApp->d_func()->in_exec) {
      emit qGuiApp->lastWindowClosed();
   }
}

bool QGuiApplicationPrivate::shouldQuitInternal(const QWindowList &processedWindows)
{
   /* if there is no visible top-level window left, we allow the quit */
   QWindowList list = QGuiApplication::topLevelWindows();
   for (int i = 0; i < list.size(); ++i) {
      QWindow *w = list.at(i);
      if (processedWindows.contains(w)) {
         continue;
      }
      if (w->isVisible() && !w->transientParent()) {
         return false;
      }
   }
   return true;
}


bool QGuiApplicationPrivate::tryCloseRemainingWindows(QWindowList processedWindows)
{
   QWindowList list = QGuiApplication::topLevelWindows();
   for (int i = 0; i < list.size(); ++i) {
      QWindow *w = list.at(i);
      if (w->isVisible() && !processedWindows.contains(w)) {
         if (!w->close()) {
            return false;
         }
         processedWindows.append(w);
         list = QGuiApplication::topLevelWindows();
         i = -1;
      }
   }
   return true;
}

Qt::ApplicationState QApplication::applicationState()
{
   return QGuiApplicationPrivate::applicationState;
}

void QGuiApplicationPrivate::setApplicationState(Qt::ApplicationState state, bool forcePropagate)
{
   if ((applicationState == state) && !forcePropagate) {
      return;
   }

   applicationState = state;

   switch (state) {
      case Qt::ApplicationActive: {
         QEvent appActivate(QEvent::ApplicationActivate);
         QCoreApplication::sendSpontaneousEvent(qApp, &appActivate);
         break;
      }
      case Qt::ApplicationInactive: {
         QEvent appDeactivate(QEvent::ApplicationDeactivate);
         QCoreApplication::sendSpontaneousEvent(qApp, &appDeactivate);
         break;
      }
      default:
         break;
   }

   QApplicationStateChangeEvent event(applicationState);
   QCoreApplication::sendSpontaneousEvent(qApp, &event);

   emit qApp->applicationStateChanged(applicationState);
}

#ifndef QT_NO_SESSIONMANAGER

bool QApplication::isFallbackSessionManagementEnabled()
{
   return QGuiApplicationPrivate::is_fallback_session_management_enabled;
}

void QApplication::setFallbackSessionManagementEnabled(bool enabled)
{
   QGuiApplicationPrivate::is_fallback_session_management_enabled = enabled;
}

bool QApplication::isSessionRestored() const
{
   Q_D(const QGuiApplication);
   return d->is_session_restored;
}

QString QApplication::sessionId() const
{
   Q_D(const QGuiApplication);
   return d->session_manager->sessionId();
}

QString QApplication::sessionKey() const
{
   Q_D(const QGuiApplication);
   return d->session_manager->sessionKey();
}

bool QApplication::isSavingSession() const
{
   Q_D(const QGuiApplication);
   return d->is_saving_session;
}

void QGuiApplicationPrivate::commitData()
{
   Q_Q(QGuiApplication);
   is_saving_session = true;

   emit q->commitDataRequest(session_manager);
   if (is_fallback_session_management_enabled && session_manager->allowsInteraction()
      && !tryCloseAllWindows()) {
      session_manager->cancel();
   }

   is_saving_session = false;
}

void QGuiApplicationPrivate::saveState()
{
   Q_Q(QGuiApplication);

   is_saving_session = true;
   emit q->saveStateRequest(session_manager);
   is_saving_session = false;
}
#endif //QT_NO_SESSIONMANAGER


void QApplication::sync()
{
   QCoreApplication::processEvents();
   if (QGuiApplicationPrivate::platform_integration
      && QGuiApplicationPrivate::platform_integration->hasCapability(QPlatformIntegration::SyncState)) {
      QGuiApplicationPrivate::platform_integration->sync();
      QCoreApplication::processEvents();
      QWindowSystemInterface::flushWindowSystemEvents();
   }
}

void QApplication::setLayoutDirection(Qt::LayoutDirection direction)
{
   if (layout_direction == direction || direction == Qt::LayoutDirectionAuto) {
      return;
   }

   layout_direction = direction;

   if (qGuiApp) {
      emit qGuiApp->layoutDirectionChanged(direction);
      QGuiApplicationPrivate::self->notifyLayoutDirectionChange();
   }
}

Qt::LayoutDirection QApplication::layoutDirection()
{
   // layout_direction is only ever Qt::LayoutDirectionAuto if setLayoutDirection
   // was never called, or called with Qt::LayoutDirectionAuto (which is a no-op).
   // In that case we return the default LeftToRight.
   return layout_direction == Qt::LayoutDirectionAuto ? Qt::LeftToRight : layout_direction;
}


#ifndef QT_NO_CURSOR
QCursor *QApplication::overrideCursor()
{
   CHECK_QAPP_INSTANCE(nullptr)
   return qGuiApp->d_func()->cursor_list.isEmpty() ? nullptr : &qGuiApp->d_func()->cursor_list.first();
}

void QApplication::changeOverrideCursor(const QCursor &cursor)
{
   CHECK_QAPP_INSTANCE()

   if (qGuiApp->d_func()->cursor_list.isEmpty()) {
      return;
   }

   qGuiApp->d_func()->cursor_list.removeFirst();
   setOverrideCursor(cursor);
}
#endif


#ifndef QT_NO_CURSOR
static inline void applyCursor(QWindow *w, QCursor c)
{
   if (const QScreen *screen = w->screen()) {
      if (QPlatformCursor *cursor = screen->handle()->cursor()) {
         cursor->changeCursor(&c, w);
      }
   }
}

static inline void unsetCursor(QWindow *w)
{
   if (const QScreen *screen = w->screen()) {
      if (QPlatformCursor *cursor = screen->handle()->cursor()) {
         cursor->changeCursor(nullptr, w);
      }
   }
}

static inline void applyCursor(const QList<QWindow *> &l, const QCursor &c)
{
   for (int i = 0; i < l.size(); ++i) {
      QWindow *w = l.at(i);
      if (w->handle() && w->type() != Qt::Desktop) {
         applyCursor(w, c);
      }
   }
}

static inline void applyWindowCursor(const QList<QWindow *> &l)
{
   for (int i = 0; i < l.size(); ++i) {
      QWindow *w = l.at(i);

      if (w->handle() && w->type() != Qt::Desktop) {
         if (qt_window_private(w)->hasCursor) {
            applyCursor(w, w->cursor());
         } else {
            unsetCursor(w);
         }
      }
   }
}

void QApplication::setOverrideCursor(const QCursor &cursor)
{
   CHECK_QAPP_INSTANCE()
   qGuiApp->d_func()->cursor_list.prepend(cursor);
   applyCursor(QGuiApplicationPrivate::window_list, cursor);
}

void QApplication::restoreOverrideCursor()
{
   CHECK_QAPP_INSTANCE()
   if (qGuiApp->d_func()->cursor_list.isEmpty()) {
      return;
   }

   qGuiApp->d_func()->cursor_list.removeFirst();

   if (qGuiApp->d_func()->cursor_list.size() > 0) {
      QCursor c(qGuiApp->d_func()->cursor_list.value(0));
      applyCursor(QGuiApplicationPrivate::window_list, c);
   } else {
      applyWindowCursor(QGuiApplicationPrivate::window_list);
   }
}
#endif// QT_NO_CURSOR

QStyleHints *QApplication::styleHints()
{
   if (!QGuiApplicationPrivate::styleHints) {
      QGuiApplicationPrivate::styleHints = new QStyleHints();
   }
   return QGuiApplicationPrivate::styleHints;
}

void QApplication::setDesktopSettingsAware(bool on)
{
   QGuiApplicationPrivate::obey_desktop_settings = on;
}

bool QApplication::desktopSettingsAware()
{
   return QGuiApplicationPrivate::obey_desktop_settings;
}

QInputMethod *QApplication::inputMethod()
{
   CHECK_QAPP_INSTANCE(nullptr)

   if (!qGuiApp->d_func()->inputMethod) {
      qGuiApp->d_func()->inputMethod = new QInputMethod();
   }

   return qGuiApp->d_func()->inputMethod;
}

QStyle *QApplication::style()
{
   if (QApplicationPrivate::app_style) {
      return QApplicationPrivate::app_style;
   }

   if (! qobject_cast<QApplication *>(QCoreApplication::instance())) {
      Q_ASSERT(! "No style available without creating a QApplication");
      return nullptr;
   }

   if (! QApplicationPrivate::app_style) {
      // Compile-time search for default style
      QString style;

      if (! QApplicationPrivate::styleOverride.isEmpty()) {
         style = QApplicationPrivate::styleOverride;
      } else {
         style = QApplicationPrivate::desktopStyleKey();
      }

      QStyle *&app_style = QApplicationPrivate::app_style;
      app_style = QStyleFactory::create(style);

      if (! app_style) {
         QStringList styles = QStyleFactory::keys();

         for (int i = 0; i < styles.size(); ++i) {
            if ((app_style = QStyleFactory::create(styles.at(i)))) {
               break;
            }
         }
      }

      if (! app_style) {
         Q_ASSERT(! "No styles are available");
         return nullptr;
      }

      QApplicationPrivate::overrides_native_style = ( app_style->objectName() != QApplicationPrivate::desktopStyleKey());
   }

   // take ownership of the style
   QApplicationPrivate::app_style->setParent(qApp);

   initSystemPalette();

   if (QApplicationPrivate::set_palette) {
      // repolish set palette with the new style
      QApplication::setPalette(*QApplicationPrivate::set_palette);
   }

#ifndef QT_NO_STYLE_STYLESHEET
   if (! QApplicationPrivate::styleSheet.isEmpty()) {
      qApp->setStyleSheet(QApplicationPrivate::styleSheet);

   } else {
      QApplicationPrivate::app_style->polish(qApp);
   }

#else
   QApplicationPrivate::app_style->polish(qApp);

#endif

#if defined(CS_SHOW_DEBUG_GUI)
   qDebug("QApplication::style() Style class = %s",
         csPrintable(QApplicationPrivate::app_style->metaObject()->className()));
#endif

   return QApplicationPrivate::app_style;
}

void QApplication::setStyle(QStyle *style)
{
   if (! style || style == QApplicationPrivate::app_style) {
      return;
   }

   QWidgetList all = allWidgets();

   // clean up the old style
   if (QApplicationPrivate::app_style) {
      if (QApplicationPrivate::is_app_running && !QApplicationPrivate::is_app_closing) {

         for (QWidgetList::const_iterator it = all.constBegin(), cend = all.constEnd(); it != cend; ++it) {
            QWidget *w = *it;

            // except desktop
            if (! (w->windowType() == Qt::Desktop) &&  w->testAttribute(Qt::WA_WState_Polished)) {
               // has been polished
               QApplicationPrivate::app_style->unpolish(w);
            }
         }
      }
      QApplicationPrivate::app_style->unpolish(qApp);
   }

   QStyle *old = QApplicationPrivate::app_style;
   QApplicationPrivate::overrides_native_style = (nativeStyleClassName() == style->metaObject()->className());

#ifndef QT_NO_STYLE_STYLESHEET
   if (! QApplicationPrivate::styleSheet.isEmpty() && !qobject_cast<QStyleSheetStyle *>(style)) {
      // have a stylesheet already and a new style is being set

      QStyleSheetStyle *newProxy = new QStyleSheetStyle(style);
      style->setParent(newProxy);
      QApplicationPrivate::app_style = newProxy;
   } else
#endif

   {
      QApplicationPrivate::app_style = style;
   }

   QApplicationPrivate::app_style->setParent(qApp); // take ownership

   // take care of possible palette requirements of certain gui styles
   // must be done before polishing the application since the style
   // might call QApplication::setPalette() itself

   if (QApplicationPrivate::set_palette) {
      QApplication::setPalette(*QApplicationPrivate::set_palette);

   } else if (QApplicationPrivate::sys_palette) {
      clearSystemPalette();
      initSystemPalette();
      QApplicationPrivate::initializeWidgetPaletteHash();
      QApplicationPrivate::initializeWidgetFontHash();
      QApplicationPrivate::setPalette_helper(*QApplicationPrivate::sys_palette, QString(), false);

   } else if (!QApplicationPrivate::sys_palette) {
      // Initialize the sys_palette if it has not happened yet
      QApplicationPrivate::setSystemPalette(QApplicationPrivate::app_style->standardPalette());
   }

   // initialize the application with the new style
   QApplicationPrivate::app_style->polish(qApp);

   // re-polish existing widgets if necessary
   if (QApplicationPrivate::is_app_running && ! QApplicationPrivate::is_app_closing) {
      for (QWidgetList::const_iterator it = all.constBegin(), cend = all.constEnd(); it != cend; ++it) {

         QWidget *w = *it;
         if (w->windowType() != Qt::Desktop && w->testAttribute(Qt::WA_WState_Polished)) {

            if (w->style() == QApplicationPrivate::app_style) {
               QApplicationPrivate::app_style->polish(w);   // repolish
            }

#ifndef QT_NO_STYLE_STYLESHEET
            else {
               w->setStyleSheet(w->styleSheet());           // touch
            }
#endif
         }
      }

      for (QWidgetList::const_iterator it = all.constBegin(), cend = all.constEnd(); it != cend; ++it) {
         QWidget *w = *it;

         if (w->windowType() != Qt::Desktop && ! w->testAttribute(Qt::WA_SetStyle)) {
            QEvent e(QEvent::StyleChange);
            QApplication::sendEvent(w, &e);
            w->update();
         }
      }
   }

#ifndef QT_NO_STYLE_STYLESHEET
   if (QStyleSheetStyle *oldProxy = qobject_cast<QStyleSheetStyle *>(old)) {
      oldProxy->deref();
   } else
#endif

      if (old && old->parent() == qApp) {
         delete old;
      }

   if (QApplicationPrivate::focus_widget) {
      QFocusEvent in(QEvent::FocusIn, Qt::OtherFocusReason);
      QApplication::sendEvent(QApplicationPrivate::focus_widget->style(), &in);
      QApplicationPrivate::focus_widget->update();
   }
}

QPixmap QGuiApplicationPrivate::getPixmapCursor(Qt::CursorShape cshape)
{
   (void) cshape;

   return QPixmap();
}

void QGuiApplicationPrivate::notifyThemeChanged()
{
   if (! (applicationResourceFlags & ApplicationPaletteExplicitlySet) &&
      !QCoreApplication::testAttribute(Qt::AA_SetPalette)) {
      clearPalette();
      initPalette();
   }

   if (! (applicationResourceFlags & ApplicationFontExplicitlySet)) {
      QMutexLocker locker(&applicationFontMutex);
      clearFontUnlocked();
      initFontUnlocked();
   }

   clearSystemPalette();
   initSystemPalette();
   qt_init_tooltip_palette();
}

void QApplicationPrivate::setPalette_helper(const QPalette &palette, const QString &className, bool clearWidgetPaletteHash)
{
   QPalette pal = palette;

   if (QApplicationPrivate::app_style) {
      QApplicationPrivate::app_style->polish(pal);   // NB: non-const reference
   }

   bool all = false;
   PaletteHash *hash = cs_app_palettes_hash();

   if (className.isEmpty()) {
      if (QApplicationPrivate::app_palette && pal.isCopyOf(*QApplicationPrivate::app_palette)) {
         return;
      }

      if (! QApplicationPrivate::app_palette) {
         QApplicationPrivate::app_palette = new QPalette(pal);
      } else {
         *QApplicationPrivate::app_palette = pal;
      }

      if (hash && hash->size()) {
         all = true;
         if (clearWidgetPaletteHash) {
            hash->clear();
         }
      }

   } else if (hash) {
      hash->insert(className, pal);
   }

   if (QApplicationPrivate::is_app_running && ! QApplicationPrivate::is_app_closing) {
      // Send ApplicationPaletteChange to qApp itself, and to the widgets.
      QEvent e(QEvent::ApplicationPaletteChange);
      QApplication::sendEvent(QApplication::instance(), &e);

      QWidgetList wids = QApplication::allWidgets();

      for (auto it = wids.constBegin(), cend = wids.constEnd(); it != cend; ++it) {
         QWidget *w = *it;

         if (all || (className.isEmpty() && w->isWindow()) || w->inherits(className)) {
            // matching class
            QApplication::sendEvent(w, &e);
         }
      }

      // Send to all scenes as well
#ifndef QT_NO_GRAPHICSVIEW
      QList<QGraphicsScene *> &scenes = qApp->d_func()->scene_list;

      for (QList<QGraphicsScene *>::const_iterator it = scenes.constBegin(); it != scenes.constEnd(); ++it) {
         QApplication::sendEvent(*it, &e);
      }
#endif

   }

   if (className.isEmpty() && (! QApplicationPrivate::sys_palette || ! palette.isCopyOf(*QApplicationPrivate::sys_palette))) {
      if (! QApplicationPrivate::set_palette) {
         QApplicationPrivate::set_palette = new QPalette(palette);
      } else {
         *QApplicationPrivate::set_palette = palette;
      }

      applicationResourceFlags |= ApplicationPaletteExplicitlySet;
      QCoreApplication::setAttribute(Qt::AA_SetPalette);

      emit qGuiApp->paletteChanged(*QGuiApplicationPrivate::app_palette);
   }
}

void QApplication::setFont(const QFont &font, const QString &className)
{
   bool all = false;
   FontHash *hash = cs_app_fonts_hash();

   if (className.isEmpty()) {
      QMutexLocker locker(&applicationFontMutex);

      if (! QGuiApplicationPrivate::app_font) {
         QGuiApplicationPrivate::app_font = new QFont(font);
      } else {
         *QGuiApplicationPrivate::app_font = font;
      }

      applicationResourceFlags |= ApplicationFontExplicitlySet;
      if (hash && hash->size()) {
         all = true;
         hash->clear();
      }

   } else if (hash) {
      hash->insert(className, font);
   }

   if (QApplicationPrivate::is_app_running && !QApplicationPrivate::is_app_closing) {
      // Send ApplicationFontChange to qApp itself, and to the widgets.
      QEvent e(QEvent::ApplicationFontChange);
      QApplication::sendEvent(QApplication::instance(), &e);

      QWidgetList wids = QApplication::allWidgets();
      for (QWidgetList::const_iterator it = wids.constBegin(), cend = wids.constEnd(); it != cend; ++it) {
         QWidget *w = *it;

         if (all || (className.isEmpty() && w->isWindow()) || w->inherits(className)) {
            // matching class
            sendEvent(w, &e);
         }
      }

#ifndef QT_NO_GRAPHICSVIEW
      // Send to all scenes as well.
      QList<QGraphicsScene *> &scenes = qApp->d_func()->scene_list;

      for (QList<QGraphicsScene *>::const_iterator it = scenes.constBegin();
         it != scenes.constEnd(); ++it) {
         QApplication::sendEvent(*it, &e);
      }
#endif
   }

   if (className.isEmpty() && (! QApplicationPrivate::sys_font || ! font.isCopyOf(*QApplicationPrivate::sys_font))) {
      if (!QApplicationPrivate::set_font) {
         QApplicationPrivate::set_font = new QFont(font);
      } else {
         *QApplicationPrivate::set_font = font;
      }
   }
}

const QDrawHelperGammaTables *QGuiApplicationPrivate::gammaTables()
{
   QDrawHelperGammaTables *result = m_gammaTables.load();

   if (! result) {
      QDrawHelperGammaTables *tables   = new QDrawHelperGammaTables(fontSmoothingGamma);
      QDrawHelperGammaTables *expected = nullptr;

      if (! m_gammaTables.compareExchange(expected, tables, std::memory_order_release)) {
         delete tables;
      }

      result = m_gammaTables.load();
   }
   return result;
}

void QGuiApplicationPrivate::_q_updateFocusObject(QObject *object)
{
   Q_Q(QGuiApplication);

   QPlatformInputContext *inputContext = platformIntegration()->inputContext();
   bool enabled = false;

   if (object && inputContext) {
      QInputMethodQueryEvent query(Qt::ImEnabled | Qt::ImHints);
      QGuiApplication::sendEvent(object, &query);
      enabled = query.value(Qt::ImEnabled).toBool();

      if (enabled) {
         static const bool supportsHiddenText = inputContext->hasCapability(QPlatformInputContext::HiddenTextCapability);
         const Qt::InputMethodHints hints = static_cast<Qt::InputMethodHints>(query.value(Qt::ImHints).toInt());
         if ((hints & Qt::ImhHiddenText) && !supportsHiddenText) {
            enabled = false;
         }
      }
   }

   QPlatformInputContextPrivate::setInputMethodAccepted(enabled);

   if (inputContext) {
      inputContext->setFocusObject(object);
   }

   emit q->focusObjectChanged(object);
}

int QGuiApplicationPrivate::mouseEventCaps(QMouseEvent *event)
{
   return event->caps & MouseCapsMask;
}

QVector2D QGuiApplicationPrivate::mouseEventVelocity(QMouseEvent *event)
{
   return event->velocity;
}

void QGuiApplicationPrivate::setMouseEventCapsAndVelocity(QMouseEvent *event, int caps, const QVector2D &velocity)
{
   Q_ASSERT(caps <= MouseCapsMask);
   event->caps &= ~MouseCapsMask;
   event->caps |= caps & MouseCapsMask;
   event->velocity = velocity;
}

Qt::MouseEventSource QGuiApplicationPrivate::mouseEventSource(const QMouseEvent *event)
{
   return Qt::MouseEventSource((event->caps & MouseSourceMaskDst) >> MouseSourceShift);
}

void QGuiApplicationPrivate::setMouseEventSource(QMouseEvent *event, Qt::MouseEventSource source)
{
   // Mouse event synthesization status is encoded in the caps field because
   // QTouchDevice::CapabilityFlag uses only 6 bits from it.
   int value = source;
   Q_ASSERT(value <= MouseSourceMaskSrc);

   event->caps &= ~MouseSourceMaskDst;
   event->caps |= (value & MouseSourceMaskSrc) << MouseSourceShift;
}

Qt::MouseEventFlags QGuiApplicationPrivate::mouseEventFlags(const QMouseEvent *event)
{
   return Qt::MouseEventFlags((event->caps & MouseFlagsCapsMask) >> MouseFlagsShift);
}

void QGuiApplicationPrivate::setMouseEventFlags(QMouseEvent *event, Qt::MouseEventFlags flags)
{
   // use the 0x00FF0000 byte from caps (containing up to 7 mouse event flags)
   unsigned int value = flags;
   Q_ASSERT(value <= Qt::MouseEventFlagMask);

   event->caps &= ~MouseFlagsCapsMask;
   event->caps |= (value & Qt::MouseEventFlagMask) << MouseFlagsShift;
}

QInputDeviceManager *QGuiApplicationPrivate::inputDeviceManager()
{
   Q_ASSERT(QGuiApplication::instance());

   if (!m_inputDeviceManager) {
      m_inputDeviceManager = new QInputDeviceManager(QGuiApplication::instance());
   }

   return m_inputDeviceManager;
}

void QGuiApplication::_q_updateFocusObject(QObject *object)
{
  Q_D(QGuiApplication);
  d->_q_updateFocusObject(object);
}
