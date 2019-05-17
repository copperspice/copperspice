/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qplatformdefs.h>
#include <qabstracteventdispatcher.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qdir.h>
#include <qevent.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qgraphicsscene.h>
#include <qhash.h>
#include <qset.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylefactory.h>
#include <qtextcodec.h>
#include <qtooltip.h>
#include <qtranslator.h>
#include <qvariant.h>
#include <qwidget.h>
#include <qdnd_p.h>
#include <qguiapplication_p.h>
#include <qcolormap.h>
#include <qdebug.h>
#include <qstylesheetstyle_p.h>
#include <qstyle_p.h>
#include <qmessagebox.h>
#include <qwidgetwindow_p.h>
#include <qgraphicsproxywidget.h>
#include <qstylehints.h>
#include <qinputmethod.h>
#include <qwindow_p.h>
#include <qplatform_theme.h>
#include <qplatform_fontdatabase.h>
#include <qdatetime.h>
#include <qplatform_window.h>
#include <qgesture.h>
#include <qthread.h>

#ifndef QT_NO_WHATSTHIS
#include <qwhatsthis.h>
#endif

#include <qkeymapper_p.h>
#include <qaccessiblewidget_factory_p.h>
#include <qthread_p.h>
#include <qfont_p.h>
#include <qapplication_p.h>
#include <qevent_p.h>
#include <qwidget_p.h>
#include <qgesturemanager_p.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>       // for qt_win_display_dc()
#endif

#include <cmath>
#include <stdlib.h>

// #define ALIEN_DEBUG

void qt_init(QApplicationPrivate *priv, int type);
void qt_init_tooltip_palette();
void qt_cleanup();

#if ! defined(QT_NO_STATEMACHINE)
int qRegisterGuiStateMachine();
int qUnregisterGuiStateMachine();
#endif

// Helper macro for static functions to check on the existence of the application class.
#define CHECK_QAPP_INSTANCE(...) \
    if (Q_LIKELY(QCoreApplication::instance())) { \
    } else { \
        qWarning("Must construct a QApplication first."); \
        return __VA_ARGS__; \
    }

bool QApplicationPrivate::autoSipEnabled = true;


QApplicationPrivate::QApplicationPrivate(int &argc, char **argv, int flags)
   : QCoreApplicationPrivate(argc, argv, flags),
     inputMethod(0), lastTouchType(QEvent::TouchEnd), ownGlobalShareContext(false)
{
   self = this;
   application_type = QCoreApplicationPrivate::Gui;

#ifndef QT_NO_SESSIONMANAGER
   is_session_restored = false;
   is_saving_session   = false;
#endif

#ifndef QT_NO_GESTURES
   gestureManager = 0;
   gestureWidget  = 0;
#endif
}


QWidget *QApplication::topLevelWidgetAt(const QPoint &pos)
{
   if (const QWindow *window = QApplication::topLevelWindowAt(pos)) {

      if (const QWidgetWindow *widgetWindow = qobject_cast<const QWidgetWindow *>(window)) {
         return widgetWindow->widget();
      }
   }

   return nullptr;
}




QStyle *QApplicationPrivate::app_style = 0;               // default application style
bool QApplicationPrivate::overrides_native_style = false; // whether native QApplication style is

// overridden, i.e. not native
#ifndef QT_NO_STYLE_STYLESHEET
QString QApplicationPrivate::styleSheet;                // default application stylesheet
#endif

QPointer<QWidget> QApplicationPrivate::leaveAfterRelease = 0;

int QApplicationPrivate::app_cspec = QApplication::NormalColor;

QPalette *QApplicationPrivate::sys_pal = 0;             // default system palette
QPalette *QApplicationPrivate::set_pal = 0;             // default palette set by programmer

QFont *QApplicationPrivate::sys_font = 0;               // default system font
QFont *QApplicationPrivate::set_font = 0;               // default font set by programmer

QWidget *QApplicationPrivate::main_widget = 0;          // main application widget
QWidget *QApplicationPrivate::focus_widget = 0;         // has keyboard input focus
QWidget *QApplicationPrivate::hidden_focus_widget = 0;  // will get keyboard input focus after show()
QWidget *QApplicationPrivate::active_window = 0;        // toplevel with keyboard focus

QWidgetList *QApplicationPrivate::popupWidgets = 0;     // has keyboard input focus


#ifndef QT_NO_WHEELEVENT
int QApplicationPrivate::wheel_scroll_lines;            // number of lines to scroll
QPointer<QWidget> QApplicationPrivate::wheel_widget;
#endif

bool qt_in_tab_key_event = false;
int qt_antialiasing_threshold = -1;
QSize QApplicationPrivate::app_strut = QSize(0, 0); // no default application strut
int QApplicationPrivate::enabledAnimations = QPlatformTheme::GeneralUiEffect;
bool QApplicationPrivate::widgetCount = false;

#ifdef QT_KEYPAD_NAVIGATION
Qt::NavigationMode QApplicationPrivate::navigationMode = Qt::NavigationModeKeypadTabOrder;
QWidget *QApplicationPrivate::oldEditFocus = 0;
#endif

bool qt_tabletChokeMouse = false;

inline bool QApplicationPrivate::isAlien(QWidget *widget)
{
   return widget && !widget->isWindow();
}

bool Q_GUI_EXPORT qt_tab_all_widgets()
{
   return QGuiApplication::styleHints()->tabFocusBehavior() == Qt::TabFocusAllControls;
}

// Default application palettes and fonts (per widget type)
PaletteHash *cs_app_palettes_hash()
{
   static PaletteHash retval;
   return &retval;
}

FontHash *cs_app_fonts_hash()
{
   static FontHash retval;
   return &retval;
}

/*!
    \internal
*/
void QApplicationPrivate::process_cmdline()
{
   if (styleOverride.isEmpty() && ! qgetenv("QT_STYLE_OVERRIDE").isEmpty()) {
      styleOverride = QString::fromUtf8(qgetenv("QT_STYLE_OVERRIDE"));
   }

   if (!styleOverride.isEmpty()) {
      if (app_style) {
         delete app_style;
         app_style = 0;
      }
   }

   // process platform-indep command line
   if (! qt_is_gui_used || ! argc) {
      return;
   }

   int i;
   int j = 1;

   for (i = 1; i < argc; i++) {
      // if you add anything here, modify QCoreApplication::arguments()

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

#ifndef QT_NO_STYLE_STYLESHEET
      if (arg == "-stylesheet" && i < argc - 1) {
         styleSheet = "file:///";
         styleSheet.append(QString::fromUtf8(argv[++i]));

      } else if (arg.startsWith("-stylesheet=")) {
         styleSheet = "file:///";
         styleSheet.append(arg.mid(12));

      }
#endif

      if (arg == "-widgetcount") {
         widgetCount = true;

      } else {
         argv[j++] = argv[i];
      }
   }

   if (j < argc) {
      argv[j] = 0;
      argc = j;
   }
}

void qt_init(QApplicationPrivate *priv, int type)
{

   QColormap::initialize();

   qt_init_tooltip_palette();

   QApplicationPrivate::initializeWidgetFontHash();
}

void qt_init_tooltip_palette()
{
#ifndef QT_NO_TOOLTIP
   if (const QPalette *toolTipPalette = QGuiApplicationPrivate::platformTheme()->palette(QPlatformTheme::ToolTipPalette)) {
      QToolTip::setPalette(*toolTipPalette);
   }
#endif
}


void QApplicationPrivate::initialize()
{
   is_app_running = false; // Starting up

   QWidgetPrivate::mapper     = new QWidgetMapper;
   QWidgetPrivate::allWidgets = new QWidgetSet;



   // emerald - needed for widgets in QML, not supported at this time
   // QAbstractDeclarativeData::setWidgetParent = QWidgetPrivate::setWidgetParentHelper;

   if (application_type != QApplicationPrivate::Tty) {
      (void) QApplication::style();   // trigger creation of application style
   }

#ifndef QT_NO_STATEMACHINE
   // trigger registering of QStateMachine's GUI types
   qRegisterGuiStateMachine();
#endif

   if (qgetenv("QT_USE_NATIVE_WINDOWS").toInt() > 0) {
      QCoreApplication::setAttribute(Qt::AA_NativeWindows);
   }

#ifndef QT_NO_WHEELEVENT
   QApplicationPrivate::wheel_scroll_lines = 3;
#endif

   if (qt_is_gui_used) {
      initializeMultitouch();
   }

   if (QApplication::desktopSettingsAware())
      if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme()) {
         QApplicationPrivate::enabledAnimations = theme->themeHint(QPlatformTheme::UiEffects).toInt();

#ifndef QT_NO_WHEELEVENT
         QApplicationPrivate::wheel_scroll_lines = theme->themeHint(QPlatformTheme::WheelScrollLines).toInt();
#endif
      }

   is_app_running = true; // no longer starting up
}

static void setPossiblePalette(const QPalette *palette, const QString &className)
{
   if (palette == 0) {
      return;
   }

   QApplicationPrivate::setPalette_helper(*palette, className, false);
}

void QApplicationPrivate::initializeWidgetPaletteHash()
{
   QPlatformTheme *platformTheme = QGuiApplicationPrivate::platformTheme();

   if (! platformTheme) {
      return;
   }

   cs_app_palettes_hash()->clear();

   setPossiblePalette(platformTheme->palette(QPlatformTheme::ToolButtonPalette), "QToolButton");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::ButtonPalette), "QAbstractButton");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::CheckBoxPalette), "QCheckBox");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::RadioButtonPalette), "QRadioButton");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::HeaderPalette), "QHeaderView");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::ItemViewPalette), "QAbstractItemView");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::MessageBoxLabelPalette), "QMessageBoxLabel");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::TabBarPalette), "QTabBar");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::LabelPalette), "QLabel");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::GroupBoxPalette), "QGroupBox");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::MenuPalette), "QMenu");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::MenuBarPalette), "QMenuBar");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::TextEditPalette), "QTextEdit");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::TextEditPalette), "QTextControl");
   setPossiblePalette(platformTheme->palette(QPlatformTheme::TextLineEditPalette), "QLineEdit");
}

void QApplicationPrivate::initializeWidgetFontHash()
{
   const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme();
   if (!theme) {
      return;
   }
   FontHash *fontHash = cs_app_fonts_hash();
   fontHash->clear();

   if (const QFont *font = theme->font(QPlatformTheme::MenuFont)) {
      fontHash->insert("QMenu", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::MenuBarFont)) {
      fontHash->insert("QMenuBar", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::MenuItemFont)) {
      fontHash->insert("QMenuItem", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::MessageBoxFont)) {
      fontHash->insert("QMessageBox", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::LabelFont)) {
      fontHash->insert("QLabel", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::TipLabelFont)) {
      fontHash->insert("QTipLabel", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::TitleBarFont)) {
      fontHash->insert("QTitleBar", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::StatusBarFont)) {
      fontHash->insert("QStatusBar", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::MdiSubWindowTitleFont)) {
      fontHash->insert("QMdiSubWindowTitleBar", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::DockWidgetTitleFont)) {
      fontHash->insert("QDockWidgetTitle", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::PushButtonFont)) {
      fontHash->insert("QPushButton", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::CheckBoxFont)) {
      fontHash->insert("QCheckBox", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::RadioButtonFont)) {
      fontHash->insert("QRadioButton", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::ToolButtonFont)) {
      fontHash->insert("QToolButton", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::ItemViewFont)) {
      fontHash->insert("QAbstractItemView", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::ListViewFont)) {
      fontHash->insert("QListView", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::HeaderViewFont)) {
      fontHash->insert("QHeaderView", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::ListBoxFont)) {
      fontHash->insert("QListBox", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::ComboMenuItemFont)) {
      fontHash->insert("QComboMenuItem", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::ComboLineEditFont)) {
      fontHash->insert("QComboLineEdit", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::SmallFont)) {
      fontHash->insert("QSmallFont", *font);
   }
   if (const QFont *font = theme->font(QPlatformTheme::MiniFont)) {
      fontHash->insert("QMiniFont", *font);
   }
}


QWidget *QApplication::activePopupWidget()
{
   return QApplicationPrivate::popupWidgets && !QApplicationPrivate::popupWidgets->isEmpty() ?
      QApplicationPrivate::popupWidgets->last() : 0;
}

QWidget *QApplication::activeModalWidget()
{
   QWidgetWindow *widgetWindow = qobject_cast<QWidgetWindow *>(modalWindow());
   return widgetWindow ? widgetWindow->widget() : 0;
}


#if defined(Q_OS_WIN)
// #fixme: Remove
static HDC displayDC = 0;                // display device context

Q_GUI_EXPORT HDC qt_win_display_dc()                        // get display DC
{
   Q_ASSERT(qApp && qApp->thread() == QThread::currentThread());
   if (!displayDC) {
      displayDC = GetDC(0);
   }
   return displayDC;
}
#endif

void qt_cleanup()
{
   QPixmapCache::clear();
   QColormap::cleanup();

   QApplicationPrivate::active_window = 0; //### this should not be necessary

#if defined(Q_OS_WIN)
   if (displayDC) {
      ReleaseDC(0, displayDC);
      displayDC = 0;
   }
#endif
}


QWidget *QApplication::widgetAt(const QPoint &p)
{
   QWidget *window = QApplication::topLevelWidgetAt(p);

   if (! window) {
      return 0;
   }

   QWidget *child = 0;

   if (!window->testAttribute(Qt::WA_TransparentForMouseEvents)) {
      child = window->childAt(window->mapFromGlobal(p));
   }

   if (child) {
      return child;
   }

   if (window->testAttribute(Qt::WA_TransparentForMouseEvents)) {
      //shoot a hole in the widget and try once again,
      //suboptimal on Qt for Embedded Linux where we do
      //know the stacking order of the toplevels.

      int x = p.x();
      int y = p.y();
      QRegion oldmask = window->mask();
      QPoint wpoint = window->mapFromGlobal(QPoint(x, y));

      QRegion newmask = (oldmask.isEmpty() ? QRegion(window->rect()) : oldmask)
         - QRegion(wpoint.x(), wpoint.y(), 1, 1);

      window->setMask(newmask);
      QWidget *recurse = 0;

      if (QApplication::topLevelWidgetAt(p) != window) {
         // verify recursion will terminate
         recurse = widgetAt(x, y);
      }

      if (oldmask.isEmpty()) {
         window->clearMask();
      } else {
         window->setMask(oldmask);
      }
      return recurse;
   }
   return window;
}


