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

#ifndef QAPPLICATION_P_H
#define QAPPLICATION_P_H

#include <qapplication.h>
#include <qbasictimer.h>
#include <qevent.h>
#include <qfont.h>
#include <qcursor.h>
#include <qhash.h>
#include <qicon.h>
#include <qmutex.h>
#include <qtranslator.h>
#include <qplatform_integration.h>
#include <qplatform_nativeinterface.h>
#include <qpointer.h>
#include <qpoint.h>
#include <qpointf.h>
#include <qregion.h>
#include <qtime.h>
#include <qwindow.h>
#include <qwidget.h>
#include <qwindowsysteminterface.h>

#include <qapplication_p.h>
#include <qcoreapplication_p.h>
#include <qshortcutmap_p.h>
#include <qthread_p.h>
#include <qwindowsysteminterface_p.h>

class QClipboard;
class QGraphicsScene;
class QInputDeviceManager;
class QObject;
class QSocketNotifier;
class QTouchDevice;
class QPlatformTheme;
class QPlatformDragQtResponse;
class QWidget;

struct QDrawHelperGammaTables;

#ifndef QT_NO_GESTURES
class QGestureManager;
#endif

#ifndef QT_NO_DRAGANDDROP
class QDrag;
#endif

#ifndef QT_NO_CLIPBOARD
extern QClipboard *qt_clipboard;
#endif

#if defined (Q_OS_WIN)
extern QSysInfo::WinVersion qt_winver;

#elif defined (Q_OS_DARWIN)
extern QSysInfo::MacVersion qt_macver;

#endif

typedef QHash<QString, QFont> FontHash;
FontHash *cs_app_fonts_hash();

typedef QHash<QString, QPalette> PaletteHash;
PaletteHash *cs_app_palettes_hash();

class Q_GUI_EXPORT QApplicationPrivate : public QCoreApplicationPrivate
{
   Q_DECLARE_PUBLIC(QApplication)

 public:
   QApplicationPrivate(int &argc, char **argv, int flags);
   ~QApplicationPrivate();

   void createPlatformIntegration();
   void createEventDispatcher() override;
   void eventDispatcherReady() override;

   virtual void notifyLayoutDirectionChange();
   virtual void notifyActiveWindowChange(QWindow *previous);

   bool shouldQuit() override;
   bool shouldQuitInternal(const QWindowList &processedWindows);

   virtual bool tryCloseAllWindows();

   static QString desktopStyleKey();

   static void dispatchEnterLeave(QWidget *enter, QWidget *leave, const QPointF &globalPosF);
   virtual void notifyWindowIconChanged();

   virtual bool popupActive()  {
      return inPopupMode();
   }

   void closePopup(QWidget *popup);
   void openPopup(QWidget *popup);

   bool notify_helper(QObject *receiver, QEvent *e);

   void init();
   void initialize();
   void process_cmdline();

   // modality
   virtual bool isWindowBlocked(QWindow *window, QWindow **blockingWindow = nullptr) const;

   QPixmap getPixmapCursor(Qt::CursorShape cshape);

   void _q_updateFocusObject(QObject *object);

   static QPlatformIntegration *platformIntegration() {
      return platform_integration;
   }
   static QPlatformTheme *platform_theme;

   static QPlatformTheme *platformTheme() {
      return platform_theme;
   }

   static QAbstractEventDispatcher *cs_internal_core_dispatcher() {
      if (QCoreApplication::instance()) {
         return CSInternalThreadData::get_m_ThreadData(QCoreApplication::instance())->eventDispatcher;
      } else {
         return nullptr;
      }
   }

   static void processMouseEvent(QWindowSystemInterfacePrivate::MouseEvent *e);
   static void processKeyEvent(QWindowSystemInterfacePrivate::KeyEvent *e);
   static void processWheelEvent(QWindowSystemInterfacePrivate::WheelEvent *e);
   static void processTouchEvent(QWindowSystemInterfacePrivate::TouchEvent *e);

   static void processCloseEvent(QWindowSystemInterfacePrivate::CloseEvent *e);

   static void processGeometryChangeEvent(QWindowSystemInterfacePrivate::GeometryChangeEvent *e);