bool QApplication::compressEvent(QEvent *event, QObject *receiver, QPostEventList *postedEvents)
{
   if ((event->type() == QEvent::UpdateRequest
         || event->type() == QEvent::LayoutRequest
         || event->type() == QEvent::Resize
         || event->type() == QEvent::Move
         || event->type() == QEvent::LanguageChange)) {

      for (QPostEventList::const_iterator it = postedEvents->constBegin(); it != postedEvents->constEnd(); ++it) {
         const QPostEvent &cur = *it;
         if (cur.receiver != receiver || cur.event == 0 || cur.event->type() != event->type()) {
            continue;
         }
         if (cur.event->type() == QEvent::LayoutRequest
            || cur.event->type() == QEvent::UpdateRequest) {
            ;
         } else if (cur.event->type() == QEvent::Resize) {
            ((QResizeEvent *)(cur.event))->s = ((QResizeEvent *)event)->s;
         } else if (cur.event->type() == QEvent::Move) {
            ((QMoveEvent *)(cur.event))->p = ((QMoveEvent *)event)->p;
         } else if (cur.event->type() == QEvent::LanguageChange) {
            ;
         } else {
            continue;
         }

         delete event;
         return true;
      }

      return false;
   }

   return QCoreApplication::compressEvent(event, receiver, postedEvents);
}


void QApplication::setAutoSipEnabled(const bool enabled)
{
   QApplicationPrivate::autoSipEnabled = enabled;
}

bool QApplication::autoSipEnabled() const
{
   return QApplicationPrivate::autoSipEnabled;
}

#ifndef QT_NO_STYLE_STYLESHEET

QString QApplication::styleSheet() const
{
   return QApplicationPrivate::styleSheet;
}

void QApplication::setStyleSheet(const QString &styleSheet)
{
   QApplicationPrivate::styleSheet = styleSheet;
   QStyleSheetStyle *proxy = qobject_cast<QStyleSheetStyle *>(QApplicationPrivate::app_style);
   if (styleSheet.isEmpty()) { // application style sheet removed
      if (!proxy) {
         return;   // there was no stylesheet before
      }
      setStyle(proxy->base);
   } else if (proxy) { // style sheet update, just repolish
      proxy->repolish(qApp);
   } else { // stylesheet set the first time
      QStyleSheetStyle *newProxy = new QStyleSheetStyle(QApplicationPrivate::app_style);
      QApplicationPrivate::app_style->setParent(newProxy);
      setStyle(newProxy);
   }
}

#endif // QT_NO_STYLE_STYLESHEET



/*!
    \overload

    Requests a QStyle object for \a style from the QStyleFactory.

    The string must be one of the QStyleFactory::keys(), typically one of
    "windows", "fusion", "windowsxp", or "macintosh". Style
    names are case insensitive.

    Returns 0 if an unknown \a style is passed, otherwise the QStyle object
    returned is set as the application's GUI style.

    \warning To ensure that the application's style is set correctly, it is
    best to call this function before the QApplication constructor, if
    possible.
*/
QStyle *QApplication::setStyle(const QString &style)
{
   QStyle *s = QStyleFactory::create(style);
   if (!s) {
      return 0;
   }

   setStyle(s);
   return s;
}

/*!
    Returns the color specification.

    \sa QApplication::setColorSpec()
*/

int QApplication::colorSpec()
{
   return QApplicationPrivate::app_cspec;
}

/*!
    Sets the color specification for the application to \a spec.

    The color specification controls how the application allocates colors when
    run on a display with a limited amount of colors, e.g. 8 bit / 256 color
    displays.

    The color specification must be set before you create the QApplication
    object.

    The options are:
    \list
        \li  QApplication::NormalColor. This is the default color allocation
            strategy. Use this option if your application uses buttons, menus,
            texts and pixmaps with few colors. With this option, the
            application uses system global colors. This works fine for most
            applications under X11, but on the Windows platform, it may cause
            dithering of non-standard colors.
        \li  QApplication::CustomColor. Use this option if your application
            needs a small number of custom colors. On X11, this option is the
            same as NormalColor. On Windows, Qt creates a Windows palette, and
            allocates colors to it on demand.
        \li  QApplication::ManyColor. Use this option if your application is
            very color hungry, e.g., it requires thousands of colors. \br
            Under X11 the effect is:
            \list
                \li  For 256-color displays which have at best a 256 color true
                    color visual, the default visual is used, and colors are
                    allocated from a color cube. The color cube is the 6x6x6
                    (216 color) "Web palette" (the red, green, and blue
                    components always have one of the following values: 0x00,
                    0x33, 0x66, 0x99, 0xCC, or 0xFF), but the number of colors
                    can be changed by the \e -ncols option. The user can force
                    the application to use the true color visual with the
                    \l{QApplication::QApplication()}{-visual} option.
                \li  For 256-color displays which have a true color visual with
                    more than 256 colors, use that visual. Silicon Graphics X
                    servers this feature, for example. They provide an 8 bit
                    visual by default but can deliver true color when asked.
            \endlist
            On Windows, Qt creates a Windows palette, and fills it with a color
            cube.
    \endlist

    Be aware that the CustomColor and ManyColor choices may lead to colormap
    flashing: The foreground application gets (most) of the available colors,
    while the background windows will look less attractive.

    Example:

    \snippet code/src_gui_kernel_qapplication.cpp 2

    \sa colorSpec()
*/

void QApplication::setColorSpec(int spec)
{
   if (qApp)
      qWarning("QApplication::setColorSpec: This function must be "
         "called before the QApplication object is created");
   QApplicationPrivate::app_cspec = spec;
}

/*!
    \property QApplication::globalStrut
    \brief the minimum size that any GUI element that the user can interact
           with should have

    For example, no button should be resized to be smaller than the global
    strut size. The strut size should be considered when reimplementing GUI
    controls that may be used on touch-screens or similar I/O devices.

    Example:

    \snippet code/src_gui_kernel_qapplication.cpp 3

    By default, this property contains a QSize object with zero width and height.
*/
QSize QApplication::globalStrut()
{
   return QApplicationPrivate::app_strut;
}

void QApplication::setGlobalStrut(const QSize &strut)
{
   QApplicationPrivate::app_strut = strut;
}

QPalette QApplication::palette(const QWidget *w)
{
   PaletteHash *hash = cs_app_palettes_hash();

   if (w && hash && hash->size()) {
      auto it         = hash->constFind(w->metaObject()->className());
      const auto cend = hash->constEnd();

      if (it != cend) {
         return *it;
      }

      for (it = hash->constBegin(); it != cend; ++it) {
         if (w->inherits(it.key())) {
            return it.value();
         }
      }
   }

   return palette();
}


QPalette QApplication::palette(const QString &className)
{
   if (!QApplicationPrivate::app_pal) {
      palette();
   }

   PaletteHash *hash = cs_app_palettes_hash();

   if (! className.isEmpty() && hash && hash->size()) {
      QHash<QString, QPalette>::const_iterator it = hash->constFind(className);

      if (it != hash->constEnd()) {
         return *it;
      }
   }

   return *QApplicationPrivate::app_pal;
}


void QApplication::setPalette(const QPalette &palette, const QString &className)
{
   QApplicationPrivate::setPalette_helper(palette, className, true);
}

void QApplicationPrivate::setSystemPalette(const QPalette &pal)
{
   QPalette adjusted;

#if 0
   // adjust the system palette to avoid dithering
   QColormap cmap = QColormap::instance();
   if (cmap.depths() > 4 && cmap.depths() < 24) {
      for (int g = 0; g < QPalette::NColorGroups; g++)
         for (int i = 0; i < QPalette::NColorRoles; i++) {
            QColor color = pal.color((QPalette::ColorGroup)g, (QPalette::ColorRole)i);
            color = cmap.colorAt(cmap.pixel(color));
            adjusted.setColor((QPalette::ColorGroup)g, (QPalette::ColorRole) i, color);
         }
   }
#else
   adjusted = pal;
#endif

   if (!sys_pal) {
      sys_pal = new QPalette(adjusted);
   } else {
      *sys_pal = adjusted;
   }


   if (!QApplicationPrivate::set_pal) {
      QApplication::setPalette(*sys_pal);
   }
}



QFont QApplication::font(const QWidget *widget)
{
   FontHash *hash = cs_app_fonts_hash();

   if (widget && hash  && hash->size()) {

#ifdef Q_OS_MAC
      // short circuit for small and mini controls

      if (widget->testAttribute(Qt::WA_MacSmallSize)) {
         return hash->value("QSmallFont");

      } else if (widget->testAttribute(Qt::WA_MacMiniSize)) {
         return hash->value("QMiniFont");
      }
#endif

      auto it         = hash->constFind(widget->metaObject()->className());
      const auto cend = hash->constEnd();

      if (it != cend) {
         return it.value();
      }

      for (it = hash->constBegin(); it != cend; ++it) {
         if (widget->inherits(it.key())) {
            return it.value();
         }
      }
   }
   return font();
}

QFont QApplication::font(const QString &className)
{
   FontHash *hash = cs_app_fonts_hash();

   if (! className.isEmpty() && hash && hash->size()) {
      QHash<QString, QFont>::const_iterator it = hash->constFind(className);

      if (it != hash->constEnd()) {
         return *it;
      }
   }
   return font();
}





/*! \internal
*/
void QApplicationPrivate::setSystemFont(const QFont &font)
{
   if (!sys_font) {
      sys_font = new QFont(font);
   } else {
      *sys_font = font;
   }

   if (!QApplicationPrivate::set_font) {
      QApplication::setFont(*sys_font);
   }
}

/*! \internal
*/
QString QApplicationPrivate::desktopStyleKey()
{
   // The platform theme might return a style that is not available, find
   // first valid one.
   if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme()) {
      const QStringList availableKeys = QStyleFactory::keys();
      foreach (const QString &style, theme->themeHint(QPlatformTheme::StyleNames).toStringList())
         if (availableKeys.contains(style, Qt::CaseInsensitive)) {
            return style;
         }
   }
   return QString();
}


void QApplicationPrivate::notifyWindowIconChanged()
{
   QEvent ev(QEvent::ApplicationWindowIconChange);
   const QWidgetList list = QApplication::topLevelWidgets();
   QWindowList windowList = QGuiApplication::topLevelWindows();

   // send to all top-level QWidgets
   for (int i = 0; i < list.size(); ++i) {
      QWidget *w = list.at(i);
      windowList.removeOne(w->windowHandle());
      QCoreApplication::sendEvent(w, &ev);
   }

   // in case there are any plain QWindows in this QApplication-using
   // application, also send the notification to them
   for (int i = 0; i < windowList.size(); ++i) {
      QCoreApplication::sendEvent(windowList.at(i), &ev);
   }
}


QWidgetList QApplication::topLevelWidgets()
{
   QWidgetList list;
   QWidgetList all = allWidgets();

   for (QWidgetList::const_iterator it = all.constBegin(), cend = all.constEnd(); it != cend; ++it) {
      QWidget *w = *it;

      if (w->isWindow() && w->windowType() != Qt::Desktop) {
         list.append(w);
      }
   }
   return list;
}

/*!
    Returns a list of all the widgets in the application.

    The list is empty (QList::isEmpty()) if there are no widgets.

    \note Some of the widgets may be hidden.

    Example:
    \snippet code/src_gui_kernel_qapplication.cpp 5

    \sa topLevelWidgets(), QWidget::isVisible()
*/

QWidgetList QApplication::allWidgets()
{
   if (QWidgetPrivate::allWidgets) {
      return QWidgetPrivate::allWidgets->toList();
   }
   return QWidgetList();
}

/*!
    Returns the application widget that has the keyboard input focus, or 0 if
    no widget in this application has the focus.

    \sa QWidget::setFocus(), QWidget::hasFocus(), activeWindow(), focusChanged()
*/

QWidget *QApplication::focusWidget()
{
   return QApplicationPrivate::focus_widget;
}

void QApplicationPrivate::setFocusWidget(QWidget *focus, Qt::FocusReason reason)
{
#ifndef QT_NO_GRAPHICSVIEW
   if (focus && focus->window()->graphicsProxyWidget()) {
      return;
   }
#endif

   hidden_focus_widget = 0;

   if (focus != focus_widget) {
      if (focus && focus->isHidden()) {
         hidden_focus_widget = focus;
         return;
      }

      if (focus && (reason == Qt::BacktabFocusReason || reason == Qt::TabFocusReason)
         && qt_in_tab_key_event) {
         focus->window()->setAttribute(Qt::WA_KeyboardFocusChange);
      } else if (focus && reason == Qt::ShortcutFocusReason) {
         focus->window()->setAttribute(Qt::WA_KeyboardFocusChange);
      }
      QWidget *prev = focus_widget;
      focus_widget = focus;

      if (focus_widget) {
         focus_widget->d_func()->setFocus_sys();
      }

      if (reason != Qt::NoFocusReason) {

         //send events
         if (prev) {
#ifdef QT_KEYPAD_NAVIGATION
            if (QApplication::keypadNavigationEnabled()) {
               if (prev->hasEditFocus() && reason != Qt::PopupFocusReason) {
                  prev->setEditFocus(false);
               }
            }
#endif
            QFocusEvent out(QEvent::FocusOut, reason);
            QPointer<QWidget> that = prev;
            QApplication::sendEvent(prev, &out);
            if (that) {
               QApplication::sendEvent(that->style(), &out);
            }
         }
         if (focus && QApplicationPrivate::focus_widget == focus) {
            QFocusEvent in(QEvent::FocusIn, reason);
            QPointer<QWidget> that = focus;
            QApplication::sendEvent(focus, &in);
            if (that) {
               QApplication::sendEvent(that->style(), &in);
            }
         }
         emit qApp->focusChanged(prev, focus_widget);
      }
   }
}


/*!
    Returns the application top-level window that has the keyboard input focus,
    or 0 if no application window has the focus. There might be an
    activeWindow() even if there is no focusWidget(), for example if no widget
    in that window accepts key events.

    \sa QWidget::setFocus(), QWidget::hasFocus(), focusWidget()
*/

QWidget *QApplication::activeWindow()
{
   return QApplicationPrivate::active_window;
}

/*!
    Returns display (screen) font metrics for the application font.

    \sa font(), setFont(), QWidget::fontMetrics(), QPainter::fontMetrics()
*/

QFontMetrics QApplication::fontMetrics()
{
   return desktop()->fontMetrics();
}

bool QApplicationPrivate::tryCloseAllWidgetWindows(QWindowList *processedWindows)
{
   Q_ASSERT(processedWindows);
   while (QWidget *w = QApplication::activeModalWidget()) {
      if (!w->isVisible() || w->data->is_closing) {
         break;
      }
      QWindow *window = w->windowHandle();
      if (!window->close()) { // Qt::WA_DeleteOnClose may cause deletion.
         return false;
      }
      if (window) {
         processedWindows->append(window);
      }
   }

   QWidgetList list = QApplication::topLevelWidgets();
   for (int i = 0; i < list.size(); ++i) {
      QWidget *w = list.at(i);
      if (w->isVisible() && w->windowType() != Qt::Desktop &&
         !w->testAttribute(Qt::WA_DontShowOnScreen) && !w->data->is_closing) {
         QWindow *window = w->windowHandle();
         if (!window->close()) { // Qt::WA_DeleteOnClose may cause deletion.
            return false;
         }
         if (window) {
            processedWindows->append(window);
         }
         list = QApplication::topLevelWidgets();
         i = -1;
      }
   }
   return true;
}

bool QApplicationPrivate::tryCloseAllWindows()
{
   QWindowList processedWindows;

   return QApplicationPrivate::tryCloseAllWidgetWindows(&processedWindows)
      && QGuiApplicationPrivate::tryCloseRemainingWindows(processedWindows);
}


void QApplication::closeAllWindows()
{
   QWindowList processedWindows;
   QApplicationPrivate::tryCloseAllWidgetWindows(&processedWindows);
}


void QApplication::aboutCs()
{
#ifndef QT_NO_MESSAGEBOX
   QMessageBox::aboutCs(activeWindow());
#endif
}

void QApplication::aboutQt()
{
#ifndef QT_NO_MESSAGEBOX
   QMessageBox::aboutCs(activeWindow());
#endif
}



// ### FIXME: topLevelWindows does not contain QWidgets without a parent until
// create_sys is called. Need to override QGuiApplication::notifyLayoutDirectionChange
// to do the right thing

void QApplicationPrivate::notifyLayoutDirectionChange()
{
   const QWidgetList list = QApplication::topLevelWidgets();
   QWindowList windowList = QGuiApplication::topLevelWindows();

   // send to all top-level QWidgets
   for (int i = 0; i < list.size(); ++i) {
      QWidget *w = list.at(i);
      windowList.removeAll(w->windowHandle());
      QEvent ev(QEvent::ApplicationLayoutDirectionChange);
      QCoreApplication::sendEvent(w, &ev);
   }

   // in case there are any plain QWindows in this QApplication-using
   // application, also send the notification to them
   for (int i = 0; i < windowList.size(); ++i) {
      QEvent ev(QEvent::ApplicationLayoutDirectionChange);
      QCoreApplication::sendEvent(windowList.at(i), &ev);
   }
}

/*!
    \fn void QApplication::setActiveWindow(QWidget* active)

    Sets the active window to the \a active widget in response to a system
    event. The function is called from the platform specific event handlers.

    \warning This function does \e not set the keyboard focus to the active
    widget. Call QWidget::activateWindow() instead.

    It sets the activeWindow() and focusWidget() attributes and sends proper
    \l{QEvent::WindowActivate}{WindowActivate}/\l{QEvent::WindowDeactivate}
    {WindowDeactivate} and \l{QEvent::FocusIn}{FocusIn}/\l{QEvent::FocusOut}
    {FocusOut} events to all appropriate widgets. The window will then be
    painted in active state (e.g. cursors in line edits will blink), and it
    will have tool tips enabled.

    \sa activeWindow(), QWidget::activateWindow()
*/
void QApplication::setActiveWindow(QWidget *act)
{
   QWidget *window = act ? act->window() : 0;

   if (QApplicationPrivate::active_window == window) {
      return;
   }

#ifndef QT_NO_GRAPHICSVIEW
   if (window && window->graphicsProxyWidget()) {
      // Activate the proxy's view->viewport() ?
      return;
   }
#endif

   QWidgetList toBeActivated;
   QWidgetList toBeDeactivated;

   if (QApplicationPrivate::active_window) {
      if (style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, QApplicationPrivate::active_window)) {
         QWidgetList list = topLevelWidgets();
         for (int i = 0; i < list.size(); ++i) {
            QWidget *w = list.at(i);
            if (w->isVisible() && w->isActiveWindow()) {
               toBeDeactivated.append(w);
            }
         }
      } else {
         toBeDeactivated.append(QApplicationPrivate::active_window);
      }
   }

   if (QApplicationPrivate::focus_widget) {
      if (QApplicationPrivate::focus_widget->testAttribute(Qt::WA_InputMethodEnabled)) {
         QGuiApplication::inputMethod()->commit();
      }

      QFocusEvent focusAboutToChange(QEvent::FocusAboutToChange, Qt::ActiveWindowFocusReason);
      QApplication::sendEvent(QApplicationPrivate::focus_widget, &focusAboutToChange);
   }

   QApplicationPrivate::active_window = window;

   if (QApplicationPrivate::active_window) {
      if (style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, QApplicationPrivate::active_window)) {
         QWidgetList list = topLevelWidgets();
         for (int i = 0; i < list.size(); ++i) {
            QWidget *w = list.at(i);
            if (w->isVisible() && w->isActiveWindow()) {
               toBeActivated.append(w);
            }
         }
      } else {
         toBeActivated.append(QApplicationPrivate::active_window);
      }

   }

   // first the activation/deactivation events
   QEvent activationChange(QEvent::ActivationChange);
   QEvent windowActivate(QEvent::WindowActivate);
   QEvent windowDeactivate(QEvent::WindowDeactivate);

   for (int i = 0; i < toBeActivated.size(); ++i) {
      QWidget *w = toBeActivated.at(i);
      sendSpontaneousEvent(w, &windowActivate);
      sendSpontaneousEvent(w, &activationChange);
   }

   for (int i = 0; i < toBeDeactivated.size(); ++i) {
      QWidget *w = toBeDeactivated.at(i);
      sendSpontaneousEvent(w, &windowDeactivate);
      sendSpontaneousEvent(w, &activationChange);
   }

   if (QApplicationPrivate::popupWidgets == 0) { // !inPopupMode()
      // then focus events
      if (!QApplicationPrivate::active_window && QApplicationPrivate::focus_widget) {
         QApplicationPrivate::setFocusWidget(0, Qt::ActiveWindowFocusReason);
      } else if (QApplicationPrivate::active_window) {
         QWidget *w = QApplicationPrivate::active_window->focusWidget();
         if (w && w->isVisible() /*&& w->focusPolicy() != QWidget::NoFocus*/) {
            w->setFocus(Qt::ActiveWindowFocusReason);
         } else {
            w = QApplicationPrivate::focusNextPrevChild_helper(QApplicationPrivate::active_window, true);
            if (w) {
               w->setFocus(Qt::ActiveWindowFocusReason);
            } else {
               // If the focus widget is not in the activate_window, clear the focus
               w = QApplicationPrivate::focus_widget;
               if (!w && QApplicationPrivate::active_window->focusPolicy() != Qt::NoFocus) {
                  QApplicationPrivate::setFocusWidget(QApplicationPrivate::active_window, Qt::ActiveWindowFocusReason);
               } else if (!QApplicationPrivate::active_window->isAncestorOf(w)) {
                  QApplicationPrivate::setFocusWidget(0, Qt::ActiveWindowFocusReason);
               }
            }
         }
      }
   }
}

QWidget *qt_tlw_for_window(QWindow *wnd)
{
   // QTBUG-32177, wnd might be a QQuickView embedded via window container.
   while (wnd && !wnd->isTopLevel()) {
      QWindow *parent = wnd->parent();
      // Don't end up in windows not belonging to this application
      if (parent && parent->type() != Qt::ForeignWindow) {
         wnd = wnd->parent();
      } else {
         break;
      }
   }
   if (wnd)
      foreach (QWidget *tlw, qApp->topLevelWidgets())
         if (tlw->windowHandle() == wnd) {
            return tlw;
         }
   return 0;
}

void QApplicationPrivate::notifyActiveWindowChange(QWindow *previous)
{
   Q_UNUSED(previous);
   QWindow *wnd = QGuiApplicationPrivate::focus_window;
   if (inPopupMode()) { // some delayed focus event to ignore
      return;
   }
   QWidget *tlw = qt_tlw_for_window(wnd);
   QApplication::setActiveWindow(tlw);
   // QTBUG-37126, Active X controls may set the focus on native child widgets.
   if (wnd && tlw && wnd != tlw->windowHandle()) {
      if (QWidgetWindow *widgetWindow = qobject_cast<QWidgetWindow *>(wnd))
         if (QWidget *widget = widgetWindow->widget())
            if (widget->inherits("QAxHostWidget")) {
               widget->setFocus(Qt::ActiveWindowFocusReason);
            }
   }
}

/*!internal
 * Helper function that returns the new focus widget, but does not set the focus reason.
 * Returns 0 if a new focus widget could not be found.
 * Shared with QGraphicsProxyWidgetPrivate::findFocusChild()
*/
QWidget *QApplicationPrivate::focusNextPrevChild_helper(QWidget *toplevel, bool next,
   bool *wrappingOccurred)
{
   uint focus_flag = qt_tab_all_widgets() ? Qt::TabFocus : Qt::StrongFocus;

   QWidget *f = toplevel->focusWidget();
   if (!f) {
      f = toplevel;
   }

   QWidget *w = f;
   QWidget *test = f->d_func()->focus_next;
   bool seenWindow = false;
   bool focusWidgetAfterWindow = false;
   while (test && test != f) {
      if (test->isWindow()) {
         seenWindow = true;
      }

      if ((test->focusPolicy() & focus_flag) == focus_flag
         && !(test->d_func()->extra && test->d_func()->extra->focus_proxy)
         && test->isVisibleTo(toplevel) && test->isEnabled()
         && !(w->windowType() == Qt::SubWindow && !w->isAncestorOf(test))
         && (toplevel->windowType() != Qt::SubWindow || toplevel->isAncestorOf(test))) {
         w = test;
         if (seenWindow) {
            focusWidgetAfterWindow = true;
         }
         if (next) {
            break;
         }
      }
      test = test->d_func()->focus_next;
   }

   if (wrappingOccurred != 0) {
      *wrappingOccurred = next ? focusWidgetAfterWindow : !focusWidgetAfterWindow;
   }

   if (w == f) {
      if (qt_in_tab_key_event) {
         w->window()->setAttribute(Qt::WA_KeyboardFocusChange);
         w->update();
      }
      return 0;
   }
   return w;
}

/*!
    \fn void QApplicationPrivate::dispatchEnterLeave(QWidget* enter, QWidget* leave, const QPointF &globalPosF)
    \internal

    Creates the proper Enter/Leave event when widget \a enter is entered and
    widget \a leave is left.
 */
void QApplicationPrivate::dispatchEnterLeave(QWidget *enter, QWidget *leave, const QPointF &globalPosF)
{
#if 0
   if (leave) {
      QEvent e(QEvent::Leave);
      QApplication::sendEvent(leave, & e);
   }
   if (enter) {
      const QPoint windowPos = enter->window()->mapFromGlobal(globalPos);
      QEnterEvent e(enter->mapFromGlobal(globalPos), windowPos, globalPos);
      QApplication::sendEvent(enter, & e);
   }
   return;
#endif

   QWidget *w ;
   if ((!enter && !leave) || (enter == leave)) {
      return;
   }
#ifdef ALIEN_DEBUG
   qDebug() << "QApplicationPrivate::dispatchEnterLeave, ENTER:" << enter << "LEAVE:" << leave;
#endif

   QWidgetList leaveList;
   QWidgetList enterList;

   bool sameWindow = leave && enter && leave->window() == enter->window();
   if (leave && !sameWindow) {
      w = leave;
      do {
         leaveList.append(w);
      } while (!w->isWindow() && (w = w->parentWidget()));
   }
   if (enter && !sameWindow) {
      w = enter;
      do {
         enterList.prepend(w);
      } while (!w->isWindow() && (w = w->parentWidget()));
   }
   if (sameWindow) {
      int enterDepth = 0;
      int leaveDepth = 0;
      w = enter;
      while (!w->isWindow() && (w = w->parentWidget())) {
         enterDepth++;
      }
      w = leave;
      while (!w->isWindow() && (w = w->parentWidget())) {
         leaveDepth++;
      }
      QWidget *wenter = enter;
      QWidget *wleave = leave;
      while (enterDepth > leaveDepth) {
         wenter = wenter->parentWidget();
         enterDepth--;
      }
      while (leaveDepth > enterDepth) {
         wleave = wleave->parentWidget();
         leaveDepth--;
      }
      while (!wenter->isWindow() && wenter != wleave) {
         wenter = wenter->parentWidget();
         wleave = wleave->parentWidget();
      }

      w = leave;
      while (w != wleave) {
         leaveList.append(w);
         w = w->parentWidget();
      }
      w = enter;
      while (w != wenter) {
         enterList.prepend(w);
         w = w->parentWidget();
      }
   }

   QEvent leaveEvent(QEvent::Leave);
   for (int i = 0; i < leaveList.size(); ++i) {
      w = leaveList.at(i);
      if (!QApplication::activeModalWidget() || QApplicationPrivate::tryModalHelper(w, 0)) {
         QApplication::sendEvent(w, &leaveEvent);
         if (w->testAttribute(Qt::WA_Hover) &&
            (!QApplication::activePopupWidget() || QApplication::activePopupWidget() == w->window())) {
            Q_ASSERT(instance());
            QHoverEvent he(QEvent::HoverLeave, QPoint(-1, -1), w->mapFromGlobal(QApplicationPrivate::instance()->hoverGlobalPos),
               QApplication::keyboardModifiers());
            qApp->d_func()->notify_helper(w, &he);
         }
      }
   }

   if (!enterList.isEmpty()) {
      // Guard against QGuiApplicationPrivate::lastCursorPosition initialized to qInf(), qInf().
      const QPoint globalPos = std::isinf(globalPosF.x())
         ? QPoint(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)
         : globalPosF.toPoint();

      const QPoint windowPos = enterList.front()->window()->mapFromGlobal(globalPos);
      for (int i = 0; i < enterList.size(); ++i) {
         w = enterList.at(i);
         if (!QApplication::activeModalWidget() || QApplicationPrivate::tryModalHelper(w, 0)) {
            const QPointF localPos = w->mapFromGlobal(globalPos);
            QEnterEvent enterEvent(localPos, windowPos, globalPosF);
            QApplication::sendEvent(w, &enterEvent);
            if (w->testAttribute(Qt::WA_Hover) &&
               (!QApplication::activePopupWidget() || QApplication::activePopupWidget() == w->window())) {
               QHoverEvent he(QEvent::HoverEnter, localPos, QPoint(-1, -1),
                  QApplication::keyboardModifiers());
               qApp->d_func()->notify_helper(w, &he);
            }
         }
      }
   }

#ifndef QT_NO_CURSOR
   // Update cursor for alien/graphics widgets.

   const bool enterOnAlien = (enter && (isAlien(enter) || enter->testAttribute(Qt::WA_DontShowOnScreen)));
   // Whenever we leave an alien widget on X11/QPA, we need to reset its nativeParentWidget()'s cursor.
   // This is not required on Windows as the cursor is reset on every single mouse move.
   QWidget *parentOfLeavingCursor = 0;
   for (int i = 0; i < leaveList.size(); ++i) {
      w = leaveList.at(i);
      if (!isAlien(w)) {
         break;
      }
      if (w->testAttribute(Qt::WA_SetCursor)) {
         QWidget *parent = w->parentWidget();
         while (parent && parent->d_func()->data.in_destructor) {
            parent = parent->parentWidget();
         }
         parentOfLeavingCursor = parent;
         //continue looping, we need to find the downest alien widget with a cursor.
         // (downest on the screen)
      }
   }
   //check that we will not call qt_x11_enforce_cursor twice with the same native widget
   if (parentOfLeavingCursor && (!enterOnAlien
         || parentOfLeavingCursor->effectiveWinId() != enter->effectiveWinId())) {
#ifndef QT_NO_GRAPHICSVIEW
      if (!parentOfLeavingCursor->window()->graphicsProxyWidget())
#endif
      {
         if (enter == QApplication::desktop()) {
            qt_qpa_set_cursor(enter, true);
         } else {
            qt_qpa_set_cursor(parentOfLeavingCursor, true);
         }
      }
   }
   if (enterOnAlien) {
      QWidget *cursorWidget = enter;
      while (!cursorWidget->isWindow() && !cursorWidget->isEnabled()) {
         cursorWidget = cursorWidget->parentWidget();
      }

      if (!cursorWidget) {
         return;
      }

#ifndef QT_NO_GRAPHICSVIEW
      if (cursorWidget->window()->graphicsProxyWidget()) {
         QWidgetPrivate::nearestGraphicsProxyWidget(cursorWidget)->setCursor(cursorWidget->cursor());
      } else
#endif
      {
         qt_qpa_set_cursor(cursorWidget, true);
      }
   }
#endif
}