   static void processEnterEvent(QWindowSystemInterfacePrivate::EnterEvent *e);
   static void processLeaveEvent(QWindowSystemInterfacePrivate::LeaveEvent *e);

   static void processActivatedEvent(QWindowSystemInterfacePrivate::ActivatedWindowEvent *e);

   static void processWindowStateChangedEvent(QWindowSystemInterfacePrivate::WindowStateChangedEvent *e);
   static void processWindowScreenChangedEvent(QWindowSystemInterfacePrivate::WindowScreenChangedEvent *e);

   static void processWindowSystemEvent(QWindowSystemInterfacePrivate::WindowSystemEvent *e);
   static void updateFilteredScreenOrientation(QScreen *screen);
   static void reportScreenOrientationChange(QScreen *screen);
   static void reportScreenOrientationChange(QWindowSystemInterfacePrivate::ScreenOrientationEvent *e);
   static void reportGeometryChange(QWindowSystemInterfacePrivate::ScreenGeometryEvent *e);
   static void reportLogicalDotsPerInchChange(QWindowSystemInterfacePrivate::ScreenLogicalDotsPerInchEvent *e);
   static void reportRefreshRateChange(QWindowSystemInterfacePrivate::ScreenRefreshRateEvent *e);
   static void processThemeChanged(QWindowSystemInterfacePrivate::ThemeChangeEvent *tce);

   static void processExposeEvent(QWindowSystemInterfacePrivate::ExposeEvent *e);

   static void processFileOpenEvent(QWindowSystemInterfacePrivate::FileOpenEvent *e);

   static void processTabletEvent(QWindowSystemInterfacePrivate::TabletEvent *e);
   static void processTabletEnterProximityEvent(QWindowSystemInterfacePrivate::TabletEnterProximityEvent *e);
   static void processTabletLeaveProximityEvent(QWindowSystemInterfacePrivate::TabletLeaveProximityEvent *e);

#ifndef QT_NO_GESTURES
   static void processGestureEvent(QWindowSystemInterfacePrivate::GestureEvent *e);
#endif
   static void processPlatformPanelEvent(QWindowSystemInterfacePrivate::PlatformPanelEvent *e);

#ifndef QT_NO_CONTEXTMENU
   static void processContextMenuEvent(QWindowSystemInterfacePrivate::ContextMenuEvent *e);
#endif

#ifndef QT_NO_DRAGANDDROP
   static QPlatformDragQtResponse processDrag(QWindow *w, const QMimeData *dropData, const QPoint &p, Qt::DropActions supportedActions);
   static QPlatformDropQtResponse processDrop(QWindow *w, const QMimeData *dropData, const QPoint &p, Qt::DropActions supportedActions);
#endif

   static bool processNativeEvent(QWindow *window, const QByteArray &eventType, void *message, long *result);

   static void sendQWindowEventToQPlatformWindow(QWindow *window, QEvent *event);

   static inline Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment) {
      if (! (alignment & Qt::AlignHorizontal_Mask)) {
         alignment |= Qt::AlignLeft;
      }

      if (! (alignment & Qt::AlignAbsolute) && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
         if (direction == Qt::RightToLeft) {
            alignment ^= (Qt::AlignLeft | Qt::AlignRight);
         }

         alignment |= Qt::AlignAbsolute;
      }

      return alignment;
   }

   static void emitLastWindowClosed();

   static QApplicationPrivate *instance() {
      return self;
   }

   static void showModalWindow(QWindow *window);
   static void hideModalWindow(QWindow *window);
   static void updateBlockedStatus(QWindow *window);

   QInputMethod *inputMethod;
   QString firstWindowTitle;
   QIcon forcedWindowIcon;
   QWindowList modalWindowList;

   static Qt::KeyboardModifiers modifier_buttons;
   static Qt::MouseButtons mouse_buttons;

   static QPlatformIntegration *platform_integration;

   static bool autoSipEnabled;
   static QIcon *app_icon;
   static QString *platform_name;
   static QString *displayName;

   static Qt::MouseButtons buttons;
   static ulong mousePressTime;
   static Qt::MouseButton mousePressButton;

   static int mousePressX;
   static int mousePressY;

   static int mouse_double_click_distance;
   static QPointF lastCursorPosition;
   static Qt::MouseButtons tabletState;
   static QWindow *tabletPressTarget;
   static QWindow *currentMouseWindow;
   static QWindow *currentMousePressWindow;
   static Qt::ApplicationState applicationState;

   static bool highDpiScalingUpdated;