/* exported for the benefit of testing tools */
Q_GUI_EXPORT bool qt_tryModalHelper(QWidget *widget, QWidget **rettop)
{
   return QApplicationPrivate::tryModalHelper(widget, rettop);
}

/*! \internal
    Returns \c true if \a widget is blocked by a modal window.
 */
bool QApplicationPrivate::isBlockedByModal(QWidget *widget)
{
   widget = widget->window();
   QWindow *window = widget->windowHandle();
   return window && self->isWindowBlocked(window);
}

bool QApplicationPrivate::isWindowBlocked(QWindow *window, QWindow **blockingWindow) const
{
   QWindow *unused = 0;
   if (!window) {
      qWarning().nospace() << "window == 0 passed.";
      return false;
   }

   if (! blockingWindow) {
      blockingWindow = &unused;
   }

   if (modalWindowList.isEmpty()) {
      *blockingWindow = 0;
      return false;
   }
   QWidget *popupWidget = QApplication::activePopupWidget();
   QWindow *popupWindow = popupWidget ? popupWidget->windowHandle() : 0;
   if (popupWindow == window || (!popupWindow && QWindowPrivate::get(window)->isPopup())) {
      *blockingWindow = 0;
      return false;
   }

   for (int i = 0; i < modalWindowList.count(); ++i) {
      QWindow *modalWindow = modalWindowList.at(i);

      {
         // check if the modal window is our window or a (transient) parent of our window
         QWindow *w = window;
         while (w) {
            if (w == modalWindow) {
               *blockingWindow = 0;
               return false;
            }
            QWindow *p = w->parent();
            if (!p) {
               p = w->transientParent();
            }
            w = p;
         }

         // Embedded in-process windows are not visible in normal parent-child chain,
         // so check the native parent chain, too.
         const QPlatformWindow *platWin = window->handle();
         const QPlatformWindow *modalPlatWin = modalWindow->handle();
         if (platWin && modalPlatWin && platWin->isEmbedded(modalPlatWin)) {
            return false;
         }
      }

      Qt::WindowModality windowModality = modalWindow->modality();
      QWidgetWindow *modalWidgetWindow = qobject_cast<QWidgetWindow *>(modalWindow);
      if (windowModality == Qt::NonModal) {
         // determine the modality type if it hasn't been set on the
         // modalWindow's widget, this normally happens when waiting for a
         // native dialog. use WindowModal if we are the child of a group
         // leader; otherwise use ApplicationModal.
         QWidget *m = modalWidgetWindow ? modalWidgetWindow->widget() : 0;
         while (m && !m->testAttribute(Qt::WA_GroupLeader)) {
            m = m->parentWidget();
            if (m) {
               m = m->window();
            }
         }
         windowModality = (m && m->testAttribute(Qt::WA_GroupLeader))
            ? Qt::WindowModal
            : Qt::ApplicationModal;
      }

      switch (windowModality) {
         case Qt::ApplicationModal: {
            QWidgetWindow *widgetWindow = qobject_cast<QWidgetWindow *>(window);
            QWidget *groupLeaderForWidget = widgetWindow ? widgetWindow->widget() : 0;
            while (groupLeaderForWidget && !groupLeaderForWidget->testAttribute(Qt::WA_GroupLeader)) {
               groupLeaderForWidget = groupLeaderForWidget->parentWidget();
            }

            if (groupLeaderForWidget) {
               // if \a widget has WA_GroupLeader, it can only be blocked by ApplicationModal children
               QWidget *m = modalWidgetWindow ? modalWidgetWindow->widget() : 0;
               while (m && m != groupLeaderForWidget && !m->testAttribute(Qt::WA_GroupLeader)) {
                  m = m->parentWidget();
               }
               if (m == groupLeaderForWidget) {
                  *blockingWindow = m->windowHandle();
                  return true;
               }
            } else if (modalWindow != window) {
               *blockingWindow = modalWindow;
               return true;
            }
            break;
         }
         case Qt::WindowModal: {
            QWindow *w = window;
            do {
               QWindow *m = modalWindow;
               do {
                  if (m == w) {
                     *blockingWindow = m;
                     return true;
                  }
                  QWindow *p = m->parent();
                  if (!p) {
                     p = m->transientParent();
                  }
                  m = p;
               } while (m);
               QWindow *p = w->parent();
               if (!p) {
                  p = w->transientParent();
               }
               w = p;
            } while (w);
            break;
         }
         default:
            Q_ASSERT_X(false, "QApplication", "internal error, a modal window cannot be modeless");
            break;
      }
   }
   *blockingWindow = 0;
   return false;
}

/*!\internal

  Called from qapplication_\e{platform}.cpp, returns \c true
  if the widget should accept the event.
 */
bool QApplicationPrivate::tryModalHelper(QWidget *widget, QWidget **rettop)
{
   QWidget *top = QApplication::activeModalWidget();
   if (rettop) {
      *rettop = top;
   }

   // the active popup widget always gets the input event
   if (QApplication::activePopupWidget()) {
      return true;
   }

   return !isBlockedByModal(widget->window());
}

bool qt_try_modal(QWidget *widget, QEvent::Type type)
{
   QWidget *top = 0;

   if (QApplicationPrivate::tryModalHelper(widget, &top)) {
      return true;
   }

   bool block_event  = false;

   switch (type) {
#if 0
      case QEvent::Focus:
         if (!static_cast<QWSFocusEvent *>(event)->simpleData.get_focus) {
            break;
         }
         // drop through
#endif
      case QEvent::MouseButtonPress:                        // disallow mouse/key events
      case QEvent::MouseButtonRelease:
      case QEvent::MouseMove:
      case QEvent::KeyPress:
      case QEvent::KeyRelease:
         block_event         = true;
         break;
      default:
         break;
   }

   if (block_event && top && top->parentWidget() == 0) {
      top->raise();
   }

   return !block_event;
}

bool QApplicationPrivate::modalState()
{
   return !self->modalWindowList.isEmpty();
}

/*
   \internal
*/
QWidget *QApplicationPrivate::pickMouseReceiver(QWidget *candidate, const QPoint &windowPos,
   QPoint *pos, QEvent::Type type,
   Qt::MouseButtons buttons, QWidget *buttonDown,
   QWidget *alienWidget)
{
   Q_ASSERT(candidate);

   QWidget *mouseGrabber = QWidget::mouseGrabber();
   if (((type == QEvent::MouseMove && buttons) || (type == QEvent::MouseButtonRelease))
      && !buttonDown && !mouseGrabber) {
      return 0;
   }

   if (alienWidget && alienWidget->internalWinId()) {
      alienWidget = 0;
   }

   QWidget *receiver = candidate;

   if (!mouseGrabber) {
      mouseGrabber = (buttonDown && !isBlockedByModal(buttonDown)) ? buttonDown : alienWidget;
   }

   if (mouseGrabber && mouseGrabber != candidate) {
      receiver = mouseGrabber;
      *pos = receiver->mapFromGlobal(candidate->mapToGlobal(windowPos));
#ifdef ALIEN_DEBUG
      qDebug() << "  ** receiver adjusted to:" << receiver << "pos:" << pos;
#endif
   }

   return receiver;

}

/*
   \internal
*/
bool QApplicationPrivate::sendMouseEvent(QWidget *receiver, QMouseEvent *event,
   QWidget *alienWidget, QWidget *nativeWidget,
   QWidget **buttonDown, QPointer<QWidget> &lastMouseReceiver,
   bool spontaneous)
{
   Q_ASSERT(receiver);
   Q_ASSERT(event);
   Q_ASSERT(nativeWidget);
   Q_ASSERT(buttonDown);

   if (alienWidget && !isAlien(alienWidget)) {
      alienWidget = 0;
   }

   QPointer<QWidget> receiverGuard = receiver;
   QPointer<QWidget> nativeGuard = nativeWidget;
   QPointer<QWidget> alienGuard = alienWidget;
   QPointer<QWidget> activePopupWidget = QApplication::activePopupWidget();

   const bool graphicsWidget = nativeWidget->testAttribute(Qt::WA_DontShowOnScreen);

   bool widgetUnderMouse = QRectF(receiver->rect()).contains(event->localPos());

   // Clear the obsolete leaveAfterRelease value, if mouse button has been released but
   // leaveAfterRelease has not been updated.
   // This happens e.g. when modal dialog or popup is shown as a response to button click.
   if (leaveAfterRelease && !*buttonDown && !event->buttons()) {
      leaveAfterRelease = 0;
   }

   if (*buttonDown) {
      if (!graphicsWidget) {
         // Register the widget that shall receive a leave event
         // after the last button is released.
         if ((alienWidget || !receiver->internalWinId()) && !leaveAfterRelease && !QWidget::mouseGrabber()) {
            leaveAfterRelease = *buttonDown;
         }
         if (event->type() == QEvent::MouseButtonRelease && !event->buttons()) {
            *buttonDown = 0;
         }
      }
   } else if (lastMouseReceiver && widgetUnderMouse) {
      // Dispatch enter/leave if we move:
      // 1) from an alien widget to another alien widget or
      //    from a native widget to an alien widget (first OR case)
      // 2) from an alien widget to a native widget (second OR case)
      if ((alienWidget && alienWidget != lastMouseReceiver)
         || (isAlien(lastMouseReceiver) && !alienWidget)) {
         if (activePopupWidget) {
            if (!QWidget::mouseGrabber()) {
               dispatchEnterLeave(alienWidget ? alienWidget : nativeWidget, lastMouseReceiver, event->screenPos());
            }
         } else {
            dispatchEnterLeave(receiver, lastMouseReceiver, event->screenPos());
         }

      }
   }

#ifdef ALIEN_DEBUG
   qDebug() << "QApplicationPrivate::sendMouseEvent: receiver:" << receiver
      << "pos:" << event->pos() << "alien" << alienWidget << "button down"
      << *buttonDown << "last" << lastMouseReceiver << "leave after release"
      << leaveAfterRelease;
#endif

   // We need this quard in case someone opens a modal dialog / popup. If that's the case
   // leaveAfterRelease is set to null, but we shall not update lastMouseReceiver.
   const bool wasLeaveAfterRelease = leaveAfterRelease != 0;
   bool result;
   if (spontaneous) {
      result = QApplication::sendSpontaneousEvent(receiver, event);
   } else {
      result = QApplication::sendEvent(receiver, event);
   }

   if (!graphicsWidget && leaveAfterRelease && event->type() == QEvent::MouseButtonRelease
      && !event->buttons() && QWidget::mouseGrabber() != leaveAfterRelease) {
      // Dispatch enter/leave if:
      // 1) the mouse grabber is an alien widget
      // 2) the button is released on an alien widget
      QWidget *enter = 0;
      if (nativeGuard) {
         enter = alienGuard ? alienWidget : nativeWidget;
      } else { // The receiver is typically deleted on mouse release with drag'n'drop.
         enter = QApplication::widgetAt(event->globalPos());
      }
      dispatchEnterLeave(enter, leaveAfterRelease, event->screenPos());
      leaveAfterRelease = 0;
      lastMouseReceiver = enter;
   } else if (!wasLeaveAfterRelease) {
      if (activePopupWidget) {
         if (!QWidget::mouseGrabber()) {
            lastMouseReceiver = alienGuard ? alienWidget : (nativeGuard ? nativeWidget : 0);
         }
      } else {
         lastMouseReceiver = receiverGuard ? receiver : QApplication::widgetAt(event->globalPos());
      }
   }

   return result;
}

/*
    This function should only be called when the widget changes visibility, i.e.
    when the \a widget is shown, hidden or deleted. This function does nothing
    if the widget is a top-level or native, i.e. not an alien widget. In that
    case enter/leave events are genereated by the underlying windowing system.
*/
extern QPointer<QWidget> qt_last_mouse_receiver;
extern QWidget *qt_button_down;

void QApplicationPrivate::sendSyntheticEnterLeave(QWidget *widget)
{
#ifndef QT_NO_CURSOR
   if (!widget || widget->isWindow()) {
      return;
   }
   const bool widgetInShow = widget->isVisible() && !widget->data->in_destructor;
   if (!widgetInShow && widget != qt_last_mouse_receiver) {
      return;   // Widget was not under the cursor when it was hidden/deleted.
   }

   if (widgetInShow && widget->parentWidget()->data->in_show) {
      return;   // Ingore recursive show.
   }

   QWidget *mouseGrabber = QWidget::mouseGrabber();
   if (mouseGrabber && mouseGrabber != widget) {
      return;   // Someone else has the grab; enter/leave should not occur.
   }

   QWidget *tlw = widget->window();
   if (tlw->data->in_destructor || tlw->data->is_closing) {
      return;   // Closing down the business.
   }

   if (widgetInShow && (!qt_last_mouse_receiver || qt_last_mouse_receiver->window() != tlw)) {
      return;   // Mouse cursor not inside the widget's top-level.
   }

   const QPoint globalPos(QCursor::pos());
   QPoint windowPos = tlw->mapFromGlobal(globalPos);

   // Find the current widget under the mouse. If this function was called from
   // the widget's destructor, we have to make sure childAt() doesn't take into
   // account widgets that are about to be destructed.
   QWidget *widgetUnderCursor = tlw->d_func()->childAt_helper(windowPos, widget->data->in_destructor);
   if (!widgetUnderCursor) {
      widgetUnderCursor = tlw;
   }
   QPoint pos = widgetUnderCursor->mapFrom(tlw, windowPos);

   if (widgetInShow && widgetUnderCursor != widget && !widget->isAncestorOf(widgetUnderCursor)) {
      return;   // Mouse cursor not inside the widget or any of its children.
   }

   if (widget->data->in_destructor && qt_button_down == widget) {
      qt_button_down = 0;
   }

   // Send enter/leave events followed by a mouse move on the entered widget.
   QMouseEvent e(QEvent::MouseMove, pos, windowPos, globalPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
   sendMouseEvent(widgetUnderCursor, &e, widgetUnderCursor, tlw, &qt_button_down, qt_last_mouse_receiver);
#endif // QT_NO_CURSOR
}



void QApplication::setStartDragTime(int ms)
{
   QGuiApplication::styleHints()->setStartDragTime(ms);
}

int QApplication::startDragTime()
{
   return QGuiApplication::styleHints()->startDragTime();
}

/*
    Sets the distance after which a drag should start to \a l pixels.

    \sa startDragDistance()
*/

void QApplication::setStartDragDistance(int l)
{
   QGuiApplication::styleHints()->setStartDragDistance(l);
}

/*!
    \property QApplication::startDragDistance

    If you support drag and drop in your application, and want to start a drag
    and drop operation after the user has moved the cursor a certain distance
    with a button held down, you should use this property's value as the
    minimum distance required.

    For example, if the mouse position of the click is stored in \c startPos
    and the current position (e.g. in the mouse move event) is \c currentPos,
    you can find out if a drag should be started with code like this:

    \snippet code/src_gui_kernel_qapplication.cpp 6

    Qt uses this value internally, e.g. in QFileDialog.

    The default value (if the platform doesn't provide a different default)
    is 10 pixels.

    \sa startDragTime(), QPoint::manhattanLength(), {Drag and Drop}
*/

int QApplication::startDragDistance()
{
   return QGuiApplication::styleHints()->startDragDistance();
}


bool QApplicationPrivate::shouldQuit()
{
   /* if there is no non-withdrawn primary window left (except
       the ones without QuitOnClose), we emit the lastWindowClosed
       signal */

   QWidgetList list = QApplication::topLevelWidgets();
   QWindowList processedWindows;

   for (int i = 0; i < list.size(); ++i) {
      QWidget *w = list.at(i);

      if (QWindow *window = w->windowHandle()) { // Menus, popup widgets may not have a QWindow
         processedWindows.push_back(window);
         if (w->isVisible() && !w->parentWidget() && w->testAttribute(Qt::WA_QuitOnClose)) {
            return false;
         }
      }
   }

   return QGuiApplicationPrivate::shouldQuitInternal(processedWindows);
}

static inline void closeAllPopups()
{
   // Close all popups: In case some popup refuses to close,
   // we give up after 1024 attempts (to avoid an infinite loop).
   int maxiter = 1024;
   QWidget *popup;
   while ((popup = QApplication::activePopupWidget()) && maxiter--) {
      popup->close();
   }
}


bool QApplication::notify(QObject *receiver, QEvent *e)
{
   Q_D(QApplication);

   // no events are delivered after ~QCoreApplication() has started
   if (QApplicationPrivate::is_app_closing) {
      return true;
   }

   if (receiver == nullptr) {
      // serious error
      qWarning("QApplication::notify: Unexpected null receiver");
      return true;
   }

#ifndef QT_NO_DEBUG
   d->checkReceiverThread(receiver);
#endif

   if (receiver->isWindowType()) {
      QGuiApplicationPrivate::sendQWindowEventToQPlatformWindow(static_cast<QWindow *>(receiver), e);
   }

   if (e->spontaneous()) {
      // Capture the current mouse and keyboard states. Doing so here is
      // required in order to support Qt Test synthesized events. Real mouse
      // and keyboard state updates from the platform plugin are managed by
      // QGuiApplicationPrivate::process(Mouse|Wheel|Key|Touch|Tablet)Event();
      switch (e->type()) {
         case QEvent::MouseButtonPress: {
            QMouseEvent *me = static_cast<QMouseEvent *>(e);
            QApplicationPrivate::modifier_buttons = me->modifiers();
            QApplicationPrivate::mouse_buttons |= me->button();
            break;
         }
         case QEvent::MouseButtonDblClick: {
            QMouseEvent *me = static_cast<QMouseEvent *>(e);
            QApplicationPrivate::modifier_buttons = me->modifiers();
            QApplicationPrivate::mouse_buttons |= me->button();
            break;
         }
         case QEvent::MouseButtonRelease: {
            QMouseEvent *me = static_cast<QMouseEvent *>(e);
            QApplicationPrivate::modifier_buttons = me->modifiers();
            QApplicationPrivate::mouse_buttons &= ~me->button();
            break;
         }
         case QEvent::KeyPress:
         case QEvent::KeyRelease:
         case QEvent::MouseMove:

#ifndef QT_NO_WHEELEVENT
         case QEvent::Wheel:
#endif
         case QEvent::TouchBegin:
         case QEvent::TouchUpdate:
         case QEvent::TouchEnd:

#ifndef QT_NO_TABLETEVENT
         case QEvent::TabletMove:
         case QEvent::TabletPress:
         case QEvent::TabletRelease:
#endif
         {
            QInputEvent *ie = static_cast<QInputEvent *>(e);
            QApplicationPrivate::modifier_buttons = ie->modifiers();
            break;
         }
         default:
            break;
      }
   }

#ifndef QT_NO_GESTURES
   // walk through parents and check for gestures
   if (d->gestureManager) {
      switch (e->type()) {
         case QEvent::Paint:
         case QEvent::MetaCall:
         case QEvent::DeferredDelete:
         case QEvent::DragEnter:
         case QEvent::DragMove:
         case QEvent::DragLeave:
         case QEvent::Drop:
         case QEvent::DragResponse:
         case QEvent::ChildAdded:
         case QEvent::ChildPolished:
         case QEvent::ChildRemoved:
         case QEvent::UpdateRequest:
         case QEvent::UpdateLater:
         case QEvent::LocaleChange:
         case QEvent::Style:
         case QEvent::IconDrag:
         case QEvent::StyleChange:
         case QEvent::GraphicsSceneDragEnter:
         case QEvent::GraphicsSceneDragMove:
         case QEvent::GraphicsSceneDragLeave:
         case QEvent::GraphicsSceneDrop:
         case QEvent::DynamicPropertyChange:
         case QEvent::NetworkReplyUpdated:
            break;

         default:
            if (d->gestureManager->thread() == QThread::currentThread()) {
               if (receiver->isWidgetType()) {
                  if (d->gestureManager->filterEvent(static_cast<QWidget *>(receiver), e)) {
                     return true;
                  }
               } else {
                  // a special case for events that go to QGesture objects.
                  // We pass the object to the gesture manager and it'll figure
                  // out if it's QGesture or not.
                  if (d->gestureManager->filterEvent(receiver, e)) {
                     return true;
                  }
               }
            }
            break;
      }
   }
#endif // QT_NO_GESTURES

   switch (e->type()) {
      case QEvent::ApplicationDeactivate:
         // Close all popups (triggers when switching applications
         // by pressing ALT-TAB on Windows, which is not receive as key event.
         closeAllPopups();
         break;
      case QEvent::Wheel: // User input and window activation makes tooltips sleep
      case QEvent::ActivationChange:
      case QEvent::KeyPress:
      case QEvent::KeyRelease:
      case QEvent::FocusOut:
      case QEvent::FocusIn:
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
         d->toolTipFallAsleep.stop();
      // fall-through
      case QEvent::Leave:
         d->toolTipWakeUp.stop();
      default:
         break;
   }

   switch (e->type()) {
      case QEvent::KeyPress: {
         int key = static_cast<QKeyEvent *>(e)->key();
         qt_in_tab_key_event = (key == Qt::Key_Backtab
               || key == Qt::Key_Tab
               || key == Qt::Key_Left
               || key == Qt::Key_Up
               || key == Qt::Key_Right
               || key == Qt::Key_Down);
      }
      default:
         break;
   }

   bool res = false;

   if (!receiver->isWidgetType()) {
      res = d->notify_helper(receiver, e);
   } else
      switch (e->type()) {
         case QEvent::ShortcutOverride:
         case QEvent::KeyPress:
         case QEvent::KeyRelease: {
            bool isWidget = receiver->isWidgetType();
#ifndef QT_NO_GRAPHICSVIEW
            const bool isGraphicsWidget = !isWidget && qobject_cast<QGraphicsWidget *>(receiver);
#endif
            QKeyEvent *key = static_cast<QKeyEvent *>(e);
            bool def = key->isAccepted();
            QPointer<QObject> pr = receiver;
            while (receiver) {
               if (def) {
                  key->accept();
               } else {
                  key->ignore();
               }
               QWidget *w = isWidget ? static_cast<QWidget *>(receiver) : 0;
#ifndef QT_NO_GRAPHICSVIEW
               QGraphicsWidget *gw = isGraphicsWidget ? static_cast<QGraphicsWidget *>(receiver) : 0;
#endif
               res = d->notify_helper(receiver, e);

               if ((res && key->isAccepted())
                  /*
                     QLineEdit will emit a signal on Key_Return, but
                     ignore the event, and sometimes the connected
                     slot deletes the QLineEdit (common in itemview
                     delegates), so we have to check if the widget
                     was destroyed even if the event was ignored (to
                     prevent a crash)

                     note that we don't have to reset pw while
                     propagating (because the original receiver will
                     be destroyed if one of its ancestors is)
                  */
                  || !pr
                  || (isWidget && (w->isWindow() || !w->parentWidget()))
#ifndef QT_NO_GRAPHICSVIEW
                  || (isGraphicsWidget && (gw->isWindow() || !gw->parentWidget()))
#endif
               ) {
                  break;
               }

#ifndef QT_NO_GRAPHICSVIEW
               receiver = w ? (QObject *)w->parentWidget() : (QObject *)gw->parentWidget();
#else
               receiver = w->parentWidget();
#endif
            }
            qt_in_tab_key_event = false;
         }
         break;
         case QEvent::MouseButtonPress:
         case QEvent::MouseButtonRelease:
         case QEvent::MouseButtonDblClick:
         case QEvent::MouseMove: {
            QWidget *w = static_cast<QWidget *>(receiver);

            QMouseEvent *mouse = static_cast<QMouseEvent *>(e);
            QPoint relpos = mouse->pos();

            if (e->spontaneous()) {
               if (e->type() != QEvent::MouseMove) {
                  QApplicationPrivate::giveFocusAccordingToFocusPolicy(w, e, relpos);
               }

               // ### Qt 5 These dynamic tool tips should be an OPT-IN feature. Some platforms
               // like OS X (probably others too), can optimize their views by not
               // dispatching mouse move events. We have attributes to control hover,
               // and mouse tracking, but as long as we are deciding to implement this
               // feature without choice of opting-in or out, you ALWAYS have to have
               // tracking enabled. Therefore, the other properties give a false sense of
               // performance enhancement.

               if (e->type() == QEvent::MouseMove && mouse->buttons() == 0
                  && w->rect().contains(relpos)) { // Outside due to mouse grab?
                  d->toolTipWidget = w;
                  d->toolTipPos = relpos;
                  d->toolTipGlobalPos = mouse->globalPos();
                  QStyle *s = d->toolTipWidget->style();
                  int wakeDelay = s->styleHint(QStyle::SH_ToolTip_WakeUpDelay, 0, d->toolTipWidget, 0);
                  d->toolTipWakeUp.start(d->toolTipFallAsleep.isActive() ? 20 : wakeDelay, this);
               }
            }

            bool eventAccepted = mouse->isAccepted();
            QPointer<QWidget> pw = w;

            while (w) {
               QMouseEvent me(mouse->type(), relpos, mouse->windowPos(), mouse->globalPos(),
                  mouse->button(), mouse->buttons(), mouse->modifiers(), mouse->source());

               me.spont = mouse->spontaneous();
               me.setTimestamp(mouse->timestamp());
               QGuiApplicationPrivate::setMouseEventFlags(&me, mouse->flags());

               // throw away any mouse-tracking-only mouse events
               if (! w->hasMouseTracking() && mouse->type() == QEvent::MouseMove && mouse->buttons() == 0) {

                  // still send them through all application event filters (normally done by notify_helper)
                  auto &eventFilters = CSInternalEvents::get_m_EventFilters(this);

                  for (int i = 0; i < eventFilters.size(); ++i) {
                     QObject *obj = eventFilters.at(i);

                     if (! obj) {
                        continue;
                     }

                     if (CSInternalThreadData::get_m_ThreadData(obj) != CSInternalThreadData::get_m_ThreadData(w)) {
                        qWarning("QApplication: Object event filter cannot be in a different thread.");
                        continue;
                     }

                     if (obj->eventFilter(w, w == receiver ? mouse : &me)) {
                        break;
                     }
                  }

                  res = true;

               } else {
                  w->setAttribute(Qt::WA_NoMouseReplay, false);
                  res = d->notify_helper(w, w == receiver ? mouse : &me);
                  e->spont = false;
               }

               eventAccepted = (w == receiver ? mouse : &me)->isAccepted();

               if (res && eventAccepted) {
                  break;
               }
               if (w->isWindow() || w->testAttribute(Qt::WA_NoMousePropagation)) {
                  break;
               }
               relpos += w->pos();
               w = w->parentWidget();
            }

            mouse->setAccepted(eventAccepted);

            if (e->type() == QEvent::MouseMove) {
               if (!pw) {
                  break;
               }

               w = static_cast<QWidget *>(receiver);
               relpos = mouse->pos();
               QPoint diff = relpos - w->mapFromGlobal(d->hoverGlobalPos);
               while (w) {
                  if (w->testAttribute(Qt::WA_Hover) &&
                     (!QApplication::activePopupWidget() || QApplication::activePopupWidget() == w->window())) {
                     QHoverEvent he(QEvent::HoverMove, relpos, relpos - diff, mouse->modifiers());
                     d->notify_helper(w, &he);
                  }
                  if (w->isWindow() || w->testAttribute(Qt::WA_NoMousePropagation)) {
                     break;
                  }
                  relpos += w->pos();
                  w = w->parentWidget();
               }
            }

            d->hoverGlobalPos = mouse->globalPos();
         }
         break;
#ifndef QT_NO_WHEELEVENT
         case QEvent::Wheel: {
            QWidget *w = static_cast<QWidget *>(receiver);

            // QTBUG-40656, QTBUG-42731: ignore wheel events when a popup (QComboBox) is open.
            if (const QWidget *popup = QApplication::activePopupWidget()) {
               if (w->window() != popup) {
                  return true;
               }
            }

            QWheelEvent *wheel = static_cast<QWheelEvent *>(e);
            const bool spontaneous = wheel->spontaneous();
            const Qt::ScrollPhase phase = wheel->phase();

            // Ideally, we should lock on a widget when it starts receiving wheel
            // events. This avoids other widgets to start receiving those events
            // as the mouse cursor hovers them. However, given the way common
            // wheeled mice work, there's no certain way of connecting different
            // wheel events as a stream. This results in the NoScrollPhase case,
            // where we just send the event from the original receiver and up its
            // hierarchy until the event gets accepted.
            //
            // In the case of more evolved input devices, like Apple's trackpad or
            // Magic Mouse, we receive the scroll phase information. This helps us
            // connect wheel events as a stream and therefore makes it easier to
            // lock on the widget onto which the scrolling was initiated.
            //
            // We assume that, when supported, the phase cycle follows the pattern:
            //
            //         ScrollBegin (ScrollUpdate* ScrollEnd)+
            //
            // This means that we can have scrolling sequences (starting with ScrollBegin)
            // or partial sequences (after a ScrollEnd and starting with ScrollUpdate).
            // If wheel_widget is null because it was deleted, we also take the same
            // code path as an initial sequence.
            if (phase == Qt::NoScrollPhase || phase == Qt::ScrollBegin || !QApplicationPrivate::wheel_widget) {

               // A system-generated ScrollBegin event starts a new user scrolling
               // sequence, so we reset wheel_widget in case no one accepts the event
               // or if we didn't get (or missed) a ScrollEnd previously.
               if (spontaneous && phase == Qt::ScrollBegin) {
                  QApplicationPrivate::wheel_widget = nullptr;
               }

               const QPoint &relpos = wheel->pos();

               if (spontaneous && (phase == Qt::NoScrollPhase || phase == Qt::ScrollUpdate)) {
                  QApplicationPrivate::giveFocusAccordingToFocusPolicy(w, e, relpos);
               }

               QWheelEvent we(relpos, wheel->globalPos(), wheel->pixelDelta(), wheel->angleDelta(), wheel->delta(), wheel->orientation(),
                  wheel->buttons(),
                  wheel->modifiers(), phase, wheel->source());
               bool eventAccepted;
               while (w) {
                  we.spont = spontaneous && w == receiver;
                  we.ignore();
                  res = d->notify_helper(w, &we);
                  eventAccepted = we.isAccepted();
                  if (res && eventAccepted) {
                     // A new scrolling sequence or partial sequence starts and w has accepted
                     // the event. Therefore, we can set wheel_widget, but only if it's not
                     // the end of a sequence.
                     if (spontaneous && (phase == Qt::ScrollBegin || phase == Qt::ScrollUpdate) && QGuiApplicationPrivate::scrollNoPhaseAllowed) {
                        QApplicationPrivate::wheel_widget = w;
                     }
                     break;
                  }
                  if (w->isWindow() || w->testAttribute(Qt::WA_NoMousePropagation)) {
                     break;
                  }

                  we.p += w->pos();
                  w = w->parentWidget();
               }
               wheel->setAccepted(eventAccepted);
            } else if (!spontaneous) {
               // wheel_widget may forward the wheel event to a delegate widget,
               // either directly or indirectly (e.g. QAbstractScrollArea will
               // forward to its QScrollBars through viewportEvent()). In that
               // case, the event will not be spontaneous but synthesized, so
               // we can send it straight to the receiver.
               d->notify_helper(w, wheel);
            } else {
               // The phase is either ScrollUpdate or ScrollEnd, and wheel_widget
               // is set. Since it accepted the wheel event previously, we continue
               // sending those events until we get a ScrollEnd, which signifies
               // the end of the natural scrolling sequence.
               const QPoint &relpos = QApplicationPrivate::wheel_widget->mapFromGlobal(wheel->globalPos());
               QWheelEvent we(relpos, wheel->globalPos(), wheel->pixelDelta(), wheel->angleDelta(), wheel->delta(), wheel->orientation(),
                  wheel->buttons(),
                  wheel->modifiers(), wheel->phase(), wheel->source());
               we.spont = true;
               we.ignore();
               d->notify_helper(QApplicationPrivate::wheel_widget, &we);
               wheel->setAccepted(we.isAccepted());
               if (phase == Qt::ScrollEnd) {
                  QApplicationPrivate::wheel_widget = nullptr;
               }
            }
         }
         break;
#endif
#ifndef QT_NO_CONTEXTMENU
         case QEvent::ContextMenu: {
            QWidget *w = static_cast<QWidget *>(receiver);
            QContextMenuEvent *context = static_cast<QContextMenuEvent *>(e);
            QPoint relpos = context->pos();
            bool eventAccepted = context->isAccepted();
            while (w) {
               QContextMenuEvent ce(context->reason(), relpos, context->globalPos(), context->modifiers());
               ce.spont = e->spontaneous();
               res = d->notify_helper(w, w == receiver ? context : &ce);
               eventAccepted = ((w == receiver) ? context : &ce)->isAccepted();
               e->spont = false;

               if ((res && eventAccepted)
                  || w->isWindow() || w->testAttribute(Qt::WA_NoMousePropagation)) {
                  break;
               }

               relpos += w->pos();
               w = w->parentWidget();
            }
            context->setAccepted(eventAccepted);
         }
         break;
#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_TABLETEVENT
         case QEvent::TabletMove:
         case QEvent::TabletPress:
         case QEvent::TabletRelease: {
            QWidget *w = static_cast<QWidget *>(receiver);
            QTabletEvent *tablet = static_cast<QTabletEvent *>(e);
            QPointF relpos = tablet->posF();
            bool eventAccepted = tablet->isAccepted();
            while (w) {
               QTabletEvent te(tablet->type(), relpos, tablet->globalPosF(),
                  tablet->device(), tablet->pointerType(),
                  tablet->pressure(), tablet->xTilt(), tablet->yTilt(),
                  tablet->tangentialPressure(), tablet->rotation(), tablet->z(),
                  tablet->modifiers(), tablet->uniqueId(), tablet->button(), tablet->buttons());
               te.spont = e->spontaneous();
               res = d->notify_helper(w, w == receiver ? tablet : &te);
               eventAccepted = ((w == receiver) ? tablet : &te)->isAccepted();
               e->spont = false;
               if ((res && eventAccepted)
                  || w->isWindow()
                  || w->testAttribute(Qt::WA_NoMousePropagation)) {
                  break;
               }

               relpos += w->pos();
               w = w->parentWidget();
            }
            tablet->setAccepted(eventAccepted);
            qt_tabletChokeMouse = tablet->isAccepted();
         }
         break;
#endif // QT_NO_TABLETEVENT

#if !defined(QT_NO_TOOLTIP) || !defined(QT_NO_WHATSTHIS)
         case QEvent::ToolTip:
         case QEvent::WhatsThis:
         case QEvent::QueryWhatsThis: {
            QWidget *w = static_cast<QWidget *>(receiver);
            QHelpEvent *help = static_cast<QHelpEvent *>(e);
            QPoint relpos = help->pos();
            bool eventAccepted = help->isAccepted();
            while (w) {
               QHelpEvent he(help->type(), relpos, help->globalPos());
               he.spont = e->spontaneous();
               res = d->notify_helper(w, w == receiver ? help : &he);
               e->spont = false;
               eventAccepted = (w == receiver ? help : &he)->isAccepted();
               if ((res && eventAccepted) || w->isWindow()) {
                  break;
               }

               relpos += w->pos();
               w = w->parentWidget();
            }
            help->setAccepted(eventAccepted);
         }
         break;
#endif
#if !defined(QT_NO_STATUSTIP) || !defined(QT_NO_WHATSTHIS)
         case QEvent::StatusTip:
         case QEvent::WhatsThisClicked: {
            QWidget *w = static_cast<QWidget *>(receiver);
            while (w) {
               res = d->notify_helper(w, e);
               if ((res && e->isAccepted()) || w->isWindow()) {
                  break;
               }
               w = w->parentWidget();
            }
         }
         break;
#endif

#ifndef QT_NO_DRAGANDDROP
         case QEvent::DragEnter: {
            QWidget *w = static_cast<QWidget *>(receiver);
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent *>(e);
#ifndef QT_NO_GRAPHICSVIEW
            // QGraphicsProxyWidget handles its own propagation,
            // and we must not change QDragManagers currentTarget.
            QWExtra *extra = w->window()->d_func()->extra;
            if (extra && extra->proxyWidget) {
               res = d->notify_helper(w, dragEvent);
               break;
            }
#endif
            while (w) {
               if (w->isEnabled() && w->acceptDrops()) {
                  res = d->notify_helper(w, dragEvent);
                  if (res && dragEvent->isAccepted()) {
                     QDragManager::self()->setCurrentTarget(w);
                     break;
                  }
               }
               if (w->isWindow()) {
                  break;
               }
               dragEvent->p = w->mapToParent(dragEvent->p.toPoint());
               w = w->parentWidget();
            }
         }
         break;
         case QEvent::DragMove:
         case QEvent::Drop:
         case QEvent::DragLeave: {
            QWidget *w = static_cast<QWidget *>(receiver);
#ifndef QT_NO_GRAPHICSVIEW
            // QGraphicsProxyWidget handles its own propagation,
            // and we must not change QDragManagers currentTarget.
            QWExtra *extra = w->window()->d_func()->extra;
            bool isProxyWidget = extra && extra->proxyWidget;
            if (!isProxyWidget)
#endif
               w = qobject_cast<QWidget *>(QDragManager::self()->currentTarget());

            if (!w) {
               break;
            }
            if (e->type() == QEvent::DragMove || e->type() == QEvent::Drop) {
               QDropEvent *dragEvent = static_cast<QDropEvent *>(e);
               QWidget *origReciver = static_cast<QWidget *>(receiver);
               while (origReciver && w != origReciver) {
                  dragEvent->p = origReciver->mapToParent(dragEvent->p.toPoint());
                  origReciver = origReciver->parentWidget();
               }
            }
            res = d->notify_helper(w, e);
            if (e->type() != QEvent::DragMove
#ifndef QT_NO_GRAPHICSVIEW
               && !isProxyWidget
#endif
            ) {
               QDragManager::self()->setCurrentTarget(0, e->type() == QEvent::Drop);
            }
         }
         break;
#endif
         case QEvent::TouchBegin:
            // Note: TouchUpdate and TouchEnd events are never propagated
         {
            QWidget *widget = static_cast<QWidget *>(receiver);
            QTouchEvent *touchEvent = static_cast<QTouchEvent *>(e);
            bool eventAccepted = touchEvent->isAccepted();
            bool acceptTouchEvents = widget->testAttribute(Qt::WA_AcceptTouchEvents);

            if (acceptTouchEvents && e->spontaneous()) {
               const QPoint localPos = touchEvent->touchPoints()[0].pos().toPoint();
               QApplicationPrivate::giveFocusAccordingToFocusPolicy(widget, e, localPos);
            }

#ifndef QT_NO_GESTURES
            QPointer<QWidget> gesturePendingWidget;
#endif

            while (widget) {
               // first, try to deliver the touch event
               acceptTouchEvents = widget->testAttribute(Qt::WA_AcceptTouchEvents);
               touchEvent->setTarget(widget);
               touchEvent->setAccepted(acceptTouchEvents);
               QPointer<QWidget> p = widget;
               res = acceptTouchEvents && d->notify_helper(widget, touchEvent);
               eventAccepted = touchEvent->isAccepted();
               if (p.isNull()) {
                  // widget was deleted
                  widget = 0;
               } else {
                  widget->setAttribute(Qt::WA_WState_AcceptedTouchBeginEvent, res && eventAccepted);
               }
               touchEvent->spont = false;
               if (res && eventAccepted) {
                  // the first widget to accept the TouchBegin gets an implicit grab.
                  d->activateImplicitTouchGrab(widget, touchEvent);
                  break;
               }
#ifndef QT_NO_GESTURES
               if (gesturePendingWidget.isNull() && widget && QGestureManager::gesturePending(widget)) {
                  gesturePendingWidget = widget;
               }
#endif
               if (p.isNull() || widget->isWindow() || widget->testAttribute(Qt::WA_NoMousePropagation)) {
                  break;
               }

               QPoint offset = widget->pos();
               widget = widget->parentWidget();
               touchEvent->setTarget(widget);
               for (int i = 0; i < touchEvent->_touchPoints.size(); ++i) {
                  QTouchEvent::TouchPoint &pt = touchEvent->_touchPoints[i];
                  QRectF rect = pt.rect();
                  rect.translate(offset);
                  pt.d->rect = rect;
                  pt.d->startPos = pt.startPos() + offset;
                  pt.d->lastPos = pt.lastPos() + offset;
               }
            }

#ifndef QT_NO_GESTURES
            if (!eventAccepted && !gesturePendingWidget.isNull()) {
               // the first widget subscribed to a gesture gets an implicit grab
               d->activateImplicitTouchGrab(gesturePendingWidget, touchEvent);
            }
#endif

            touchEvent->setAccepted(eventAccepted);
            break;
         }
         case QEvent::TouchUpdate:
         case QEvent::TouchEnd: {
            QWidget *widget = static_cast<QWidget *>(receiver);
            // We may get here if the widget is subscribed to a gesture,
            // but has not accepted TouchBegin. Propagate touch events
            // only if TouchBegin has been accepted.
            if (widget && widget->testAttribute(Qt::WA_WState_AcceptedTouchBeginEvent)) {
               res = d->notify_helper(widget, e);
            }
            break;
         }
         case QEvent::RequestSoftwareInputPanel:
            inputMethod()->show();
            break;
         case QEvent::CloseSoftwareInputPanel:
            inputMethod()->hide();
            break;

#ifndef QT_NO_GESTURES
         case QEvent::NativeGesture: {
            // only propagate the first gesture event (after the GID_BEGIN)
            QWidget *w = static_cast<QWidget *>(receiver);
            while (w) {
               e->ignore();
               res = d->notify_helper(w, e);
               if ((res && e->isAccepted()) || w->isWindow()) {
                  break;
               }
               w = w->parentWidget();
            }
            break;
         }
         case QEvent::Gesture:
         case QEvent::GestureOverride: {
            if (receiver->isWidgetType()) {
               QWidget *w = static_cast<QWidget *>(receiver);
               QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(e);
               QList<QGesture *> allGestures = gestureEvent->gestures();

               bool eventAccepted = gestureEvent->isAccepted();
               bool wasAccepted = eventAccepted;
               while (w) {
                  // send only gestures the widget expects
                  QList<QGesture *> gestures;
                  QWidgetPrivate *wd = w->d_func();
                  for (int i = 0; i < allGestures.size();) {
                     QGesture *g = allGestures.at(i);
                     Qt::GestureType type = g->gestureType();
                     QMap<Qt::GestureType, Qt::GestureFlags>::iterator contextit =
                        wd->gestureContext.find(type);
                     bool deliver = contextit != wd->gestureContext.end() &&
                        (g->state() == Qt::GestureStarted || w == receiver ||
                           (contextit.value() & Qt::ReceivePartialGestures));
                     if (deliver) {
                        allGestures.removeAt(i);
                        gestures.append(g);
                     } else {
                        ++i;
                     }
                  }

                  if (! gestures.isEmpty()) {
                     // we have gestures for this w

                     QGestureEvent ge(gestures);
                     ge.t = gestureEvent->t;
                     ge.spont = gestureEvent->spont;
                     ge.m_accept = wasAccepted;
                     ge.m_accepted = gestureEvent->m_accepted;
                     res = d->notify_helper(w, &ge);
                     gestureEvent->spont = false;
                     eventAccepted = ge.isAccepted();

                     for (int i = 0; i < gestures.size(); ++i) {
                        QGesture *g = gestures.at(i);

                        // Ignore res [event return value] because handling of multiple gestures
                        // packed into a single QEvent depends on not consuming the event
                        if (eventAccepted || ge.isAccepted(g)) {
                           // if the gesture was accepted, mark the target widget for it
                           gestureEvent->m_targetWidgets[g->gestureType()] = w;
                           gestureEvent->setAccepted(g, true);

                        } else {
                           // if the gesture was explicitly ignored by the application,
                           // put it back so a parent can get it
                           allGestures.append(g);
                        }
                     }
                  }
                  if (allGestures.isEmpty()) { // everything delivered
                     break;
                  }
                  if (w->isWindow()) {
                     break;
                  }
                  w = w->parentWidget();
               }
               foreach (QGesture *g, allGestures) {
                  gestureEvent->setAccepted(g, false);
               }
               gestureEvent->m_accept = false; // to make sure we check individual gestures
            } else {
               res = d->notify_helper(receiver, e);
            }
            break;
         }
#endif // QT_NO_GESTURES

#ifdef Q_OS_MAC

         // Enable touch events on enter, disable on leave.
         typedef void (*RegisterTouchWindowFn)(QWindow *,  bool);

         case QEvent::Enter:
            if (receiver->isWidgetType()) {
               QWidget *w = static_cast<QWidget *>(receiver);
               if (w->testAttribute(Qt::WA_AcceptTouchEvents)) {
                  RegisterTouchWindowFn registerTouchWindow = reinterpret_cast<RegisterTouchWindowFn>
                     (platformNativeInterface()->nativeResourceFunctionForIntegration("registertouchwindow"));
                  if (registerTouchWindow) {
                     registerTouchWindow(w->window()->windowHandle(), true);
                  }
               }
            }
            res = d->notify_helper(receiver, e);
            break;

         case QEvent::Leave:
            if (receiver->isWidgetType()) {
               QWidget *w = static_cast<QWidget *>(receiver);
               if (w->testAttribute(Qt::WA_AcceptTouchEvents)) {
                  RegisterTouchWindowFn registerTouchWindow = reinterpret_cast<RegisterTouchWindowFn>
                     (platformNativeInterface()->nativeResourceFunctionForIntegration("registertouchwindow"));
                  if (registerTouchWindow) {
                     registerTouchWindow(w->window()->windowHandle(), false);
                  }
               }
            }
            res = d->notify_helper(receiver, e);
            break;
#endif
         default:
            res = d->notify_helper(receiver, e);
            break;
      }

   return res;
}

bool QApplicationPrivate::notify_helper(QObject *receiver, QEvent *e)
{
   // send to all application event filters
   if ( CSInternalThreadData::get_m_ThreadData(receiver)->thread == mainThread()
      && sendThroughApplicationEventFilters(receiver, e)) {
      return true;
   }

   if (receiver->isWidgetType()) {
      QWidget *widget = static_cast<QWidget *>(receiver);

      // toggle HasMouse widget state on enter and leave
      if ((e->type() == QEvent::Enter || e->type() == QEvent::DragEnter) &&
         (!QApplication::activePopupWidget() || QApplication::activePopupWidget() == widget->window())) {
         widget->setAttribute(Qt::WA_UnderMouse, true);

      } else if (e->type() == QEvent::Leave || e->type() == QEvent::DragLeave) {
         widget->setAttribute(Qt::WA_UnderMouse, false);
      }

      if (QLayout *layout = widget->d_func()->layout) {
         layout->widgetEvent(e);
      }
   }

   // send to all receiver event filters
   if (sendThroughObjectEventFilters(receiver, e)) {
      return true;
   }

   // deliver the event
   bool consumed = receiver->event(e);
   QCoreApplicationPrivate::setEventSpontaneous(e, false);
   return consumed;
}

bool QApplicationPrivate::inPopupMode()
{
   return QApplicationPrivate::popupWidgets != 0;
}

static void ungrabKeyboardForPopup(QWidget *popup)
{
   if (QWidget::keyboardGrabber()) {
      qt_widget_private(QWidget::keyboardGrabber())->stealKeyboardGrab(true);
   } else {
      qt_widget_private(popup)->stealKeyboardGrab(false);
   }
}

static void ungrabMouseForPopup(QWidget *popup)
{
   if (QWidget::mouseGrabber()) {
      qt_widget_private(QWidget::mouseGrabber())->stealMouseGrab(true);
   } else {
      qt_widget_private(popup)->stealMouseGrab(false);
   }
}

static bool popupGrabOk;

static void grabForPopup(QWidget *popup)
{
   Q_ASSERT(popup->testAttribute(Qt::WA_WState_Created));
   popupGrabOk = qt_widget_private(popup)->stealKeyboardGrab(true);
   if (popupGrabOk) {
      popupGrabOk = qt_widget_private(popup)->stealMouseGrab(true);
      if (!popupGrabOk) {
         // transfer grab back to the keyboard grabber if any
         ungrabKeyboardForPopup(popup);
      }
   }
}

extern QWidget *qt_button_down;
extern QWidget *qt_popup_down;
extern bool qt_replay_popup_mouse_event;

void QApplicationPrivate::closePopup(QWidget *popup)
{
   if (!popupWidgets) {
      return;
   }
   popupWidgets->removeAll(popup);

   if (popup == qt_popup_down) {
      qt_button_down = 0;
      qt_popup_down = 0;
   }

   if (QApplicationPrivate::popupWidgets->count() == 0) { // this was the last popup
      delete QApplicationPrivate::popupWidgets;
      QApplicationPrivate::popupWidgets = 0;

      if (popupGrabOk) {
         popupGrabOk = false;

         if (popup->geometry().contains(QPoint(QGuiApplicationPrivate::mousePressX,
                  QGuiApplicationPrivate::mousePressY))
            || popup->testAttribute(Qt::WA_NoMouseReplay)) {
            // mouse release event or inside
            qt_replay_popup_mouse_event = false;
         } else { // mouse press event
            qt_replay_popup_mouse_event = true;
         }

         // transfer grab back to mouse grabber if any, otherwise release the grab
         ungrabMouseForPopup(popup);

         // transfer grab back to keyboard grabber if any, otherwise release the grab
         ungrabKeyboardForPopup(popup);
      }

      if (active_window) {
         if (QWidget *fw = active_window->focusWidget()) {
            if (fw != QApplication::focusWidget()) {
               fw->setFocus(Qt::PopupFocusReason);
            } else {
               QFocusEvent e(QEvent::FocusIn, Qt::PopupFocusReason);
               QCoreApplication::sendEvent(fw, &e);
            }
         }
      }

   } else {
      // A popup was closed, so the previous popup gets the focus.
      QWidget *aw = QApplicationPrivate::popupWidgets->last();
      if (QWidget *fw = aw->focusWidget()) {
         fw->setFocus(Qt::PopupFocusReason);
      }

      if (QApplicationPrivate::popupWidgets->count() == 1) { // grab mouse/keyboard
         grabForPopup(aw);
      }
   }

}

int openPopupCount = 0;

void QApplicationPrivate::openPopup(QWidget *popup)
{
   openPopupCount++;
   if (!popupWidgets) { // create list
      popupWidgets = new QWidgetList;
   }
   popupWidgets->append(popup); // add to end of list

   if (QApplicationPrivate::popupWidgets->count() == 1) { // grab mouse/keyboard
      grabForPopup(popup);
   }

   // popups are not focus-handled by the window system (the first
   // popup grabbed the keyboard), so we have to do that manually: A
   // new popup gets the focus
   if (popup->focusWidget()) {
      popup->focusWidget()->setFocus(Qt::PopupFocusReason);
   } else if (popupWidgets->count() == 1) { // this was the first popup
      if (QWidget *fw = QApplication::focusWidget()) {
         QFocusEvent e(QEvent::FocusOut, Qt::PopupFocusReason);
         QApplication::sendEvent(fw, &e);
      }
   }
}

#ifdef QT_KEYPAD_NAVIGATION
void QApplication::setNavigationMode(Qt::NavigationMode mode)
{
   QApplicationPrivate::navigationMode = mode;
}

Qt::NavigationMode QApplication::navigationMode()
{
   return QApplicationPrivate::navigationMode;
}


void QApplication::setKeypadNavigationEnabled(bool enable)
{
   if (enable) {
      QApplication::setNavigationMode(Qt::NavigationModeKeypadTabOrder);
   } else {
      QApplication::setNavigationMode(Qt::NavigationModeNone);
   }
}


bool QApplication::keypadNavigationEnabled()
{
   return QApplicationPrivate::navigationMode == Qt::NavigationModeKeypadTabOrder ||
      QApplicationPrivate::navigationMode == Qt::NavigationModeKeypadDirectional;
}
#endif

/*!
    \fn void QApplication::alert(QWidget *widget, int msec)
    \since 4.3

    Causes an alert to be shown for \a widget if the window is not the active
    window. The alert is shown for \a msec miliseconds. If \a msec is zero (the
    default), then the alert is shown indefinitely until the window becomes
    active again.

    Currently this function does nothing on Qt for Embedded Linux.

    On \macos, this works more at the application level and will cause the
    application icon to bounce in the dock.

    On Windows, this causes the window's taskbar entry to flash for a time. If
    \a msec is zero, the flashing will stop and the taskbar entry will turn a
    different color (currently orange).

    On X11, this will cause the window to be marked as "demands attention", the
    window must not be hidden (i.e. not have hide() called on it, but be
    visible in some sort of way) in order for this to work.
*/
void QApplication::alert(QWidget *widget, int duration)
{
   if (widget) {
      if (widget->window()->isActiveWindow() && !(widget->window()->windowState() & Qt::WindowMinimized)) {
         return;
      }
      if (QWindow *window = QApplicationPrivate::windowForWidget(widget)) {
         window->alert(duration);
      }
   } else {
      foreach (QWidget *topLevel, topLevelWidgets()) {
         QApplication::alert(topLevel, duration);
      }
   }
}

/*!
    \property QApplication::cursorFlashTime
    \brief the text cursor's flash (blink) time in milliseconds

    The flash time is the time required to display, invert and restore the
    caret display. Usually the text cursor is displayed for half the cursor
    flash time, then hidden for the same amount of time, but this may vary.

    The default value on X11 is 1000 milliseconds. On Windows, the
    \uicontrol{Control Panel} value is used and setting this property sets the cursor
    flash time for all applications.

    We recommend that widgets do not cache this value as it may change at any
    time if the user changes the global desktop settings.

    \note This property may hold a negative value, for instance if cursor
    blinking is disabled.
*/
void QApplication::setCursorFlashTime(int msecs)
{
   QGuiApplication::styleHints()->setCursorFlashTime(msecs);
}

int QApplication::cursorFlashTime()
{
   return QGuiApplication::styleHints()->cursorFlashTime();
}

/*!
    \property QApplication::doubleClickInterval
    \brief the time limit in milliseconds that distinguishes a double click
    from two consecutive mouse clicks

    The default value on X11 is 400 milliseconds. On Windows and Mac OS, the
    operating system's value is used.
*/
void QApplication::setDoubleClickInterval(int ms)
{
   QGuiApplication::styleHints()->setMouseDoubleClickInterval(ms);
}

int QApplication::doubleClickInterval()
{
   return QGuiApplication::styleHints()->mouseDoubleClickInterval();
}

/*!
    \fn QApplication::keyboardInputDirection()
    \since 4.2
    \deprecated

    Returns the current keyboard input direction. Replaced with QInputMethod::inputDirection()
    \sa QInputMethod::inputDirection()
*/

/*!
    \property QApplication::keyboardInputInterval
    \brief the time limit in milliseconds that distinguishes a key press
    from two consecutive key presses
    \since 4.2

    The default value on X11 is 400 milliseconds. On Windows and Mac OS, the
    operating system's value is used.
*/
void QApplication::setKeyboardInputInterval(int ms)
{
   QGuiApplication::styleHints()->setKeyboardInputInterval(ms);
}

int QApplication::keyboardInputInterval()
{
   return QGuiApplication::styleHints()->keyboardInputInterval();
}

/*!
    \property QApplication::wheelScrollLines
    \brief the number of lines to scroll a widget, when the
    mouse wheel is rotated.

    If the value exceeds the widget's number of visible lines, the widget
    should interpret the scroll operation as a single \e{page up} or
    \e{page down}. If the widget is an \l{QAbstractItemView}{item view class},
    then the result of scrolling one \e line depends on the setting of the
    widget's \l{QAbstractItemView::verticalScrollMode()}{scroll mode}. Scroll
    one \e line can mean \l{QAbstractItemView::ScrollPerItem}{scroll one item}
    or \l{QAbstractItemView::ScrollPerPixel}{scroll one pixel}.

    By default, this property has a value of 3.
*/

#ifndef QT_NO_WHEELEVENT
int QApplication::wheelScrollLines()
{
   return QApplicationPrivate::wheel_scroll_lines;
}

void QApplication::setWheelScrollLines(int lines)
{
   QApplicationPrivate::wheel_scroll_lines = lines;
}
#endif

static inline int uiEffectToFlag(Qt::UIEffect effect)
{
   switch (effect) {
      case Qt::UI_General:
         return QPlatformTheme::GeneralUiEffect;
      case Qt::UI_AnimateMenu:
         return QPlatformTheme::AnimateMenuUiEffect;
      case Qt::UI_FadeMenu:
         return QPlatformTheme::FadeMenuUiEffect;
      case Qt::UI_AnimateCombo:
         return QPlatformTheme::AnimateComboUiEffect;
      case Qt::UI_AnimateTooltip:
         return QPlatformTheme::AnimateTooltipUiEffect;
      case Qt::UI_FadeTooltip:
         return QPlatformTheme::FadeTooltipUiEffect;
      case Qt::UI_AnimateToolBox:
         return QPlatformTheme::AnimateToolBoxUiEffect;
   }
   return 0;
}

/*!
    \fn void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)

    Enables the UI effect \a effect if \a enable is true, otherwise the effect
    will not be used.

    \note All effects are disabled on screens running at less than 16-bit color
    depth.

    \sa isEffectEnabled(), Qt::UIEffect, setDesktopSettingsAware()
*/
void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
   int effectFlags = uiEffectToFlag(effect);
   if (enable) {
      if (effectFlags & QPlatformTheme::FadeMenuUiEffect) {
         effectFlags |= QPlatformTheme::AnimateMenuUiEffect;
      }
      if (effectFlags & QPlatformTheme::FadeTooltipUiEffect) {
         effectFlags |= QPlatformTheme::AnimateTooltipUiEffect;
      }
      QApplicationPrivate::enabledAnimations |= effectFlags;
   } else {
      QApplicationPrivate::enabledAnimations &= ~effectFlags;
   }
}