#ifndef QT_NO_CLIPBOARD
   static QClipboard *qt_clipboard;
#endif

   static QPalette *app_palette;
   static QPalette *sys_palette;
   static QPalette *set_palette;
   static QFont *sys_font;
   static QFont *set_font;

   static QWidget *main_widget;
   static QWidget *focus_widget;
   static QWidget *hidden_focus_widget;
   static QWidget *active_window;

   static QWindowList window_list;
   static QWindow *focus_window;

#ifndef QT_NO_CURSOR
   QList<QCursor> cursor_list;
#endif

   static QList<QScreen *> screen_list;
   static QFont *app_font;
   static QString styleOverride;
   static QStyleHints *styleHints;

   static bool obey_desktop_settings;

   static QList<QObject *> generic_plugin_list;

#ifndef QT_NO_SHORTCUT
   QShortcutMap shortcutMap;
#endif

   static bool isBlockedByModal(QWidget *widget);
   static bool modalState();
   static bool tryModalHelper(QWidget *widget, QWidget **rettop = nullptr);

   // style
   static bool usesNativeStyle() {
      return ! overrides_native_style;
   }

   static bool inPopupMode();

   static void setFocusWidget(QWidget *focus, Qt::FocusReason reason);
   static QWidget *focusNextPrevChild_helper(QWidget *toplevel, bool next, bool *wrappingOccurred = nullptr);

#ifndef QT_NO_SESSIONMANAGER
   static bool is_fallback_session_management_enabled;

   QSessionManager *session_manager;
   bool is_session_restored;

   bool is_saving_session;
   void commitData();
   void saveState();
#endif

   struct ActiveTouchPointsKey {
      ActiveTouchPointsKey(QTouchDevice *dev, int id) : device(dev), touchPointId(id) { }
      QTouchDevice *device;
      int touchPointId;
   };

   struct ActiveTouchPointsValue {
      QPointer<QWindow> window;
      QPointer<QObject> target;
      QTouchEvent::TouchPoint touchPoint;
   };

   QHash<ActiveTouchPointsKey, ActiveTouchPointsValue> activeTouchPoints;
   QEvent::Type lastTouchType;

   struct SynthesizedMouseData {
      SynthesizedMouseData(const QPointF &p, const QPointF &sp, QWindow *w)
         : pos(p), screenPos(sp), window(w) { }
      QPointF pos;
      QPointF screenPos;
      QPointer<QWindow> window;
   };

   QHash<QWindow *, SynthesizedMouseData> synthesizedMousePoints;

   static int mouseEventCaps(QMouseEvent *event);
   static QVector2D mouseEventVelocity(QMouseEvent *event);

   static void setMouseEventCapsAndVelocity(QMouseEvent *event, int caps, const QVector2D &velocity);

   static Qt::MouseEventSource mouseEventSource(const QMouseEvent *event);
   static void setMouseEventSource(QMouseEvent *event, Qt::MouseEventSource source);

   static Qt::MouseEventFlags mouseEventFlags(const QMouseEvent *event);

   static void setMouseEventFlags(QMouseEvent *event, Qt::MouseEventFlags flags);

   static QInputDeviceManager *inputDeviceManager();

   const QDrawHelperGammaTables *gammaTables();

   virtual QPixmap applyQIconStyleHelper(QIcon::Mode mode, const QPixmap &base) const;

   static void applyWindowGeometrySpecificationTo(QWindow *window);
   static void setApplicationState(Qt::ApplicationState state, bool forcePropagate = false);

   // emerald - for QTBUG-50199
   static bool scrollNoPhaseAllowed;

#ifndef QT_NO_GRAPHICSVIEW
   // Maintain a list of all scenes to ensure font and palette propagation to all scenes.
   QList<QGraphicsScene *> scene_list;
#endif

   QBasicTimer toolTipWakeUp, toolTipFallAsleep;
   QPoint toolTipPos, toolTipGlobalPos, hoverGlobalPos;
   QPointer<QWidget> toolTipWidget;

   static QSize app_strut;
   static QWidgetList *popupWidgets;
   static QStyle *app_style;
   static bool overrides_native_style;

   static int app_cspec;