/*!
    \fn bool QApplication::isEffectEnabled(Qt::UIEffect effect)

    Returns \c true if \a effect is enabled; otherwise returns \c false.

    By default, Qt will try to use the desktop settings. To prevent this, call
    setDesktopSettingsAware(false).

    \note All effects are disabled on screens running at less than 16-bit color
    depth.

    \sa setEffectEnabled(), Qt::UIEffect
*/
bool QApplication::isEffectEnabled(Qt::UIEffect effect)
{
   CHECK_QAPP_INSTANCE(false)
   return QColormap::instance().depth() >= 16
      && (QApplicationPrivate::enabledAnimations & QPlatformTheme::GeneralUiEffect)
      && (QApplicationPrivate::enabledAnimations & uiEffectToFlag(effect));
}

/*!
    \fn void QApplication::beep()

    Sounds the bell, using the default volume and sound. The function is \e not
    available in Qt for Embedded Linux.
*/
void QApplication::beep()
{
   QMetaObject::invokeMethod(QGuiApplication::platformNativeInterface(), "beep");
}

/*!
    \macro qApp
    \relates QApplication

    A global pointer referring to the unique application object. It is
    equivalent to QCoreApplication::instance(), but cast as a QApplication pointer,
    so only valid when the unique application object is a QApplication.

    \sa QCoreApplication::instance(), qGuiApp
*/

/*!
    \fn QLocale QApplication::keyboardInputLocale()
    \since 4.2
    \obsolete

    Returns the current keyboard input locale. Replaced with QInputMethod::locale()
*/

bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event)
{
   return QGuiApplication::sendSpontaneousEvent(receiver, event);
}

void QApplicationPrivate::giveFocusAccordingToFocusPolicy(QWidget *widget, QEvent *event, QPoint localPos)
{
   const bool setFocusOnRelease = QGuiApplication::styleHints()->setFocusOnTouchRelease();
   Qt::FocusPolicy focusPolicy = Qt::ClickFocus;
   static QPointer<QWidget> focusedWidgetOnTouchBegin = 0;

   switch (event->type()) {
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonDblClick:
      case QEvent::TouchBegin:
         focusedWidgetOnTouchBegin = QApplication::focusWidget();
         if (setFocusOnRelease) {
            return;
         }
         break;
      case QEvent::MouseButtonRelease:
      case QEvent::TouchEnd:
         if (!setFocusOnRelease) {
            return;
         }
         if (focusedWidgetOnTouchBegin != QApplication::focusWidget()) {
            // Focus widget was changed while delivering press/move events.
            // To not interfere with application logic, we leave focus as-is
            return;
         }
         break;
      case QEvent::Wheel:
         focusPolicy = Qt::WheelFocus;
         break;
      default:
         return;
   }

   QWidget *focusWidget = widget;
   while (focusWidget) {
      if (focusWidget->isEnabled()
         && focusWidget->rect().contains(localPos)
         && QApplicationPrivate::shouldSetFocus(focusWidget, focusPolicy)) {
         focusWidget->setFocus(Qt::MouseFocusReason);
         break;
      }
      if (focusWidget->isWindow()) {
         break;
      }

      // find out whether this widget (or its proxy) already has focus
      QWidget *f = focusWidget;
      if (focusWidget->d_func()->extra && focusWidget->d_func()->extra->focus_proxy) {
         f = focusWidget->d_func()->extra->focus_proxy;
      }
      // if it has, stop here.
      // otherwise a click on the focused widget would remove its focus if ClickFocus isn't set
      if (f->hasFocus()) {
         break;
      }

      localPos += focusWidget->pos();
      focusWidget = focusWidget->parentWidget();
   }
}