#ifndef QT_NO_WHEELEVENT
   static int  wheel_scroll_lines;
   static QPointer<QWidget> wheel_widget;
#endif

   static int enabledAnimations;            // Combination of QPlatformTheme::UiEffect

   static void setSystemPalette(const QPalette &pal);
   static void setPalette_helper(const QPalette &palette, const QString &className, bool clearWidgetPaletteHash);
   static void initializeWidgetPaletteHash();
   static void initializeWidgetFontHash();
   static void setSystemFont(const QFont &font);

#ifndef QT_NO_STYLE_STYLESHEET
   static QString styleSheet;

#endif

   static QPointer<QWidget> leaveAfterRelease;
   static QWidget *pickMouseReceiver(QWidget *candidate, const QPoint &windowPos, QPoint *pos,
      QEvent::Type type, Qt::MouseButtons buttons,
      QWidget *buttonDown, QWidget *alienWidget);

   static bool sendMouseEvent(QWidget *receiver, QMouseEvent *event, QWidget *alienWidget,
      QWidget *native, QWidget **buttonDown, QPointer<QWidget> &lastMouseReceiver, bool spontaneous = true);

   void sendSyntheticEnterLeave(QWidget *widget);

   static QWindow *windowForWidget(const QWidget *widget) {
      if (QWindow *window = widget->windowHandle()) {
         return window;
      }

      if (const QWidget *nativeParent = widget->nativeParentWidget()) {
         return nativeParent->windowHandle();
      }

      return nullptr;
   }

#ifdef Q_OS_WIN
   static HWND getHWNDForWidget(const QWidget *widget) {
      if (QWindow *window = windowForWidget(widget)) {
         if (window->handle()) {
            return static_cast<HWND> (QApplication::platformNativeInterface()->nativeResourceForWindow("handle", window));
         }
      }

      return nullptr;
   }
#endif

#ifndef QT_NO_GESTURES
   QGestureManager *gestureManager;
   QWidget *gestureWidget;
#endif

   static bool updateTouchPointsForWidget(QWidget *widget, QTouchEvent *touchEvent);

   void initializeMultitouch();
   void initializeMultitouch_sys();
   void cleanupMultitouch();
   void cleanupMultitouch_sys();

   QWidget *findClosestTouchPointTarget(QTouchDevice *device, const QTouchEvent::TouchPoint &touchPoint);

   void appendTouchPoint(const QTouchEvent::TouchPoint &touchPoint);
   void removeTouchPoint(int touchPointId);
   void activateImplicitTouchGrab(QWidget *widget, QTouchEvent *touchBeginEvent);

   static bool translateRawTouchEvent(QWidget *widget, QTouchDevice *device,
      const QList<QTouchEvent::TouchPoint> &touchPoints, ulong timestamp);

   static void translateTouchCancel(QTouchDevice *device, ulong timestamp);

 protected:
   virtual void notifyThemeChanged();
   bool tryCloseRemainingWindows(QWindowList processedWindows);

#ifndef QT_NO_DRAGANDDROP
   virtual void notifyDragStarted(const QDrag *);
#endif

 private:
   static QTouchDevice *m_fakeTouchDevice;
   static int m_fakeMouseSourcePointId;
   QAtomicPointer<QDrawHelperGammaTables> m_gammaTables;

   bool ownGlobalShareContext;

   static QInputDeviceManager *m_inputDeviceManager;

   static QApplicationPrivate *self;

   static bool tryCloseAllWidgetWindows(QWindowList *processedWindows);

   static void giveFocusAccordingToFocusPolicy(QWidget *w, QEvent *event, QPoint localPos);
   static bool shouldSetFocus(QWidget *w, Qt::FocusPolicy policy);

   static bool isAlien(QWidget *);

   friend class QDragManager;
};

Q_GUI_EXPORT uint qHash(const QApplicationPrivate::ActiveTouchPointsKey &k);

Q_GUI_EXPORT bool operator==(const QApplicationPrivate::ActiveTouchPointsKey &a,
   const QApplicationPrivate::ActiveTouchPointsKey &b);

extern void cs_internal_set_cursor(QWidget *w, bool force);

#endif