bool QApplicationPrivate::shouldSetFocus(QWidget *w, Qt::FocusPolicy policy)
{
   QWidget *f = w;
   while (f->d_func()->extra && f->d_func()->extra->focus_proxy) {
      f = f->d_func()->extra->focus_proxy;
   }

   if ((w->focusPolicy() & policy) != policy) {
      return false;
   }
   if (w != f && (f->focusPolicy() & policy) != policy) {
      return false;
   }
   return true;
}

bool QApplicationPrivate::updateTouchPointsForWidget(QWidget *widget, QTouchEvent *touchEvent)
{
   bool containsPress = false;
   for (int i = 0; i < touchEvent->touchPoints().count(); ++i) {
      QTouchEvent::TouchPoint &touchPoint = touchEvent->_touchPoints[i];

      // preserve the sub-pixel resolution
      QRectF rect = touchPoint.screenRect();
      const QPointF screenPos = rect.center();
      const QPointF delta = screenPos - screenPos.toPoint();

      rect.moveCenter(widget->mapFromGlobal(screenPos.toPoint()) + delta);
      touchPoint.d->rect = rect;
      touchPoint.d->startPos = widget->mapFromGlobal(touchPoint.startScreenPos().toPoint()) + delta;
      touchPoint.d->lastPos = widget->mapFromGlobal(touchPoint.lastScreenPos().toPoint()) + delta;

      if (touchPoint.state() == Qt::TouchPointPressed) {
         containsPress = true;
      }
   }
   return containsPress;
}

void QApplicationPrivate::initializeMultitouch()
{
   initializeMultitouch_sys();
}

void QApplicationPrivate::initializeMultitouch_sys()
{
}

void QApplicationPrivate::cleanupMultitouch()
{
   cleanupMultitouch_sys();
}

void QApplicationPrivate::cleanupMultitouch_sys()
{
}

QWidget *QApplicationPrivate::findClosestTouchPointTarget(QTouchDevice *device, const QTouchEvent::TouchPoint &touchPoint)
{
   const QPointF screenPos = touchPoint.screenPos();
   int closestTouchPointId = -1;
   QObject *closestTarget = 0;
   qreal closestDistance = qreal(0.);
   QHash<ActiveTouchPointsKey, ActiveTouchPointsValue>::const_iterator it = activeTouchPoints.constBegin(),
                                                                       ite = activeTouchPoints.constEnd();
   while (it != ite) {
      if (it.key().device == device && it.key().touchPointId != touchPoint.id()) {
         const QTouchEvent::TouchPoint &touchPoint = it->touchPoint;
         qreal dx = screenPos.x() - touchPoint.screenPos().x();
         qreal dy = screenPos.y() - touchPoint.screenPos().y();
         qreal distance = dx * dx + dy * dy;
         if (closestTouchPointId == -1 || distance < closestDistance) {
            closestTouchPointId = touchPoint.id();
            closestDistance = distance;
            closestTarget = it.value().target.data();
         }
      }
      ++it;
   }
   return static_cast<QWidget *>(closestTarget);
}

void QApplicationPrivate::activateImplicitTouchGrab(QWidget *widget, QTouchEvent *touchEvent)
{
   if (touchEvent->type() != QEvent::TouchBegin) {
      return;
   }

   for (int i = 0, tc = touchEvent->touchPoints().count(); i < tc; ++i) {
      const QTouchEvent::TouchPoint &touchPoint = touchEvent->touchPoints().at(i);
      activeTouchPoints[QGuiApplicationPrivate::ActiveTouchPointsKey(touchEvent->device(), touchPoint.id())].target = widget;
   }
}

bool QApplicationPrivate::translateRawTouchEvent(QWidget *window, QTouchDevice *device,
   const QList<QTouchEvent::TouchPoint> &touchPoints, ulong timestamp)
{
   QApplicationPrivate *d = self;
   typedef QPair<Qt::TouchPointStates, QList<QTouchEvent::TouchPoint>> StatesAndTouchPoints;
   QHash<QWidget *, StatesAndTouchPoints> widgetsNeedingEvents;

   for (int i = 0; i < touchPoints.count(); ++i) {
      QTouchEvent::TouchPoint touchPoint = touchPoints.at(i);
      // explicitly detach from the original touch point that we got, so even
      // if the touchpoint structs are reused, we will make a copy that we'll
      // deliver to the user (which might want to store the struct for later use).
      touchPoint.d = touchPoint.d->detach();

      // update state
      QPointer<QObject> target;
      ActiveTouchPointsKey touchInfoKey(device, touchPoint.id());
      ActiveTouchPointsValue &touchInfo = d->activeTouchPoints[touchInfoKey];

      if (touchPoint.state() == Qt::TouchPointPressed) {
         if (device->type() == QTouchDevice::TouchPad) {
            // on touch-pads, send all touch points to the same widget
            target = d->activeTouchPoints.isEmpty()
               ? QPointer<QObject>()
               : d->activeTouchPoints.constBegin().value().target;
         }

         if (!target) {
            // determine which widget this event will go to

            if (! window) {
               window = QApplication::topLevelWidgetAt(touchPoint.screenPos().toPoint());
            }

            if (!window) {
               continue;
            }

            target = window->childAt(window->mapFromGlobal(touchPoint.screenPos().toPoint()));

            if (!target) {
               target = window;
            }
         }

         if (device->type() == QTouchDevice::TouchScreen) {
            QWidget *closestWidget = d->findClosestTouchPointTarget(device, touchPoint);
            QWidget *widget = static_cast<QWidget *>(target.data());
            if (closestWidget
               && (widget->isAncestorOf(closestWidget) || closestWidget->isAncestorOf(widget))) {
               target = closestWidget;
            }
         }

         touchInfo.target = target;
      } else {
         target = touchInfo.target;
         if (!target) {
            continue;
         }
      }
      Q_ASSERT(target.data() != 0);

      QWidget *targetWidget = static_cast<QWidget *>(target.data());

#ifdef Q_OS_MAC
      // Single-touch events are normally not sent unless WA_TouchPadAcceptSingleTouchEvents is set.
      // In Qt 4 this check was in OS X-only code. That behavior is preserved here by the #ifdef.
      if (touchPoints.count() == 1
         && device->type() == QTouchDevice::TouchPad
         && !targetWidget->testAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents)) {
         continue;
      }
#endif

      StatesAndTouchPoints &maskAndPoints = widgetsNeedingEvents[targetWidget];
      maskAndPoints.first |= touchPoint.state();
      maskAndPoints.second.append(touchPoint);
   }

   if (widgetsNeedingEvents.isEmpty()) {
      return false;
   }

   bool accepted = false;
   QHash<QWidget *, StatesAndTouchPoints>::const_iterator it = widgetsNeedingEvents.constBegin();
   const QHash<QWidget *, StatesAndTouchPoints>::const_iterator end = widgetsNeedingEvents.constEnd();
   for (; it != end; ++it) {
      const QPointer<QWidget> widget = it.key();
      if (!QApplicationPrivate::tryModalHelper(widget, 0)) {
         continue;
      }

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

      QTouchEvent touchEvent(eventType,
         device,
         QApplication::keyboardModifiers(),
         it.value().first,
         it.value().second);
      bool containsPress = updateTouchPointsForWidget(widget, &touchEvent);
      touchEvent.setTimestamp(timestamp);
      touchEvent.setWindow(window->windowHandle());
      touchEvent.setTarget(widget);

      if (containsPress) {
         widget->setAttribute(Qt::WA_WState_AcceptedTouchBeginEvent);
      }

      switch (touchEvent.type()) {
         case QEvent::TouchBegin: {
            // if the TouchBegin handler recurses, we assume that means the event
            // has been implicitly accepted and continue to send touch events
            if (QApplication::sendSpontaneousEvent(widget, &touchEvent) && touchEvent.isAccepted()) {
               accepted = true;
               if (!widget.isNull()) {
                  widget->setAttribute(Qt::WA_WState_AcceptedTouchBeginEvent);
               }
            }
            break;
         }
         default:
            if (widget->testAttribute(Qt::WA_WState_AcceptedTouchBeginEvent)
#ifndef QT_NO_GESTURES
               || QGestureManager::gesturePending(widget)
#endif
            ) {
               if (QApplication::sendSpontaneousEvent(widget, &touchEvent) && touchEvent.isAccepted()) {
                  accepted = true;
               }
               // widget can be deleted on TouchEnd
               if (touchEvent.type() == QEvent::TouchEnd && !widget.isNull()) {
                  widget->setAttribute(Qt::WA_WState_AcceptedTouchBeginEvent, false);
               }
            }
            break;
      }
   }
   return accepted;
}

void QApplicationPrivate::translateTouchCancel(QTouchDevice *device, ulong timestamp)
{
   QTouchEvent touchEvent(QEvent::TouchCancel, device, QApplication::keyboardModifiers());
   touchEvent.setTimestamp(timestamp);

   QHash<ActiveTouchPointsKey, ActiveTouchPointsValue>::const_iterator it
      = self->activeTouchPoints.constBegin(), ite = self->activeTouchPoints.constEnd();

   QSet<QWidget *> widgetsNeedingCancel;

   while (it != ite) {
      QWidget *widget = static_cast<QWidget *>(it->target.data());
      if (widget) {
         widgetsNeedingCancel.insert(widget);
      }
      ++it;
   }
   for (QSet<QWidget *>::const_iterator widIt = widgetsNeedingCancel.constBegin(),
      widItEnd = widgetsNeedingCancel.constEnd(); widIt != widItEnd; ++widIt) {
      QWidget *widget = *widIt;
      touchEvent.setWindow(widget->windowHandle());
      touchEvent.setTarget(widget);
      QApplication::sendSpontaneousEvent(widget, &touchEvent);
   }
}


#ifndef QT_NO_DRAGANDDROP
void QApplicationPrivate::notifyDragStarted(const QDrag *drag)
{
   // Prevent pickMouseReceiver() from using the widget where the drag was started after a drag operation.
   QGuiApplicationPrivate::notifyDragStarted(drag);
   qt_button_down = 0;
}
#endif

#ifndef QT_NO_GESTURES
QGestureManager *QGestureManager::instance()
{
   QApplicationPrivate *qAppPriv = QApplicationPrivate::instance();

   if (!qAppPriv) {
      return 0;
   }

   if (!qAppPriv->gestureManager) {
      qAppPriv->gestureManager = new QGestureManager(qApp);
   }
   return qAppPriv->gestureManager;
}
#endif

QPixmap QApplication::cs_internal_applyQIconStyle(QIcon::Mode mode, const QPixmap &base) const
{
   Q_D(const QApplication);
   return d->applyQIconStyleHelper(mode, base);
}

QPixmap QApplicationPrivate::applyQIconStyleHelper(QIcon::Mode mode, const QPixmap &base) const
{
   QStyleOption opt(0);
   opt.palette = QGuiApplication::palette();

   return QApplication::style()->generatedIconPixmap(mode, base, &opt);
}


